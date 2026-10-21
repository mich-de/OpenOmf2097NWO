// OpenOMF 2097 NWO: the 24 pilots' pictures (see game/nwo_pilots.c for who they are):
//
//   pilots <resources-dir> <art-dir>
//
// reads  MELEE.BK, VS.BK, <art-dir>/<name>.pix (drawn pixel by pixel) or else <name>.png
//        (drawn by portraits.py)
// writes PILOTS.EXT with their pictures in their own colors (see game/utils/har_extended.c),
//        MELEE.BK with their grid icons (animation 3) and big faces (animation 4), and
//        VS.BK with their big faces (animation 4), at their pilot ids (0..9, 11..24). In the BK
//        files the pictures are only approximations in the scene's colors (those palettes
//        have no room left): the engine draws them from PILOTS.EXT.
//
// A .pix is the big face itself, 57x57, drawn a pixel at a time as the original faces were:
// palette lines "c #rrggbb", then 57 rows of pixels ('.' transparent), each written out, or as
// "lead|pixels" (lead transparent pixels first), or as "@column:pixels column:pixels ..." pieces. Its icon is cut out of it 1:1, brows to chin.
//
// The art is otherwise a 240x240 portrait, transparent around the figure. Like the original pilots'
// pictures, the big face (57x57) and the icon (51x36, on black) are close-ups of the head.
// Colors come from each file's own palette, from the entries its scene keeps fixed
// (0x60..0xEF: not the HAR ramps below, not the blinking cursors and texts above), so the
// menus' palette effects (the dimmed unselected icons) work on them too.
#include <png.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats/bk.h"
#include "formats/bkanim.h"
#include "formats/error.h"
#include "formats/sprite.h"
#include "formats/vga_image.h"
#include "utils/path.h"

#define ICONS 3 // MELEE.BK: grid icons, one per pilot
#define FACES 4 // MELEE.BK and VS.BK: big faces, one per pilot (Kreissack's is 10)
#define ART_SIZE 240
#define FACE_SIZE 57
#define ICON_W 51
#define ICON_H 36
#define PIX_ICON_X 3 // where a .pix face's icon is cut out
#define PIX_ICON_Y 12

// in the select grid's order; pilot id 10 stays Major Kreissack
static const char *pilot_art[] = {"trump",    "musk",     "meloni",    "macron",     "scholz",  "starmer",
                                  "leyen",    "rutte",    "xi",        "kim",        "putin",   "zelensky",
                                  "belousov", "umerov",   "lukashenko", "netanyahu", "katz",    "khamenei",
                                  "al-houthi", "erdogan", "bin salman", "al thani",  "al-sisi", "orban"};
#define NEW_PILOTS 24
#define PILOT_ID(n) ((n) < 10 ? (n) : (n) + 1)

typedef struct {
    int w, h;
    unsigned char *px; // RGBA
} rgba;

static bool rgba_load(rgba *img, const char *dir, const char *name) {
    char fn[1024];
    snprintf(fn, sizeof(fn), "%s/%s.png", dir, name);
    png_image png;
    memset(&png, 0, sizeof(png));
    png.version = PNG_IMAGE_VERSION;
    if(!png_image_begin_read_from_file(&png, fn)) {
        fprintf(stderr, "cannot read %s\n", fn);
        return false;
    }
    png.format = PNG_FORMAT_RGBA;
    img->w = png.width;
    img->h = png.height;
    img->px = malloc(PNG_IMAGE_SIZE(png));
    return png_image_finish_read(&png, NULL, img->px, 0, NULL) != 0;
}

// A face drawn pixel by pixel; false when the pilot has none.
static bool pix_load(rgba *img, const char *dir, const char *name) {
    char fn[1024], line[1024];
    snprintf(fn, sizeof(fn), "%s/%s.pix", dir, name);
    FILE *in = fopen(fn, "r");
    if(!in) {
        return false;
    }
    unsigned char pal[256][3];
    bool known[256] = {false};
    *img = (rgba){FACE_SIZE, FACE_SIZE, calloc(FACE_SIZE * FACE_SIZE * 4, 1)};
    int row = 0;
    while(fgets(line, sizeof(line), in)) {
        line[strcspn(line, "\r\n")] = 0;
        unsigned r, g, b;
        if(row == 0 && strlen(line) == 9 && sscanf(line + 1, " #%2x%2x%2x", &r, &g, &b) == 3) {
            unsigned char c = line[0];
            pal[c][0] = r, pal[c][1] = g, pal[c][2] = b;
            known[c] = true;
            continue;
        }
        if(!line[0] || row >= FACE_SIZE) {
            continue;
        }
        char px[FACE_SIZE + 1];
        memset(px, '.', FACE_SIZE);
        px[FACE_SIZE] = 0;
        if(line[0] == '@') {
            for(char *piece = strtok(line + 1, " "); piece; piece = strtok(NULL, " ")) {
                char *colon = strchr(piece, ':');
                for(int col = colon ? atoi(piece) : FACE_SIZE, i = 0; colon && colon[1 + i] && col + i < FACE_SIZE; i++) {
                    px[col + i] = colon[1 + i];
                }
            }
        } else {
            // "lead|pixels": that many transparent pixels first
            char *bar = strchr(line, '|');
            int lead = bar ? atoi(line) : 0;
            const char *pixels = bar ? bar + 1 : line;
            for(int i = 0; pixels[i] && lead + i < FACE_SIZE; i++) {
                px[lead + i] = pixels[i];
            }
        }
        for(int x = 0; x < FACE_SIZE; x++) {
            unsigned char c = px[x], *o = img->px + (row * FACE_SIZE + x) * 4;
            if(c != '.' && !known[c]) {
                fprintf(stderr, "%s: row %d: no color '%c'\n", fn, row, c);
                fclose(in);
                return false;
            }
            if(c != '.') {
                o[0] = pal[c][0], o[1] = pal[c][1], o[2] = pal[c][2], o[3] = 255;
            }
        }
        row++;
    }
    fclose(in);
    return true;
}

// Area-averaged resize of the source rectangle (sx, sy, sw, sh) to w x h, alpha weighted.
static rgba resample(const rgba *src, float sx, float sy, float sw, float sh, int w, int h) {
    rgba out = {w, h, calloc(w * h * 4, 1)};
    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            float x0 = sx + sw * x / w, x1 = sx + sw * (x + 1) / w;
            float y0 = sy + sh * y / h, y1 = sy + sh * (y + 1) / h;
            double sum[4] = {0}, weight = 0;
            for(int py = (int)y0; py < (int)(y1 + 0.999f) && py < src->h; py++) {
                for(int px = (int)x0; px < (int)(x1 + 0.999f) && px < src->w; px++) {
                    if(px < 0 || py < 0) {
                        continue;
                    }
                    float wx = (px + 1 < x1 ? px + 1 : x1) - (px > x0 ? px : x0);
                    float wy = (py + 1 < y1 ? py + 1 : y1) - (py > y0 ? py : y0);
                    float a = wx * wy;
                    const unsigned char *p = src->px + (py * src->w + px) * 4;
                    float alpha = p[3] / 255.0f;
                    for(int c = 0; c < 3; c++) {
                        sum[c] += p[c] * a * alpha;
                    }
                    sum[3] += alpha * a;
                    weight += a;
                }
            }
            unsigned char *o = out.px + (y * w + x) * 4;
            if(sum[3] > 0) {
                for(int c = 0; c < 3; c++) {
                    o[c] = (unsigned char)(sum[c] / sum[3] + 0.5);
                }
            }
            o[3] = weight > 0 ? (unsigned char)(255 * sum[3] / weight + 0.5) : 0;
        }
    }
    return out;
}

static int color_distance(vga_color a, int r, int g, int b) {
    int rm = (a.r + r) / 2;
    int dr = a.r - r, dg = a.g - g, db = a.b - b;
    return (((512 + rm) * dr * dr) >> 8) + 4 * dg * dg + (((767 - rm) * db * db) >> 8);
}

// The palette entries a scene keeps fixed: what the new pictures may use.
#define FIXED_FIRST 0x60
#define FIXED_LAST 0xEF
static void fixed_indices(bool allowed[256]) {
    for(int k = 0; k < 256; k++) {
        allowed[k] = k >= FIXED_FIRST && k <= FIXED_LAST;
    }
}

// A sprite from the image, in the palette's `allowed` colors; `background` fills what is
// transparent in the image (0 keeps it transparent).
static void to_sprite(sd_sprite *s, const rgba *img, const vga_palette *pal, const bool allowed[256],
                      int background, vec2i pos) {
    sd_vga_image vga;
    sd_vga_image_create(&vga, img->w, img->h);
    for(int i = 0; i < img->w * img->h; i++) {
        const unsigned char *p = img->px + i * 4;
        int best = background, best_d = 1 << 30;
        if(p[3] >= 128) {
            for(int k = 1; k < 256; k++) {
                int d = allowed[k] ? color_distance(pal->colors[k], p[0], p[1], p[2]) : 1 << 30;
                if(d < best_d) {
                    best_d = d;
                    best = k;
                }
            }
        }
        vga.data[i] = (char)best;
    }
    sd_sprite_create(s);
    sd_sprite_vga_encode(s, &vga);
    s->pos = pos;
    sd_vga_image_free(&vga);
}

// Sets sprite `index`, appending copies of `filler` until the animation is that long.
static void put_sprite(sd_animation *ani, int index, sd_sprite *s, int filler) {
    while(sd_animation_get_sprite_count(ani) < index) {
        sd_sprite copy;
        sd_sprite_copy(&copy, sd_animation_get_sprite(ani, filler));
        copy.index = sd_animation_get_sprite_count(ani);
        sd_animation_push_sprite(ani, &copy);
        sd_sprite_free(&copy);
    }
    s->index = index;
    if(sd_animation_get_sprite_count(ani) > index) {
        sd_animation_set_sprite(ani, index, s); // rebuilt over a previous run
    } else {
        sd_animation_push_sprite(ani, s);
    }
}

static int add_pilots(const char *dir, const char *bk_name, bool with_icons, const rgba faces[], const rgba icons[]) {
    char fn[1024];
    path p;
    sd_bk_file bk;
    sd_bk_create(&bk);
    snprintf(fn, sizeof(fn), "%s/%s", dir, bk_name);
    path_from_c(&p, fn);
    if(sd_bk_load(&bk, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot read %s\n", fn);
        return 1;
    }
    const vga_palette *pal = bk.palettes[0];
    bool used[256];

    sd_animation *face_ani = sd_bk_get_anim(&bk, FACES)->animation;
    fixed_indices(used);
    vec2i face_pos = sd_animation_get_sprite(face_ani, 0)->pos;
    for(int n = 0; n < NEW_PILOTS; n++) {
        sd_sprite s;
        to_sprite(&s, &faces[n], pal, used, 0, face_pos);
        put_sprite(face_ani, PILOT_ID(n), &s, 0);
        sd_sprite_free(&s);
    }

    if(with_icons) {
        sd_animation *icon_ani = sd_bk_get_anim(&bk, ICONS)->animation;
        // the icons' black: the darkest fixed color
        int black = 1, black_d = 1 << 30;
        for(int k = 1; k < 256; k++) {
            int d = used[k] ? color_distance(pal->colors[k], 0, 0, 0) : 1 << 30;
            if(d < black_d) {
                black_d = d;
                black = k;
            }
        }
        vec2i icon_pos = sd_animation_get_sprite(icon_ani, 0)->pos;
        for(int n = 0; n < NEW_PILOTS; n++) {
            sd_sprite s;
            to_sprite(&s, &icons[n], pal, used, black, icon_pos);
            put_sprite(icon_ani, PILOT_ID(n), &s, 0); // index 10 (Kreissack) gets a copy
            sd_sprite_free(&s);
        }
    }

    if(sd_bk_save(&bk, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot write %s\n", fn);
        return 1;
    }
    sd_bk_free(&bk);
    printf("wrote %s (%d pilots)\n", fn, NEW_PILOTS);
    return 0;
}

// --- PILOTS.EXT: each pilot's pictures in up to 64 colors of its own ---------------------

#define PILOT_COLORS 32 // the select screen gives each of its 12 cells 32 colors

typedef struct {
    unsigned char r, g, b;
} rgb;

static int rgb_distance(rgb a, rgb b) {
    return color_distance((vga_color){a.r, a.g, a.b}, b.r, b.g, b.b);
}

// k-means over the pictures' opaque pixels (the icon's transparent parts are its black)
static int pilot_palette(const rgba *face, const rgba *icon, rgb colors[PILOT_COLORS]) {
    int n = face->w * face->h + icon->w * icon->h;
    rgb *px = malloc(sizeof(rgb) * n);
    int count = 0;
    for(int i = 0; i < face->w * face->h; i++) {
        const unsigned char *p = face->px + i * 4;
        if(p[3] >= 128) {
            px[count++] = (rgb){p[0], p[1], p[2]};
        }
    }
    for(int i = 0; i < icon->w * icon->h; i++) {
        const unsigned char *p = icon->px + i * 4;
        px[count++] = p[3] >= 128 ? (rgb){p[0], p[1], p[2]} : (rgb){0, 0, 0};
    }
    // seeds: each the pixel farthest from those chosen so far, so every hue gets one (in pixel
    // order, the hair's many shades would take them all before the skin), then refine
    int k = 0;
    int *nearest_d = malloc(sizeof(int) * count);
    for(int i = 0; i < count; i++) {
        nearest_d[i] = 1 << 30;
    }
    int pick = 0;
    while(k < PILOT_COLORS) {
        colors[k++] = px[pick];
        int far = -1;
        for(int i = 0; i < count; i++) {
            int d = rgb_distance(colors[k - 1], px[i]);
            nearest_d[i] = d < nearest_d[i] ? d : nearest_d[i];
            if(far < 0 || nearest_d[i] > nearest_d[far]) {
                far = i;
            }
        }
        if(far < 0 || nearest_d[far] == 0) {
            break; // every pixel already has its exact color
        }
        pick = far;
    }
    for(int iter = 0; iter < 20; iter++) {
        double sum[PILOT_COLORS][3] = {{0}};
        int members[PILOT_COLORS] = {0};
        for(int i = 0; i < count; i++) {
            int best = 0, best_d = 1 << 30;
            for(int j = 0; j < k; j++) {
                int d = rgb_distance(colors[j], px[i]);
                if(d < best_d) {
                    best_d = d;
                    best = j;
                }
            }
            sum[best][0] += px[i].r;
            sum[best][1] += px[i].g;
            sum[best][2] += px[i].b;
            members[best]++;
        }
        for(int j = 0; j < k; j++) {
            if(members[j]) {
                colors[j] = (rgb){(unsigned char)(sum[j][0] / members[j] + 0.5), (unsigned char)(sum[j][1] / members[j] + 0.5),
                                  (unsigned char)(sum[j][2] / members[j] + 0.5)};
            }
        }
    }
    free(nearest_d);
    free(px);
    return k;
}

static int nearest(const rgb colors[], int k, rgb c) {
    int best = 0, best_d = 1 << 30;
    for(int j = 0; j < k; j++) {
        int d = rgb_distance(colors[j], c);
        if(d < best_d) {
            best_d = d;
            best = j;
        }
    }
    return best;
}

static void put16(FILE *out, unsigned v) {
    fputc(v & 0xFF, out);
    fputc(v >> 8, out);
}

// a picture as color numbers 1..64 (0: transparent; with `opaque`, transparent is black)
static void put_picture(FILE *out, const rgba *img, const rgb colors[], int k, bool opaque) {
    put16(out, img->w);
    put16(out, img->h);
    for(int i = 0; i < img->w * img->h; i++) {
        const unsigned char *p = img->px + i * 4;
        if(p[3] < 128 && !opaque) {
            fputc(0, out);
        } else {
            rgb c = p[3] >= 128 ? (rgb){p[0], p[1], p[2]} : (rgb){0, 0, 0};
            fputc(1 + nearest(colors, k, c), out);
        }
    }
}

// Format: see game/utils/har_extended.c.
static int write_pictures(const char *dir, const rgba faces[], const rgba icons[]) {
    char fn[1024];
    snprintf(fn, sizeof(fn), "%s/PILOTS.EXT", dir);
    FILE *out = fopen(fn, "wb");
    if(!out) {
        fprintf(stderr, "cannot write %s\n", fn);
        return 1;
    }
    fwrite("OMFPIL\0\1", 1, 8, out);
    fputc(NEW_PILOTS, out);
    for(int n = 0; n < NEW_PILOTS; n++) {
        rgb colors[PILOT_COLORS];
        int k = pilot_palette(&faces[n], &icons[n], colors);
        fputc(PILOT_ID(n), out);
        fputc(k, out);
        for(int j = 0; j < k; j++) {
            fputc(colors[j].r, out);
            fputc(colors[j].g, out);
            fputc(colors[j].b, out);
        }
        put_picture(out, &faces[n], colors, k, false);
        put_picture(out, &icons[n], colors, k, true);
        printf("%s: %d colors\n", pilot_art[n], k);
    }
    fclose(out);
    printf("wrote %s\n", fn);
    return 0;
}

int main(int argc, char **argv) {
    if(argc != 3) {
        fprintf(stderr, "usage: %s <resources-dir> <art-dir>\n", argv[0]);
        return 2;
    }
    rgba faces[NEW_PILOTS], icons[NEW_PILOTS];
    for(int n = 0; n < NEW_PILOTS; n++) {
        rgba art;
        if(pix_load(&faces[n], argv[2], pilot_art[n])) {
            // drawn at the face's size: used as it is, and the icon is a piece of it
            icons[n] = resample(&faces[n], PIX_ICON_X, PIX_ICON_Y, ICON_W, ICON_H, ICON_W, ICON_H);
            continue;
        }
        if(!rgba_load(&art, argv[2], pilot_art[n])) {
            return 1;
        }
        if(art.w != ART_SIZE || art.h != ART_SIZE) {
            fprintf(stderr, "%s.png is %dx%d, expected %dx%d\n", pilot_art[n], art.w, art.h, ART_SIZE, ART_SIZE);
            return 1;
        }
        // the head filling the picture, as the original pictures frame it
        faces[n] = resample(&art, 46, 32, 144, 144, FACE_SIZE, FACE_SIZE);
        // the face, at the icons' 51:36
        const float crop_w = 200, crop_h = crop_w * ICON_H / ICON_W;
        icons[n] = resample(&art, 24, 34, crop_w, crop_h, ICON_W, ICON_H);
        free(art.px);
    }
    return write_pictures(argv[1], faces, icons) || add_pilots(argv[1], "MELEE.BK", true, faces, icons) ||
           add_pilots(argv[1], "VS.BK", false, faces, icons);
}

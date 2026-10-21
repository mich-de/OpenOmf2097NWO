// Builds OCELOT, a twelfth robot, from JAGUAR's game files and the hand-drawn frames
// in tools/web/ocelot/art:
//
//   ocelot <resources-dir> <art-dir>
//
// reads  FIGHTR0.AF (Jaguar), ALTPALS.DAT, VS.BK, <art-dir>/mMM_fFF.png
// writes FIGHTR11.AF (Ocelot's moves, Jaguar's 8-bit pixels as a fallback),
//        FIGHTR11.X16 (its 16-bit pixels and own colors, see game/utils/har_extended.h)
//        and VS.BK with Ocelot's portrait appended as sprite 11
//
// The art is Jaguar's frames, exported 4x in pilot Milano's colors and repainted
// (a jaguar's coat, a cat's head, a tail): one PNG per sprite, the same size as the
// sprite at 4x, transparent where the sprite is. Each 4x4 block becomes one pixel.
// Its colors become Ocelot's own palette, up to 383 colors, kept exactly when the art
// has no more than that: in FIGHTR11.X16 a pixel of value 257 + k is color k, and values
// below 256 keep meaning the colors every arena shares (the fire).
//
// Sprites without art (body pieces) keep Jaguar's pixels, mapped through Milano's
// colors onto Ocelot's palette. The concussion cannon becomes a fire spit.
//
// Palette notes (indices 160..249 are shared by every arena, MELEE.BK and VS.BK):
//   1..47     Jaguar's HAR colors (P2 is drawn +48), 32..47 its plating
//   257..639  Ocelot's own colors (16-bit pixels; P2's are drawn +384)
//   176..183 red, 240..243 orange to yellow, 207 pale yellow, 223 white: the fire
#include <math.h>
#include <png.h>
#include <zlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats/af.h"
#include "formats/altpal.h"
#include "formats/bk.h"
#include "formats/bkanim.h"
#include "formats/error.h"
#include "formats/move.h"
#include "formats/sprite.h"
#include "formats/vga_image.h"
#include "utils/path.h"
#include "utils/vector.h"

#define HAR_COLORS 48
#define OCELOT_COLORS 383 // the extended palette has room for this many per player
#define EXTENDED_BASE 257 // pixel value of Ocelot's color 0
#define PORTRAIT_COLORS 159 // an 8-bit portrait reaches its first 159 colors (bytes 1..159)
#define SHARED_FIRST 160  // the colors every arena, MELEE.BK and VS.BK share
#define SHARED_LAST 249
#define PLATE_FIRST 32
#define PLATE_LAST 47
#define ART_SCALE 4

#define MOVE_CANNON 21     // Jaguar's concussion cannon: D, DB, B + punch
#define MOVE_SHOT 38       // its projectile
#define MOVE_SHOT_HIT 37   // the projectile's impact
#define VS_HAR_PORTRAITS 5 // VS.BK animation holding one big portrait per HAR
#define OCELOT_ID 11

// pilot Milano's colors, which the art was painted over: altpal ramps per HAR ramp
static const int milano_ramps[3] = {7, 12, 11};

static vga_color milano[HAR_COLORS];
static vga_color ocelot[OCELOT_COLORS];
static int ocelot_count = 0;

// --- colors ---------------------------------------------------------------------

static int color_distance(vga_color a, vga_color b) {
    // "redmean" weighting: closer to what the eye sees than plain RGB distance
    int rm = (a.r + b.r) / 2;
    int dr = a.r - b.r, dg = a.g - b.g, db = a.b - b.b;
    return (((512 + rm) * dr * dr) >> 8) + 4 * dg * dg + (((767 - rm) * db * db) >> 8);
}

// Index of the closest of Ocelot's first `limit` colors.
static int nearest_ocelot_in(vga_color c, int limit) {
    int best = 0, best_d = 1 << 30;
    for(int i = 0; i < ocelot_count && i < limit; i++) {
        int d = color_distance(c, ocelot[i]);
        if(d < best_d) {
            best_d = d;
            best = i;
        }
    }
    return best;
}

static int nearest_ocelot(vga_color c) {
    return nearest_ocelot_in(c, OCELOT_COLORS);
}

typedef struct {
    vga_color color;
    int count;
} color_count;

typedef struct {
    color_count *items;
    int n;
} color_set;

static void color_set_add(color_set *set, vga_color c) {
    for(int i = 0; i < set->n; i++) {
        if(set->items[i].color.r == c.r && set->items[i].color.g == c.g && set->items[i].color.b == c.b) {
            set->items[i].count++;
            return;
        }
    }
    set->items = realloc(set->items, sizeof(color_count) * (set->n + 1));
    set->items[set->n++] = (color_count){c, 1};
}

static int nearest_cluster(const color_set *set, vga_color c) {
    int best = 0, best_d = 1 << 30;
    for(int i = 0; i < set->n; i++) {
        int d = color_distance(c, set->items[i].color);
        if(d < best_d) {
            best_d = d;
            best = i;
        }
    }
    return best;
}

static int by_count(const void *a, const void *b) {
    return ((const color_count *)b)->count - ((const color_count *)a)->count;
}

// Ocelot's colors: the art's own, or k-means over them when there are more than 383.
// Most used first, so the 8-bit portrait's 159 get the colors that matter.
static void build_palette(const color_set *set) {
    const int k = OCELOT_COLORS;
    if(set->n <= k) {
        color_count *sorted = malloc(sizeof(color_count) * set->n);
        memcpy(sorted, set->items, sizeof(color_count) * set->n);
        qsort(sorted, set->n, sizeof(color_count), by_count);
        for(int i = 0; i < set->n; i++) {
            ocelot[i] = sorted[i].color;
        }
        ocelot_count = set->n;
        free(sorted);
        return;
    }
    vga_color centers[OCELOT_COLORS];
    int used = k;
    // farthest point seeding, starting from the most used color
    int first = 0;
    for(int i = 1; i < set->n; i++) {
        if(set->items[i].count > set->items[first].count) {
            first = i;
        }
    }
    centers[0] = set->items[first].color;
    for(int c = 1; c < used; c++) {
        int pick = 0, pick_d = -1;
        for(int i = 0; i < set->n; i++) {
            int d = 1 << 30;
            for(int j = 0; j < c; j++) {
                int dj = color_distance(set->items[i].color, centers[j]);
                d = dj < d ? dj : d;
            }
            if(d > pick_d) {
                pick_d = d;
                pick = i;
            }
        }
        centers[c] = set->items[pick].color;
    }
    for(int iter = 0; iter < 30; iter++) {
        double sum[OCELOT_COLORS][3] = {{0}};
        double weight[OCELOT_COLORS] = {0};
        for(int i = 0; i < set->n; i++) {
            int best = 0, best_d = 1 << 30;
            for(int c = 0; c < used; c++) {
                int d = color_distance(set->items[i].color, centers[c]);
                if(d < best_d) {
                    best_d = d;
                    best = c;
                }
            }
            sum[best][0] += set->items[i].color.r * (double)set->items[i].count;
            sum[best][1] += set->items[i].color.g * (double)set->items[i].count;
            sum[best][2] += set->items[i].color.b * (double)set->items[i].count;
            weight[best] += set->items[i].count;
        }
        for(int c = 0; c < used; c++) {
            if(weight[c] > 0) {
                centers[c] = (vga_color){(uint8_t)lround(sum[c][0] / weight[c]), (uint8_t)lround(sum[c][1] / weight[c]),
                                         (uint8_t)lround(sum[c][2] / weight[c])};
            }
        }
    }
    color_set clusters = {NULL, 0};
    for(int c = 0; c < used; c++) {
        color_set_add(&clusters, centers[c]);
    }
    for(int i = 0; i < set->n; i++) {
        clusters.items[nearest_cluster(&clusters, set->items[i].color)].count += set->items[i].count;
    }
    build_palette(&clusters); // now within the limit: sorts by use
    free(clusters.items);
}

// --- the art ----------------------------------------------------------------------

typedef struct {
    int w, h;           // at 1x
    unsigned char *px;  // RGBA at 1x
    int mixed_blocks;   // 4x4 blocks that were not one color: the art has finer detail
    unsigned char *big; // RGBA at 4x, kept when it has finer detail (an HD frame), else NULL
} art_frame;

static bool art_load(art_frame *a, const char *dir, int move, int frame) {
    char fn[1024];
    snprintf(fn, sizeof(fn), "%s/m%02d_f%02d.png", dir, move, frame);
    png_image img;
    memset(&img, 0, sizeof(img));
    img.version = PNG_IMAGE_VERSION;
    if(!png_image_begin_read_from_file(&img, fn)) {
        return false;
    }
    img.format = PNG_FORMAT_RGBA;
    unsigned char *big = malloc(PNG_IMAGE_SIZE(img));
    if(!png_image_finish_read(&img, NULL, big, 0, NULL)) {
        free(big);
        return false;
    }
    a->w = img.width / ART_SCALE;
    a->h = img.height / ART_SCALE;
    a->px = malloc(a->w * a->h * 4);
    a->mixed_blocks = 0;
    for(int y = 0; y < a->h; y++) {
        for(int x = 0; x < a->w; x++) {
            const unsigned char *src = big + ((y * ART_SCALE) * img.width + x * ART_SCALE) * 4;
            memcpy(a->px + (y * a->w + x) * 4, src, 4);
            for(int by = 0; by < ART_SCALE; by++) {
                if(memcmp(src, big + ((y * ART_SCALE + by) * img.width + x * ART_SCALE) * 4, 4 * ART_SCALE) != 0) {
                    a->mixed_blocks++;
                    break;
                }
            }
        }
    }
    a->big = a->mixed_blocks ? big : NULL;
    if(!a->big) {
        free(big);
    }
    return true;
}

// The art's pixels as stored: 4x when it has finer detail, 1x when it is the game's pixels enlarged.
static int art_scale(const art_frame *a) {
    return a->big ? ART_SCALE : 1;
}

static const unsigned char *art_pixels(const art_frame *a) {
    return a->big ? a->big : a->px;
}

// --- sprites as paletted canvases ---------------------------------------------------

typedef struct {
    int x, y; // position of the top left pixel, relative to the HAR's origin
    int w, h;
    unsigned char *px; // 0 is transparent
} canvas;

static void canvas_from_sprite(canvas *c, const sd_sprite *s) {
    sd_vga_image img;
    sd_sprite_vga_decode(&img, s);
    c->x = s->pos.x;
    c->y = s->pos.y;
    c->w = img.w;
    c->h = img.h;
    c->px = malloc(c->w * c->h);
    memcpy(c->px, img.data, c->w * c->h);
    sd_vga_image_free(&img);
}

static void canvas_to_sprite(const canvas *c, sd_sprite *s) {
    sd_vga_image img;
    sd_vga_image_create(&img, c->w, c->h);
    memcpy(img.data, c->px, c->w * c->h);
    uint8_t index = s->index;
    sd_sprite_free(s);
    sd_sprite_create(s);
    sd_sprite_vga_encode(s, &img);
    s->index = index;
    s->pos.x = c->x;
    s->pos.y = c->y;
    sd_vga_image_free(&img);
}

// --- FIGHTR11.X16 ------------------------------------------------------------------

typedef struct {
    int move, sprite, scale, w, h;
    uint16_t *px;
} x16_frame;

// A sprite's 16-bit pixels: the art in Ocelot's colors; without art, Jaguar's HAR colors
// as Milano wore them, mapped onto Ocelot's; everything else (the fire) as it is.
static x16_frame x16_from(int move, int sprite, const canvas *c, const art_frame *a) {
    int scale = a ? art_scale(a) : 1;
    int w = c->w * scale, h = c->h * scale;
    x16_frame f = {move, sprite, scale, w, h, malloc(sizeof(uint16_t) * w * h)};
    for(int i = 0; i < w * h; i++) {
        if(a) {
            const unsigned char *p = art_pixels(a) + i * 4;
            f.px[i] = p[3] < 128 ? 0 : EXTENDED_BASE + nearest_ocelot((vga_color){p[0], p[1], p[2]});
        } else if(c->px[i] > 0 && c->px[i] < HAR_COLORS) {
            f.px[i] = EXTENDED_BASE + nearest_ocelot(milano[c->px[i]]);
        } else {
            f.px[i] = c->px[i];
        }
    }
    return f;
}

typedef struct {
    unsigned char *data;
    size_t size, room;
} bytes;

static void put8(bytes *b, unsigned v) {
    if(b->size == b->room) {
        b->room = b->room ? b->room * 2 : 65536;
        b->data = realloc(b->data, b->room);
    }
    b->data[b->size++] = v & 0xFF;
}

static void put16(bytes *b, unsigned v) {
    put8(b, v);
    put8(b, v >> 8);
}

// Format: see game/utils/har_extended.c.
static bool x16_write(const char *fn, const x16_frame *frames, int count) {
    bytes body = {NULL, 0, 0};
    put16(&body, ocelot_count);
    for(int i = 0; i < ocelot_count; i++) {
        put8(&body, ocelot[i].r);
        put8(&body, ocelot[i].g);
        put8(&body, ocelot[i].b);
    }
    put16(&body, count);
    for(int f = 0; f < count; f++) {
        const x16_frame *fr = &frames[f];
        put8(&body, fr->move);
        put8(&body, fr->sprite);
        put8(&body, fr->scale);
        put16(&body, fr->w);
        put16(&body, fr->h);
        int size = fr->w * fr->h;
        for(int at = 0; at < size;) {
            int skip = 0, n = 0;
            while(at + skip < size && !fr->px[at + skip] && skip < 0xFFFF) {
                skip++;
            }
            while(at + skip + n < size && fr->px[at + skip + n] && n < 0xFFFF) {
                n++;
            }
            put16(&body, skip);
            put16(&body, n);
            for(int k = 0; k < n; k++) {
                put16(&body, fr->px[at + skip + k]);
            }
            at += skip + n;
        }
    }
    uLongf packed_size = compressBound(body.size);
    unsigned char *packed = malloc(packed_size);
    bool ok = compress2(packed, &packed_size, body.data, body.size, Z_BEST_COMPRESSION) == Z_OK;
    FILE *out = ok ? fopen(fn, "wb") : NULL;
    if(out) {
        fwrite("OMFX16\0\2", 1, 8, out);
        unsigned char size[4] = {packed_size & 0xFF, (packed_size >> 8) & 0xFF, (packed_size >> 16) & 0xFF,
                                 (packed_size >> 24) & 0xFF};
        fwrite(size, 1, 4, out);
        fwrite(packed, 1, packed_size, out);
        ok = fclose(out) == 0;
    }
    printf("%d frames of 16-bit pixels, %zu bytes, %lu packed\n", count, body.size, (unsigned long)packed_size);
    free(packed);
    free(body.data);
    return out && ok;
}

// The 8-bit VS portrait reaches Ocelot's first 159 colors, as bytes 1..159.
static unsigned char portrait_color(vga_color c) {
    return 1 + nearest_ocelot_in(c, PORTRAIT_COLORS);
}

// Jaguar's HAR colors, as Milano wore them, onto the portrait's colors; other indices stay.
static void remap_har_colors(canvas *c) {
    for(int i = 0; i < c->w * c->h; i++) {
        if(c->px[i] > 0 && c->px[i] < HAR_COLORS) {
            c->px[i] = portrait_color(milano[c->px[i]]);
        }
    }
}

// --- the VS portrait's coat ------------------------------------------------------------

static uint32_t hash2(int x, int y, uint32_t seed) {
    uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u + seed * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

static float hash01(int x, int y, uint32_t seed) {
    return (hash2(x, y, seed) & 0xFFFF) / 65535.0f;
}

// Spots like the art's: dense, irregular specks. '#' spot, 'b' brown rim, 'o' darker fur.
#define STAMP 3
static const char *stamps[][STAMP] = {
    {"##.", "#b.", "..."}, {".#.", "##.", ".b."}, {"###", "b..", "..."},
    {"#..", "#..", "b.."}, {"##.", ".##", "..."}, {".o.", "##o", "..."},
};
#define STAMP_COUNT (int)(sizeof(stamps) / sizeof(stamps[0]))

// The art's fur, from shadow to light: plating shades are matched to it, not to Milano.
static unsigned char portrait_fur(int shade) {
    static const vga_color stops[4] = {{47, 31, 16}, {141, 107, 60}, {199, 159, 92}, {237, 194, 114}};
    float t = (shade < 0 ? 0 : shade > 15 ? 15 : shade) / 15.0f * 3.0f;
    int i = t >= 3.0f ? 2 : (int)t;
    float f = t - i;
    vga_color a = stops[i], b = stops[i + 1];
    return portrait_color((vga_color){(uint8_t)(a.r + (b.r - a.r) * f), (uint8_t)(a.g + (b.g - a.g) * f),
                                      (uint8_t)(a.b + (b.b - a.b) * f)});
}

// The VS portrait has no hand-drawn version: Jaguar's, in Ocelot's colors, with spots
// scattered over the plating.
static void paint_portrait_coat(canvas *c) {
    unsigned char *src = malloc((size_t)c->w * c->h);
    memcpy(src, c->px, c->w * c->h);
    unsigned char spot = portrait_color((vga_color){26, 15, 11});
    unsigned char rim = portrait_color((vga_color){78, 44, 16});
    const int sx = 4, sy = 3;
    remap_har_colors(c);
    for(int i = 0; i < c->w * c->h; i++) {
        if(src[i] >= PLATE_FIRST && src[i] <= PLATE_LAST) {
            c->px[i] = portrait_fur(src[i] - PLATE_FIRST);
        }
    }
    for(int oy = sy / 2; oy < c->h; oy += sy) {
        for(int ox = ((oy / sy) & 1 ? sx / 2 : 0) + 2; ox < c->w; ox += sx) {
            if(src[oy * c->w + ox] < PLATE_FIRST + 3 || src[oy * c->w + ox] > PLATE_LAST) {
                continue;
            }
            int pick = hash2(ox, oy, 3) % STAMP_COUNT;
            int jx = (int)(hash01(ox, oy, 4) * 3) - 1, jy = (int)(hash01(ox, oy, 5) * 3) - 1;
            if(hash01(ox, oy, 6) < 0.3f) {
                continue; // gaps keep it from looking like a grid
            }
            for(int y = 0; y < STAMP; y++) {
                for(int x = 0; x < STAMP; x++) {
                    char t = stamps[pick][y][x];
                    int px = ox - STAMP / 2 + x + jx, py = oy - STAMP / 2 + y + jy;
                    if(t == '.' || px < 0 || py < 0 || px >= c->w || py >= c->h) {
                        continue;
                    }
                    int k = py * c->w + px;
                    if(src[k] < PLATE_FIRST || src[k] > PLATE_LAST) {
                        continue; // spots stay on the plating
                    }
                    c->px[k] = t == '#' ? spot : t == 'b' ? rim : portrait_fur(src[k] - PLATE_FIRST - 4);
                }
            }
        }
    }
    free(src);
}

// --- the fire spit ----------------------------------------------------------------

static unsigned char fire(float heat) {
    static const unsigned char ramp[] = {177, 180, 183, 240, 241, 242, 243, 207, 223};
    int i = (int)(heat * 8.999f);
    return ramp[i < 0 ? 0 : i > 8 ? 8 : i];
}

// A ball of fire flying right, its flames trailing left.
static void draw_spit(canvas *c, int cx, int cy, int variant) {
    const int w = 30, h = 15;
    c->x = cx - w + 7;
    c->y = cy - h / 2;
    c->w = w;
    c->h = h;
    c->px = calloc(w * h, 1);
    float bx = w - 7.5f, by = h / 2.0f;
    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            float dx = x + 0.5f - bx, dy = y + 0.5f - by;
            float heat;
            if(dx >= 0) {
                heat = 1.0f - hypotf(dx, dy) / 6.5f; // the ball
            } else {
                // the trail: narrows and cools with distance, flickers per variant
                float flick = 0.35f * sinf(-dx * 0.9f + variant * 2.1f) + 0.25f * sinf(-dx * 0.37f + variant);
                float width = 6.5f * (1.0f + dx / (w - 8.0f)) + flick;
                heat = (1.0f - fabsf(dy + flick * 0.8f) / (width > 0.5f ? width : 0.5f)) * (1.0f + dx / (w - 6.0f));
            }
            if(heat <= 0) {
                continue;
            }
            float value = heat + hash01(x, y, 7 + variant) * 0.25f - 0.1f;
            if(value > 0.08f) {
                c->px[y * w + x] = fire(value);
            }
        }
    }
}

static void recolor_cannon_to_fire(canvas *c) {
    // the impact uses the cannon's cyan ramp 224..231
    static const unsigned char map[8] = {177, 180, 183, 240, 241, 242, 243, 207};
    for(int i = 0; i < c->w * c->h; i++) {
        if(c->px[i] >= 224 && c->px[i] <= 231) {
            c->px[i] = map[c->px[i] - 224];
        }
    }
}

// Every other frame of the shot shows the second flicker sprite.
static void alternate_shot_frames(str *anim_string) {
    const char *src = str_c(anim_string);
    size_t n = strlen(src);
    char *out = malloc(n + 1);
    memcpy(out, src, n + 1);
    int frame = 0;
    for(size_t i = 0; i < n; i++) {
        if(out[i] == 'A' && (i + 1 >= n || (out[i + 1] >= '0' && out[i + 1] <= '9'))) {
            if(frame++ % 2) {
                out[i] = 'B';
            }
        }
    }
    str_set_c(anim_string, out);
    free(out);
}

// Moves the projectile spawn ("mx+60my-59" in the cannon's string) to the given point.
static int move_spawn_point(str *anim_string, int x, int y) {
    const char *src = str_c(anim_string);
    const char *at = strstr(src, "mx");
    if(!at) {
        return 1;
    }
    const char *end = at + 2;
    end += strspn(end, "+-0123456789");
    if(strncmp(end, "my", 2) != 0) {
        return 1;
    }
    end += 2 + strspn(end + 2, "+-0123456789");
    char buf[1024];
    snprintf(buf, sizeof(buf), "%.*smx%+dmy%+d%s", (int)(at - src), src, x, y, end);
    str_set_c(anim_string, buf);
    return 0;
}

// The mouth: the frontmost pixel near the top of the head, in the frame where the shot leaves.
static void find_mouth(const canvas *c, int *mx, int *my) {
    int top = -1;
    for(int y = 0; y < c->h && top < 0; y++) {
        for(int x = 0; x < c->w; x++) {
            if(c->px[y * c->w + x]) {
                top = y;
                break;
            }
        }
    }
    int front = 0, front_y = top;
    for(int y = top; y < top + 10 && y < c->h; y++) {
        for(int x = c->w - 1; x >= 0; x--) {
            if(c->px[y * c->w + x]) {
                if(x > front) {
                    front = x;
                    front_y = y;
                }
                break;
            }
        }
    }
    *mx = c->x + front;
    *my = c->y + front_y + 3;
}

static int frame_of_spawn(const sd_animation *ani) {
    // the frame letter just before the spawn tag
    const char *s = str_c(&ani->anim_string);
    const char *at = strstr(s, "mx");
    for(const char *p = at; p && p > s; p--) {
        if(p[-1] >= 'A' && p[-1] <= 'Z') {
            return p[-1] - 'A';
        }
    }
    return 0;
}

// --- the robot ----------------------------------------------------------------------

static bool is_fire(int m) {
    return m == MOVE_SHOT || m == MOVE_SHOT_HIT;
}

static int make_ocelot(const char *dir, const char *art_dir) {
    char fn[1024];
    path p;

    altpal_file altpal;
    altpal_create(&altpal);
    snprintf(fn, sizeof(fn), "%s/ALTPALS.DAT", dir);
    path_from_c(&p, fn);
    if(altpals_load(&altpal, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot read %s\n", fn);
        return 1;
    }
    for(int r = 0; r < 3; r++) {
        memcpy(&milano[r * 16], &altpal.palettes[0].colors[milano_ramps[r] * 16], 16 * sizeof(vga_color));
    }

    sd_af_file af;
    sd_af_create(&af);
    snprintf(fn, sizeof(fn), "%s/FIGHTR0.AF", dir);
    path_from_c(&p, fn);
    if(sd_af_load(&af, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot read %s\n", fn);
        return 1;
    }
    af.fighter_id = OCELOT_ID;

    // the art, and its colors
    art_frame *art[70][32] = {{NULL}};
    color_set colors = {NULL, 0};
    int frames = 0, mixed = 0;
    for(int m = 0; m < 70; m++) {
        sd_move *move = sd_af_get_move(&af, m);
        if(!move || is_fire(m)) {
            continue;
        }
        for(int i = 0; i < sd_animation_get_sprite_count(move->animation) && i < 32; i++) {
            const sd_sprite *s = sd_animation_get_sprite(move->animation, i);
            art_frame a;
            if(s->missing || !art_load(&a, art_dir, m, i)) {
                continue;
            }
            if(a.w != s->width || a.h != s->height) {
                fprintf(stderr, "m%02d_f%02d.png is %dx%d, the sprite is %dx%d (x%d)\n", m, i, a.w * ART_SCALE,
                        a.h * ART_SCALE, s->width * ART_SCALE, s->height * ART_SCALE, ART_SCALE);
                return 1;
            }
            const unsigned char *px = art_pixels(&a);
            int pixel_count = a.w * a.h * art_scale(&a) * art_scale(&a);
            for(int k = 0; k < pixel_count; k++) {
                if(px[k * 4 + 3] >= 128) {
                    color_set_add(&colors, (vga_color){px[k * 4], px[k * 4 + 1], px[k * 4 + 2]});
                }
            }
            mixed += a.mixed_blocks;
            art[m][i] = malloc(sizeof(art_frame));
            *art[m][i] = a;
            frames++;
        }
    }
    if(!frames) {
        fprintf(stderr, "no art in %s\n", art_dir);
        return 1;
    }
    build_palette(&colors);
    printf("%d frames of art, %d colors: %d in Ocelot's palette", frames, colors.n, ocelot_count);
    if(mixed) {
        printf(" (frames with finer detail than 4x4 blocks are kept at 4x)");
    }
    printf("\n");

    x16_frame *frames_x16 = NULL;
    int frame_count = 0;
    for(int m = 0; m < 70; m++) {
        sd_move *move = sd_af_get_move(&af, m);
        if(!move) {
            continue;
        }
        sd_animation *ani = move->animation;
        int count = sd_animation_get_sprite_count(ani);

        if(m == MOVE_SHOT) {
            // a fireball in place of the cannon's beam, centered where the beam was
            // (shared colors only: the 8-bit sprites need no 16-bit copy)
            sd_sprite *old = sd_animation_get_sprite(ani, 0);
            int cx = old->pos.x + old->width / 2, cy = old->pos.y + old->height / 2;
            for(int variant = 0; variant < 2; variant++) {
                canvas c;
                draw_spit(&c, cx, cy, variant);
                sd_sprite spr;
                sd_sprite_create(&spr);
                canvas_to_sprite(&c, &spr);
                spr.index = variant;
                if(variant < count) {
                    sd_animation_set_sprite(ani, variant, &spr);
                } else {
                    sd_animation_push_sprite(ani, &spr);
                }
                sd_sprite_free(&spr);
                free(c.px);
            }
            alternate_shot_frames(&ani->anim_string);
            // hit points on the fireball, the same for both flicker frames
            static const vec2i shape[] = {{4, 0},   {2, -4},  {2, 4},   {-2, -5}, {-2, 5},
                                          {-6, -3}, {-6, 3},  {-11, 0}, {-16, 0}, {-21, 0}};
            vector_clear(&ani->coord_table);
            for(int frame = 0; frame < 2; frame++) {
                for(unsigned k = 0; k < sizeof(shape) / sizeof(shape[0]); k++) {
                    sd_coord coord = {{cx + shape[k].x, cy + shape[k].y}, 0, frame};
                    vector_append(&ani->coord_table, &coord);
                }
            }
            continue;
        }

        for(int i = 0; i < count; i++) {
            sd_sprite *s = sd_animation_get_sprite(ani, i);
            if(s->missing || s->len == 0) {
                continue;
            }
            canvas c;
            canvas_from_sprite(&c, s);
            if(m == MOVE_SHOT_HIT) {
                recolor_cannon_to_fire(&c);
                canvas_to_sprite(&c, s);
            }
            frames_x16 = realloc(frames_x16, sizeof(x16_frame) * (frame_count + 1));
            frames_x16[frame_count++] = x16_from(m, i, &c, i < 32 ? art[m][i] : NULL);
            free(c.px);
        }

        if(m == MOVE_CANNON) {
            // spit from the mouth
            canvas c;
            canvas_from_sprite(&c, sd_animation_get_sprite(ani, frame_of_spawn(ani)));
            int mx, my;
            find_mouth(&c, &mx, &my);
            free(c.px);
            if(move_spawn_point(&ani->anim_string, mx, my)) {
                fprintf(stderr, "no spawn point in the cannon's animation\n");
                return 1;
            }
            printf("fire spit leaves the mouth at %+d,%+d\n", mx, my);
        }
    }

    snprintf(fn, sizeof(fn), "%s/FIGHTR11.AF", dir);
    path_from_c(&p, fn);
    if(sd_af_save(&af, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot write %s\n", fn);
        return 1;
    }
    sd_af_free(&af);
    printf("wrote %s\n", fn);

    snprintf(fn, sizeof(fn), "%s/FIGHTR11.X16", dir);
    if(!x16_write(fn, frames_x16, frame_count)) {
        fprintf(stderr, "cannot write %s\n", fn);
        return 1;
    }
    printf("wrote %s\n", fn);
    return 0;
}

static int make_vs_portrait(const char *dir) {
    char fn[1024];
    path p;
    sd_bk_file bk;
    sd_bk_create(&bk);
    snprintf(fn, sizeof(fn), "%s/VS.BK", dir);
    path_from_c(&p, fn);
    if(sd_bk_load(&bk, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot read %s\n", fn);
        return 1;
    }
    sd_animation *ani = sd_bk_get_anim(&bk, VS_HAR_PORTRAITS)->animation;
    int count = sd_animation_get_sprite_count(ani);
    if(count > OCELOT_ID + 1 || count < OCELOT_ID) {
        fprintf(stderr, "unexpected VS.BK: %d HAR portraits\n", count);
        return 1;
    }
    canvas c;
    canvas_from_sprite(&c, sd_animation_get_sprite(ani, 0));
    // The portrait also carries bits of the cage, in the scene's own colors (48..159).
    // Those bytes now mean Ocelot's colors: move them to the closest shared ones.
    const vga_palette *scene_pal = bk.palettes[0];
    for(int i = 0; i < c.w * c.h; i++) {
        if(c.px[i] >= HAR_COLORS && c.px[i] < OCELOT_COLORS) {
            int best = SHARED_FIRST, best_d = 1 << 30;
            for(int k = SHARED_FIRST; k <= SHARED_LAST; k++) {
                int d = color_distance(scene_pal->colors[c.px[i]], scene_pal->colors[k]);
                if(d < best_d) {
                    best_d = d;
                    best = k;
                }
            }
            c.px[i] = best;
        }
    }
    paint_portrait_coat(&c);
    sd_sprite spr;
    sd_sprite_create(&spr);
    canvas_to_sprite(&c, &spr);
    free(c.px);
    spr.index = OCELOT_ID;
    if(count == OCELOT_ID + 1) {
        sd_animation_set_sprite(ani, OCELOT_ID, &spr); // rebuilt over a previous run
    } else {
        sd_animation_push_sprite(ani, &spr);
    }
    sd_sprite_free(&spr);
    if(sd_bk_save(&bk, &p) != SD_SUCCESS) {
        fprintf(stderr, "cannot write %s\n", fn);
        return 1;
    }
    sd_bk_free(&bk);
    printf("wrote %s (portrait %d)\n", fn, OCELOT_ID);
    return 0;
}

int main(int argc, char **argv) {
    if(argc != 3) {
        fprintf(stderr, "usage: %s <resources-dir> <art-dir>\n", argv[0]);
        return 2;
    }
    return make_ocelot(argv[1], argv[2]) || make_vs_portrait(argv[1]);
}

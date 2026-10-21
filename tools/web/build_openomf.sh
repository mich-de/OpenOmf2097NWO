#!/usr/bin/env bash
# Build OpenOMF (the C remake of ONE MUST FALL 2097) to WebAssembly and copy
# the result into this repository's wasm/ directory, ready for GitHub Pages.
#
#   source ~/emsdk/emsdk_env.sh
#   tools/web/build_deps.sh          # once: enet, libxmp, libconfuse
#   tools/web/build_openomf.sh
#
# Requires tools/web/build_deps.sh to have run, and the original game files in
# installed_game/ (OMF 2097 has been freeware since 1999).
set -euo pipefail

: "${OMF_WEB_BUILD:=$HOME/omf-web-build}"
B=$OMF_WEB_BUILD
DEPS=$B/deps
REPO=$(cd "$(dirname "$0")/../.." && pwd)
SHIM=$REPO/tools/web/shim
SRC=$B/openomf-src
BUILD=$B/omf-build
GAME=$REPO/installed_game
DIST=$REPO/wasm

OMF_COMMIT=4fd5be9a0a47645974a2c7716f5805a0303e8080

command -v emcc >/dev/null || { echo "emcc not found: source ~/emsdk/emsdk_env.sh first"; exit 1; }
[ -f "$DEPS/lib/libenet.a" ] || { echo "deps missing: run tools/web/build_deps.sh first"; exit 1; }
[ -d "$GAME" ] || { echo "installed_game/ missing: the original game files are required"; exit 1; }

# --- openomf source, pinned --------------------------------------------------
if [ ! -d "$SRC/.git" ]; then
    git clone https://github.com/omf2097/openomf.git "$SRC"
fi
if [ "$(git -C "$SRC" rev-parse HEAD)" != "$OMF_COMMIT" ]; then
    git -C "$SRC" fetch --depth 1 origin "$OMF_COMMIT"
    git -C "$SRC" checkout -q "$OMF_COMMIT"
fi

# GL availability probing must say why it failed: a silent failure here leaves
# the engine with zero renderers and it exits before rendering anything.
# (Applied against pristine sources, before the seds below.)
grep -q "GL probe" "$SRC/src/video/renderers/opengl3/sdl_window.c" || \
    git -C "$SRC" apply "$REPO/tools/web/gl-probe-log.patch"

# WebGL2 has no desktop GL core profiles: ask SDL for an OpenGL ES context.
grep -q PROFILE_ES "$SRC/src/video/renderers/opengl3/sdl_window.c" || \
    sed -i 's/SDL_GL_CONTEXT_PROFILE_CORE/SDL_GL_CONTEXT_PROFILE_ES/' \
        "$SRC/src/video/renderers/opengl3/sdl_window.c"

# ...and a plain 3.0 one: emscripten's EGL only understands
# EGL_CONTEXT_CLIENT_VERSION (no EGL_KHR_create_context), so a minor version
# other than 0 makes SDL take the KHR path and the context creation fails,
# which leaves the engine with no renderer at all.
grep -q "MINOR_VERSION, 0" "$SRC/src/video/renderers/opengl3/sdl_window.c" || \
    sed -i 's/SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, version_minor);/SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);/; s/SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);/SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);/' \
        "$SRC/src/video/renderers/opengl3/sdl_window.c"

# Upstream's "no opusfile" fallback stubs opus_load but forgets
# opus_load_memory, which audio_sources.c always calls.
grep -q "Web build: audio_sources.c always calls this" "$SRC/src/audio/music_sources/opus_source.c" || \
    git -C "$SRC" apply "$REPO/tools/web/opus-stub.patch"

# The engine loop paces itself on vsync; WebGL's SwapBuffers never blocks, so
# yield once per presented frame (see the patch for details).
grep -q "Web build: WebGL cannot block" "$SRC/src/video/video.c" || \
    git -C "$SRC" apply "$REPO/tools/web/yield-patch.patch"

# console_tick() polls stdin every tick; in the browser read(0, ...) never
# returns, which freezes the game loop on the first tick.
grep -q "Web build: stdin never receives data" "$SRC/src/console/console.c" || \
    git -C "$SRC" apply "$REPO/tools/web/stdin-skip.patch"

# WebGL2 cannot render to GL_RGBA16 (only sample from it): the FBO comes out
# incomplete and the screen stays black. Use renderable RGBA16F instead.
grep -q "table 3.13" "$SRC/src/video/renderers/opengl3/helpers/render_target.c" || \
    git -C "$SRC" apply "$REPO/tools/web/rgba16f-patch.patch"

# ...and WebGL2 reads that float FBO back only as RGBA/FLOAT: the match photo
# for the newsroom (har_screencaps_capture) otherwise comes back empty.
grep -q "reads a float color buffer" "$SRC/src/video/renderers/opengl3/gl3_renderer.c" || \
    git -C "$SRC" apply "$REPO/tools/web/readpixels-float.patch"

# Full HD and beyond: the canvas takes the screen's real pixels (SDL resizable + high
# DPI, sized by the page's CSS) and the game draws into a framebuffer about as tall,
# re-fitted whenever the page resizes it. (Applied after the sdl_window.c seds above.)
grep -q "web_fit_video" "$SRC/src/engine.c" || \
    git -C "$SRC" apply "$REPO/tools/web/web-hd.patch"

# OCELOT, a twelfth robot: Jaguar with a jaguar's coat, a tail and a fire spit.
# The engine learns about HAR 11 here; its art is generated below (tools/web/ocelot).
grep -q "HAR_OCELOT" "$SRC/src/game/common_defines.h" || \
    git -C "$SRC" apply "$REPO/tools/web/har-ocelot.patch"

# A sixth column on the robot select screen: Nova on top, Ocelot below.
# MELEE.BK only has art for ten robots, so the grid is re-cut from the
# background and the two portraits come from their idle animations.
grep -q "EXTRA_COLUMN" "$SRC/src/game/scenes/melee.c" || \
    git -C "$SRC" apply "$REPO/tools/web/melee-sixth-column.patch"

# HD sprites: robots drawn up to 4x finer than the game's pixels (Scale2x when they load,
# or the art's own detail), still moving and colliding at the game's size; a larger
# texture atlas that starts over when full. (Applied after the two patches above.)
grep -q "surface_upscale_pixelart" "$SRC/src/video/surface.h" || \
    git -C "$SRC" apply "$REPO/tools/web/hd-sprites.patch"

# Two more pilots, Trump and Musk (pictures: tools/web/pilots); the pilots' faces in the
# arena in single player; rehit on by default; the select screen's icons for the extra
# robots framed like the others; the GPU load following the frame rate. (After the above.)
grep -q "PILOT_TRUMP" "$SRC/src/game/common_defines.h" || \
    git -C "$SRC" apply "$REPO/tools/web/web-extras.patch"

# NWO: the 24 pilots (src/game/nwo_pilots.c) in place of the original ten, on two pages of
# the pilot grid. Their portraits: tools/web/pilots/art, drawn by tools/web/pilots/portraits.py.
grep -q "nwo_pilot_get" "$SRC/src/game/common_defines.c" || \
    git -C "$SRC" apply "$REPO/tools/web/nwo-pilots.patch"

# --- epoxy shim (libepoxy stand-in) -------------------------------------------
mkdir -p "$B/shim"
emcc -O2 -I"$SHIM" -c "$SHIM/epoxy_shim.c" -o "$B/shim/epoxy_shim.o"
emar rcs "$B/shim/libepoxy.a" "$B/shim/epoxy_shim.o"

# --- shaders: GLSL 330 core -> GLSL ES 300 ------------------------------------
python3 - "$SRC/shaders" "$B/shaders-es" <<'EOF'
import pathlib, re, sys
src, dst = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
count = 0
for f in src.rglob("*"):
    if f.suffix in (".vert", ".frag"):
        text = f.read_text().replace(
            "#version 330 core",
            "#version 300 es\nprecision highp float;\nprecision highp int;",
        )
        # GLSL ES: integer samplers have no default precision
        text = text.replace("uniform usampler2D", "uniform highp usampler2D")
        # GLSL ES: built-ins like gl_FragCoord must not be redeclared
        text = re.sub(r"(?m)^in\s+vec4\s+gl_FragCoord;\s*\n", "", text)
        # GLSL ES: global initializers must be constant expressions, but the
        # option flag globals derive from runtime uniforms/varyings (locals
        # are indented, so only true globals match ^bool)
        text = re.sub(r"(?m)^bool\s+(\w+)\s*=\s*(.+);\s*$", r"#define \1 (\2)", text)
        # GLSL ES: operators and function arguments get no implicit
        # int->float conversion (constructors do), e.g. textureLod(..., 0)
        text = re.sub(r"(textureLod\([^;]*?),\s*0\)", r"\1, 0.0)", text)
        text = text.replace("tan(10 * PHI", "tan(10.0 * PHI")
        text = text.replace("float decimate_limit = opacity / 255.0;",
                            "float decimate_limit = float(opacity) / 255.0;")
        out = dst / f.relative_to(src)
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(text)
        count += 1
print("shaders translated:", count)
EOF

# --- data pack: game files + translated shaders + controller db ---------------
# The engine resolves game files as <resource_dir>/resources/<NAME>, and
# resource_path_init() accepts <resource_dir> only when it contains
# resources/openomf.bk (see PATHS.md / resource_files.c).
DATA=$B/gamedata
rm -rf "$DATA" && mkdir -p "$DATA/resources"
cp -r "$GAME"/. "$DATA/resources"/
cp "$SRC/resources/gamecontrollerdb/gamecontrollerdb.txt" "$DATA/resources/"
cp "$SRC/resources/openomf.bk" "$DATA/resources/"
rm -rf "$DATA/shaders" && cp -r "$B/shaders-es" "$DATA/shaders"

# lang_init() hard-requires <LANG>.DAT2 (the OpenOMF language supplement);
# upstream generates it at build time from resources/<LANG>2.TXT with
# languagetool. We build that tool with the host compiler, because the wasm
# cross-build's copy cannot be executed here.
LT=$B/languagetool
if [ ! -x "$LT" ] || [ "$SRC/resources/ENGLISH2.TXT" -nt "$LT" ]; then
    cat > "$B/logstub.c" <<'STUB'
#include <stdarg.h>
#include <stdio.h>
#include "utils/log.h"
void log_msg(log_level level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}
STUB
    gcc -O1 -o "$LT" "$SRC/tools/languagetool/main.c" \
        "$SRC/src/formats/language.c" "$SRC/src/formats/error.c" "$SRC"/src/formats/internal/*.c \
        "$SRC"/src/utils/str.c "$SRC"/src/utils/path.c "$SRC"/src/utils/cp437.c \
        "$SRC"/src/utils/crash.c "$SRC"/src/utils/vector.c "$SRC"/src/utils/list.c \
        "$SRC"/src/utils/iterator.c "$SRC"/src/utils/c_string_util.c \
        "$SRC/src/vendored/argtable/argtable3.c" "$B/logstub.c" \
        -I"$SRC/src" -I"$SRC/src/vendored/argtable" -w -lm
fi
# Ocelot: FIGHTR11.AF from Jaguar's file and the hand-drawn frames in
# tools/web/ocelot/art, its own colors (OCELOT.GPL) and its VS.BK portrait.
OCELOT=$B/ocelot
if [ ! -x "$OCELOT" ] || [ "$REPO/tools/web/ocelot/ocelot.c" -nt "$OCELOT" ]; then
    gcc -O1 -o "$OCELOT" "$REPO/tools/web/ocelot/ocelot.c" "$REPO/tools/web/ocelot/host_stubs.c" \
        "$SRC"/src/formats/{af,altpal,bk,bkanim,animation,sprite,vga_image,rgba_image,pcx,move,palette,error}.c \
        "$SRC"/src/formats/internal/*.c "$SRC"/src/video/vga_palette.c "$SRC"/src/video/vga_remap.c \
        "$SRC"/src/utils/{str,path,vector,array,iterator,c_string_util,crash,list}.c "$B/logstub.c" \
        -I"$SRC/src" -w -lm -lpng -lz
fi
"$OCELOT" "$DATA/resources" "$REPO/tools/web/ocelot/art"

# The pilots' pictures: PILOTS.EXT, and their faces in MELEE.BK and VS.BK.
PILOTS=$B/pilots
if [ ! -x "$PILOTS" ] || [ "$REPO/tools/web/pilots/pilots.c" -nt "$PILOTS" ]; then
    gcc -O1 -o "$PILOTS" "$REPO/tools/web/pilots/pilots.c" "$REPO/tools/web/ocelot/host_stubs.c" \
        "$SRC"/src/formats/{af,altpal,bk,bkanim,animation,sprite,vga_image,rgba_image,pcx,move,palette,error}.c \
        "$SRC"/src/formats/internal/*.c "$SRC"/src/video/vga_palette.c "$SRC"/src/video/vga_remap.c \
        "$SRC"/src/utils/{str,path,vector,array,iterator,c_string_util,crash,list}.c "$B/logstub.c" \
        -I"$SRC/src" -w -lm -lpng
fi
"$PILOTS" "$DATA/resources" "$REPO/tools/web/pilots/art"

for L in ENGLISH GERMAN; do
    "$LT" -i "$SRC/resources/${L}2.TXT" -o "$DATA/resources/${L}.DAT2" --check-count 1
done

# --- flags --------------------------------------------------------------------
SYSROOT_INC="${EMSDK:?EMSDK not set - source emsdk_env.sh}/upstream/emscripten/cache/sysroot/include"
SYSROOT_INCLUDE="$SYSROOT_INC/SDL2"
[ -f "$SYSROOT_INCLUDE/SDL.h" ] || { echo "SDL2 port headers missing"; exit 1; }
# Fetch the zlib/libpng ports so their headers exist before cmake probes them.
emcc -O2 -sUSE_ZLIB=1 -sUSE_LIBPNG=1 -c -x c - -o /dev/null <<<'int main(void){return 0;}' 2>/dev/null
[ -f "$SYSROOT_INC/png.h" ] || { echo "libpng port headers missing"; exit 1; }
DUMMY=$B/shim/libdummy.a
[ -f "$DUMMY" ] || { echo 'int omf_dummy;' > "$B/shim/dummy.c"; emcc -c "$B/shim/dummy.c" -o "$B/shim/dummy.o"; emar rcs "$DUMMY" "$B/shim/dummy.o"; }

EM_FLAGS="-O2 -I$DEPS/include -sUSE_SDL=2 -sUSE_SDL_MIXER=2 -sUSE_ZLIB=1 -sUSE_LIBPNG=1 -sFULL_ES3=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2"
EM_LINK="$EM_FLAGS -sFORCE_FILESYSTEM=1 -sALLOW_MEMORY_GROWTH=1 -sASYNCIFY -sEXIT_RUNTIME=1 -sENVIRONMENT=web -sEXPORTED_RUNTIME_METHODS=FS -lidbfs.js --preload-file $DATA@/"
[ "${OMF_EM_DEBUG:-0}" = 1 ] && { EM_FLAGS="$EM_FLAGS -DDEBUGMODE"; EM_LINK="$EM_LINK -sASSERTIONS=1"; }

emcmake cmake -S "$SRC" -B "$BUILD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_PCH=OFF -DUSE_MINIUPNPC=OFF -DUSE_NATPMP=OFF -DUSE_LIBPNG=ON \
    -DUSE_OPUSFILE=OFF -DBUILD_LANGUAGES=OFF \
    -DSDL2_INCLUDE_DIR="$SYSROOT_INCLUDE" -DSDL2_LIBRARY="$DUMMY" -DSDL2MAIN_LIBRARY="$DUMMY" \
    -DSDL2_MIXER_INCLUDE_DIR="$SYSROOT_INCLUDE" -DSDL2_MIXER_LIBRARY="$DUMMY" \
    -DZLIB_INCLUDE_DIR="$SYSROOT_INC" -DZLIB_LIBRARY="$DUMMY" \
    -DPNG_PNG_INCLUDE_DIR="$SYSROOT_INC" -DPNG_LIBRARY="$DUMMY" \
    -DENET_INCLUDE_DIR="$DEPS/include" -DENET_LIBRARY="$DEPS/lib/libenet.a" \
    -DCONFUSE_INCLUDE_DIR="$DEPS/include" -DCONFUSE_LIBRARY="$DEPS/lib/libconfuse.a" \
    -DXMP_INCLUDE_DIR="$DEPS/include" -DXMP_LIBRARY="$DEPS/lib/libxmp.a" \
    -DEPOXY_INCLUDE_DIR="$SHIM" -DEPOXY_LIBRARY="$B/shim/libepoxy.a" \
    -DCMAKE_C_FLAGS="$EM_FLAGS" -DCMAKE_EXE_LINKER_FLAGS="$EM_LINK" >/dev/null

cmake --build "$BUILD" -j"$(nproc)" --target openomf

mkdir -p "$DIST"
cp "$BUILD/openomf.js" "$BUILD/openomf.wasm" "$BUILD/openomf.data" "$DIST/"
ls -la "$DIST"

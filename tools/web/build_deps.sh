#!/usr/bin/env bash
# Build OpenOMF's C dependencies (enet, libxmp, libconfuse) as WebAssembly
# static libraries with Emscripten. SDL2 and SDL2_mixer come from emscripten's
# own ports instead, so they are not built here.
#
#   source ~/emsdk/emsdk_env.sh
#   tools/web/build_deps.sh [build-dir]
#
# Output: <build-dir>/deps/lib/*.a and <build-dir>/deps/include/
set -euo pipefail

: "${OMF_WEB_BUILD:=$HOME/omf-web-build}"
B=${1:-$OMF_WEB_BUILD}
SRC=$B/deps-src
DEPS=$B/deps
mkdir -p "$SRC" "$DEPS/lib" "$DEPS/include"
command -v emcc >/dev/null || { echo "emcc not found: source ~/emsdk/emsdk_env.sh first"; exit 1; }

# --- sources (pinned) ---------------------------------------------------------
[ -d "$SRC/enet" ] || git clone --depth 1 --branch v1.3.18 https://github.com/lsalzman/enet.git "$SRC/enet"
[ -d "$SRC/libconfuse" ] || git clone --depth 1 https://github.com/libconfuse/libconfuse.git "$SRC/libconfuse"
[ -d "$SRC/libxmp-src" ] || git clone --depth 1 --branch libxmp-4.6.3 https://github.com/libxmp/libxmp.git "$SRC/libxmp-src"

# --- enet ---------------------------------------------------------------------
if [ ! -f "$DEPS/lib/libenet.a" ]; then
    emcmake cmake -S "$SRC/enet" -B "$SRC/enet/build-web" \
        -DCMAKE_BUILD_TYPE=Release -DENET_BUILD_SHARED=OFF -DENET_BUILD_STATIC=ON >/dev/null
    emmake cmake --build "$SRC/enet/build-web" >/dev/null
    cp "$SRC/enet/build-web/libenet.a" "$DEPS/lib/"
fi

# --- libconfuse ---------------------------------------------------------------
if [ ! -f "$DEPS/lib/libconfuse.a" ]; then
    (cd "$SRC/libconfuse" && [ -x configure ] || ./autogen.sh >/dev/null)
    (cd "$SRC/libconfuse" && emconfigure ./configure --prefix="$DEPS" --disable-shared --enable-static \
        --disable-examples --disable-tests >/dev/null && emmake make -j"$(nproc)" install >/dev/null)
fi

# --- libxmp -------------------------------------------------------------------
if [ ! -f "$DEPS/lib/libxmp.a" ]; then
    emcmake cmake -S "$SRC/libxmp-src" -B "$SRC/libxmp-src/build-web" \
        -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED=OFF -DBUILD_STATIC=ON \
        -DCMAKE_INSTALL_PREFIX="$DEPS" -DCMAKE_INSTALL_LIBDIR=lib >/dev/null
    emmake cmake --build "$SRC/libxmp-src/build-web" --target install >/dev/null
fi

echo "deps ready in $DEPS"
ls "$DEPS/lib"

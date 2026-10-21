#!/usr/bin/env python3
"""Insert synchronous-XHR stage probes into an OpenOMF checkout.

Every probe reports "GET /stage/<n>" to the web server while the game boots,
so the exact init stage stays visible even when the browser's main thread
ends up blocked (CDP and the console become unreachable then).

Idempotent: anchors are searched line by line and skipped when a probe for
that stage already sits next to them.

Usage: stage_probe.py <openomf-src-dir>
"""
import sys
from pathlib import Path

STAGE_FN = """#ifdef __EMSCRIPTEN__
#include <emscripten.h>
EM_JS(void, omf_stage, (int n), {
    try { var x = new XMLHttpRequest(); x.open('GET', '/stage/' + n, false); x.send(); } catch (e) {}
});
#define OMF_STAGE(n) omf_stage(n)
#else
#define OMF_STAGE(n) ((void)0)
#endif
"""

# omf_stage is defined once (main.c); other translation units only declare it
# (an EM_JS body in each TU would emit duplicate wasm symbols).
STAGE_FN_DECL = """#ifdef __EMSCRIPTEN__
extern void omf_stage(int n);
#define OMF_STAGE(n) omf_stage(n)
#else
#define OMF_STAGE(n) ((void)0)
#endif
"""

# (file, anchor, before/after, stage number)
INSERTS = [
    ("src/main.c", "int main(", "before", "0"),
    ("src/main.c", "    log_init();", "after", "11"),
    ("src/main.c", "    if(!resource_path_init()) {", "before", "12"),
    ("src/main.c", "    resource_path_create_dirs();", "after", "13"),
    ("src/main.c", "    if(SDL_InitSubSystem(SDL_INIT_JOYSTICK", "before", "20"),
    ("src/main.c", "    if(enet_initialize() != 0) {", "before", "22"),
    ("src/main.c", "    if(engine_init(&init_flags)) {", "before", "30"),
    ("src/main.c", "    engine_run(&init_flags);", "before", "40"),
    ("src/engine.c", "void engine_run(", "before", "0"),
    ("src/engine.c", '    log_info(" --- BEGIN GAME LOG ---");', "after", "41"),
    ("src/engine.c", "    if(game_state_create(gs, init_flags)) {", "before", "42"),
    ("src/engine.c", "    joystick_init();", "before", "43"),
    ("src/engine.c", "        video_render_prepare(framebuffer_options);", "after", "49"),
    ("src/engine.c", "            video_render_finish();", "after", "50"),
    ("src/engine.c", "        // hide mouse after n ticks", "before", "51"),
    ("src/engine.c", "        // Do the actual video rendering jobs", "before", "52"),
    ("src/engine.c", "        } while(tick_limit-- && (has_dynamic || has_static));", "after", "53"),
    ("src/engine.c", "            video_render_prepare(game_state_get_framebuffer_options(gs));", "before", "54"),
    ("src/engine.c", "                game_state_static_tick(gs, false);", "before", "55"),
    ("src/engine.c", "                game_state_static_tick(gs, false);", "after", "56"),
    ("src/engine.c", "                game_state_dynamic_tick(gs, false);", "before", "57"),
    ("src/engine.c", "                game_state_dynamic_tick(gs, false);", "after", "58"),
    ("src/engine.c", "            int dyntick_ms = game_state_ms_per_dyntick(gs);", "after", "59"),
    ("src/engine.c", "                console_tick(gs);", "before", "60"),
    ("src/engine.c", "                console_tick(gs);", "after", "61"),
    ("src/engine.c", "                osd_tick();", "before", "62"),
    ("src/engine.c", "                osd_tick();", "after", "63"),
    ("src/engine.c", "            // Handle other events", "after", "70"),
]


def inject(root: Path, rel: str, anchor: str, where: str, n: str) -> None:
    f = root / rel
    lines = f.read_text().splitlines(keepends=True)
    hits = [i for i, ln in enumerate(lines) if anchor in ln]
    if not hits:
        print(f"ANCHOR NOT FOUND: {rel}: {anchor!r}")
        sys.exit(1)
    i = hits[0]
    probe = f"    OMF_STAGE({n});  /* stage probe */\n"
    # idempotency: a probe for this stage already next to the anchor
    window = lines[max(0, i - 2) : i + 3]
    if any(f"OMF_STAGE({n})" in ln for ln in window):
        print(f"already injected: OMF_STAGE({n}) {rel}")
        return
    if n == "0":
        if any("EM_JS(void, omf_stage" in ln or "extern void omf_stage" in ln for ln in lines):
            print("stage helper already present")
            return
        lines.insert(i, (STAGE_FN if rel == "src/main.c" else STAGE_FN_DECL) + "\n")
    elif where == "after":
        lines.insert(i + 1, probe)
    else:
        lines.insert(i, probe)
    f.write_text("".join(lines))
    print(f"injected OMF_STAGE({n}) in {rel}")


def main() -> None:
    root = Path(sys.argv[1])
    for rel, anchor, where, n in INSERTS:
        inject(root, rel, anchor, where, n)


if __name__ == "__main__":
    main()

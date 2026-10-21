#!/usr/bin/env python3
"""Draws the NWO pilots' portraits as 240x240 SVGs: stylized caricatures, flat shapes.

    portraits.py <out-dir>      writes <out-dir>/<name>.svg for every pilot

Each portrait is the same head and shoulders, with the features that make the character:
hair, facial hair, glasses, headwear, clothes. Convert with ImageMagick (see README).
"""
import sys
from pathlib import Path

SKIN = {"light": ("#efc8a8", "#d9a886"), "fair": ("#f3d3bb", "#ddb294"), "tan": ("#e7a060", "#cf8446"),
        "olive": ("#d9a77c", "#bd895f"), "brown": ("#b97a52", "#9a6140")}


def head(skin, wide=1.0, jaw=1.0):
    face, shade = SKIN[skin]
    rx, ry = 44 * wide, 58 * jaw
    return f'''
<rect x="101" y="148" width="38" height="34" fill="{shade}"/>
<ellipse cx="120" cy="110" rx="{rx:.0f}" ry="{ry:.0f}" fill="{face}"/>
<ellipse cx="{120 + rx * 0.55:.0f}" cy="118" rx="{rx * 0.42:.0f}" ry="{ry * 0.78:.0f}" fill="{shade}" opacity=".55"/>
<ellipse cx="{120 - rx - 2:.0f}" cy="112" rx="8" ry="13" fill="{shade}"/>
<ellipse cx="{120 + rx + 2:.0f}" cy="112" rx="8" ry="13" fill="{shade}"/>'''


def eyes(color="#3a3a46", brows="#3a2a20", look="even", narrow=False):
    h = 3 if narrow else 5
    tilt = {"even": (0, 0), "stern": (4, -4), "worried": (-4, 4)}[look]
    return f'''
<ellipse cx="102" cy="106" rx="9" ry="{h}" fill="#ffffff"/><ellipse cx="138" cy="106" rx="9" ry="{h}" fill="#ffffff"/>
<ellipse cx="103" cy="106" rx="4" ry="{min(h, 4)}" fill="{color}"/><ellipse cx="137" cy="106" rx="4" ry="{min(h, 4)}" fill="{color}"/>
<polygon points="90,{95 + tilt[1]} 114,{93 + tilt[0]} 114,{98 + tilt[0]} 90,{99 + tilt[1]}" fill="{brows}"/>
<polygon points="126,{93 + tilt[0]} 150,{95 + tilt[1]} 150,{99 + tilt[1]} 126,{98 + tilt[0]}" fill="{brows}"/>'''


def nose(shade):
    return f'<polygon points="118,110 128,130 112,132" fill="{shade}"/>'


def mouth(kind="neutral"):
    if kind == "smile":
        return ('<path d="M 100,146 C 110,160 130,160 140,146 C 130,152 110,152 100,146 Z" fill="#7a2f2a"/>'
                '<path d="M 104,148 C 114,153 126,153 136,148 C 126,151 114,151 104,148 Z" fill="#f4f0ea"/>')
    if kind == "pursed":
        return '<ellipse cx="120" cy="147" rx="11" ry="7" fill="#9c3b34"/><ellipse cx="120" cy="147" rx="5" ry="3" fill="#6e2420"/>'
    if kind == "grin":
        return '<path d="M 98,144 C 108,162 132,162 142,144 Z" fill="#7a2f2a"/><path d="M 101,145 L 139,145 L 136,150 L 104,150 Z" fill="#f4f0ea"/>'
    return '<rect x="106" y="145" width="28" height="4" fill="#8a4a40"/>'


def glasses(color="#202020", round_=False):
    if round_:
        return (f'<circle cx="102" cy="106" r="13" fill="none" stroke="{color}" stroke-width="3"/>'
                f'<circle cx="138" cy="106" r="13" fill="none" stroke="{color}" stroke-width="3"/>'
                f'<line x1="115" y1="105" x2="125" y2="105" stroke="{color}" stroke-width="3"/>')
    return (f'<rect x="88" y="97" width="28" height="18" rx="4" fill="none" stroke="{color}" stroke-width="3"/>'
            f'<rect x="124" y="97" width="28" height="18" rx="4" fill="none" stroke="{color}" stroke-width="3"/>'
            f'<line x1="116" y1="104" x2="124" y2="104" stroke="{color}" stroke-width="3"/>')


HAIR = {
    "swept": lambda c, h: f'''<path d="M 66,100 C 60,58 82,32 122,30 C 164,28 186,52 180,98 C 172,72 156,64 132,66 C 104,68 84,74 66,100 Z" fill="{c}"/>
<path d="M 70,84 C 82,52 122,40 150,46 C 176,52 186,72 180,92 C 168,70 150,60 120,60 C 98,60 82,68 70,84 Z" fill="{h}"/>''',
    "short": lambda c, h: f'''<path d="M 74,96 C 70,56 92,36 122,34 C 152,34 170,54 168,92 C 160,70 146,62 124,62 C 100,62 84,72 74,96 Z" fill="{c}"/>
<path d="M 88,66 C 100,48 132,44 156,58 C 140,54 112,54 88,66 Z" fill="{h}"/>''',
    "side": lambda c, h: f'''<path d="M 74,100 C 66,56 92,34 124,34 C 156,34 172,56 168,96 C 162,74 150,64 132,64 L 104,60 C 92,64 82,76 74,100 Z" fill="{c}"/>
<path d="M 104,40 L 110,62 L 100,62 Z" fill="{h}"/>''',
    "bald": lambda c, h: f'''<path d="M 76,104 C 74,86 78,76 84,70 L 86,104 Z" fill="{c}"/><path d="M 164,104 C 166,86 162,76 156,70 L 154,104 Z" fill="{c}"/>
<ellipse cx="112" cy="62" rx="22" ry="8" fill="#ffffff" opacity=".25"/>''',
    "receding": lambda c, h: f'''<path d="M 74,100 C 70,70 84,50 104,46 C 98,58 96,64 92,74 C 86,82 80,90 74,100 Z" fill="{c}"/>
<path d="M 166,100 C 170,70 156,50 136,46 C 142,58 144,64 148,74 C 154,82 160,90 166,100 Z" fill="{c}"/>
<path d="M 104,46 C 116,42 126,42 136,46 C 126,50 114,50 104,46 Z" fill="{c}"/>''',
    "bob": lambda c, h: f'''<path d="M 60,150 C 52,90 66,40 118,32 C 172,28 192,80 182,150 C 176,120 170,96 160,78 C 140,62 110,58 90,70 C 76,86 68,116 60,150 Z" fill="{c}"/>
<path d="M 80,70 C 100,44 146,40 170,64 C 148,54 112,52 80,70 Z" fill="{h}"/>''',
    "long": lambda c, h: f'''<path d="M 58,190 C 46,110 60,40 120,30 C 180,26 196,110 182,190 C 176,140 174,104 162,82 C 146,64 104,58 84,74 C 70,96 66,140 58,190 Z" fill="{c}"/>
<path d="M 86,62 C 108,42 146,42 168,66 C 146,58 110,56 86,62 Z" fill="{h}"/>''',
    "volume": lambda c, h: f'''<path d="M 72,98 C 66,50 88,22 122,20 C 156,20 178,48 170,96 C 166,74 154,58 122,56 C 92,56 78,72 72,98 Z" fill="{c}"/>
<rect x="72" y="74" width="10" height="34" fill="#c8aa90"/><rect x="160" y="74" width="10" height="34" fill="#c8aa90"/>''',
    "combover": lambda c, h: f'''<path d="M 72,98 C 68,62 90,42 120,40 C 150,40 170,58 168,96 C 156,70 130,58 100,60 C 88,64 78,80 72,98 Z" fill="{c}"/>
<path d="M 80,62 C 110,46 150,48 170,70 C 148,60 112,56 80,62 Z" fill="{h}"/>''',
}


def hair(style, color, highlight):
    return HAIR[style](color, highlight) if style else ""


def facial(kind, color):
    if kind == "stubble":
        return f'<path d="M 82,128 C 86,170 154,170 158,128 C 150,160 90,160 82,128 Z" fill="{color}" opacity=".55"/>'
    if kind == "beard":
        return f'<path d="M 78,120 C 80,184 160,184 162,120 C 150,150 136,160 120,160 C 104,160 90,150 78,120 Z" fill="{color}"/>'
    if kind == "long beard":
        return f'<path d="M 76,118 C 76,214 164,214 164,118 C 152,150 136,160 120,160 C 104,160 88,150 76,118 Z" fill="{color}"/>'
    if kind == "mustache":
        return f'<path d="M 96,138 C 106,130 134,130 144,138 C 134,142 106,142 96,138 Z" fill="{color}"/>'
    return ""


HEADWEAR = {
    "turban black": '<path d="M 64,92 C 58,40 92,22 120,22 C 148,22 182,40 176,92 C 160,76 140,70 120,70 C 100,70 80,76 64,92 Z" fill="#141418"/>'
                    '<path d="M 70,80 C 96,62 144,62 170,80" fill="none" stroke="#2a2a34" stroke-width="5"/>',
    "turban white": '<path d="M 64,92 C 58,40 92,22 120,22 C 148,22 182,40 176,92 C 160,76 140,70 120,70 C 100,70 80,76 64,92 Z" fill="#e8e4d8"/>'
                    '<path d="M 70,80 C 96,62 144,62 170,80" fill="none" stroke="#c8c2b0" stroke-width="5"/>',
    "ghutra": '<path d="M 58,210 C 50,120 60,40 120,34 C 180,40 190,120 182,210 C 172,160 170,110 164,84 C 150,68 90,68 76,84 C 70,110 68,160 58,210 Z" fill="#f2f2f2"/>'
              '<path d="M 70,62 C 96,46 144,46 170,62 L 170,74 C 144,58 96,58 70,74 Z" fill="#141414"/>',
}


def outfit(kind, color="#1e2a5a", tie="#c8152a", shirt="#f4f4f4"):
    shoulders = f'<polygon points="6,240 28,198 72,180 120,174 168,180 212,198 234,240" fill="{color}"/>'
    if kind == "suit":
        return (shoulders + f'<polygon points="96,178 144,178 132,240 108,240" fill="{shirt}"/>'
                f'<polygon points="113,186 127,186 136,240 104,240" fill="{tie}"/><polygon points="111,178 129,178 125,192 115,192" fill="{tie}"/>')
    if kind == "open":
        return shoulders + f'<polygon points="98,178 142,178 120,214" fill="{shirt}"/>'
    if kind == "tee":
        return f'<polygon points="6,240 26,196 74,180 120,186 166,180 214,196 234,240" fill="{color}"/>'
    if kind == "mao":
        return shoulders + f'<rect x="100" y="174" width="40" height="12" fill="{color}" stroke="#000" stroke-opacity=".3"/><line x1="120" y1="186" x2="120" y2="240" stroke="#000" stroke-opacity=".3" stroke-width="2"/>'
    if kind == "thobe":
        return (f'<polygon points="6,240 28,198 72,180 120,174 168,180 212,198 234,240" fill="{color}"/>'
                f'<polygon points="92,178 148,178 140,240 100,240" fill="#fafafa"/>')
    if kind == "robe":
        return f'<polygon points="0,240 24,196 70,176 120,172 170,176 216,196 240,240" fill="{color}"/>'
    return shoulders


def pearls():
    return "".join(f'<circle cx="{120 + 30 * x:.0f}" cy="{176 + 12 * (1 - x * x):.0f}" r="4" fill="#fbf6ea"/>'
                   for x in [i / 5 - 1 for i in range(11)])


def portrait(p):
    face, shade = SKIN[p.get("skin", "light")]
    parts = [outfit(*p.get("outfit", ("suit",)))]
    if p.get("pearls"):
        parts.append(pearls())
    if p.get("headwear") == "ghutra":
        parts.append(HEADWEAR["ghutra"])
    if p.get("hair_back"):
        parts.append(hair(p["hair_back"], *p["hair_colors"]))
    parts.append(head(p.get("skin", "light"), p.get("wide", 1.0), p.get("jaw", 1.0)))
    parts.append(facial(p.get("facial", ""), p.get("facial_color", "#2a1e18")))
    parts.append(eyes(p.get("eye_color", "#3a3a46"), p.get("brows", "#3a2a20"), p.get("look", "even"), p.get("narrow", False)))
    parts.append(nose(shade))
    parts.append(mouth(p.get("mouth", "neutral")))
    if p.get("mustache"):
        parts.append(facial("mustache", p["mustache"]))
    if p.get("glasses"):
        parts.append(glasses(*p["glasses"]))
    if p.get("hair"):
        parts.append(hair(p["hair"], *p["hair_colors"]))
    if p.get("headwear") in ("turban black", "turban white"):
        parts.append(HEADWEAR[p["headwear"]])
    if p.get("headwear") == "ghutra":
        parts.append(HEADWEAR["ghutra"].split("/>", 1)[1])  # the agal again, over the forehead
    return ('<svg xmlns="http://www.w3.org/2000/svg" width="240" height="240" viewBox="0 0 240 240">'
            + "".join(parts) + "</svg>")


NAVY, CHARCOAL, BLACK, GREY = "#1e2a5a", "#2a2c34", "#16161c", "#4a4e58"
PILOTS = {
    "trump": dict(skin="tan", hair="swept", hair_colors=("#e8c060", "#f6d987"), mouth="pursed", wide=1.04,
                  outfit=("suit", NAVY, "#c8152a")),
    "musk": dict(skin="fair", hair="short", hair_colors=("#3a2a22", "#4e3a2e"), mouth="smile", eye_color="#3c5878",
                 outfit=("tee", BLACK)),
    "meloni": dict(skin="fair", hair="long", hair_colors=("#d8b46a", "#ecd08e"), mouth="smile", brows="#8a6a40",
                   eye_color="#4a6a8a", jaw=0.92, outfit=("open", BLACK, None, "#f4f4f4")),
    "macron": dict(skin="fair", hair="side", hair_colors=("#4a3428", "#5e4434"), eye_color="#3c5878",
                   outfit=("suit", NAVY, "#1c2440")),
    "scholz": dict(skin="fair", hair="bald", hair_colors=("#9a9a9a", "#bbbbbb"), wide=1.06, look="worried",
                   outfit=("suit", CHARCOAL, "#5a6a8a")),
    "starmer": dict(skin="fair", hair="side", hair_colors=("#b8b8b4", "#d8d8d4"), glasses=("#1a1a1a",),
                    outfit=("suit", NAVY, "#3a5a9a")),
    "leyen": dict(skin="fair", hair="bob", hair_colors=("#e2c88e", "#f2dcaa"), mouth="smile", eye_color="#4a6a8a",
                  brows="#a08a60", jaw=0.92, pearls=True, outfit=("open", "#c84a7a", None, "#f4f4f4")),
    "rutte": dict(skin="fair", hair="receding", hair_colors=("#9a8a7a", "#b0a090"), mouth="grin", eye_color="#3c5878",
                  outfit=("suit", NAVY, "#3a5a9a")),
    "xi": dict(skin="olive", hair="side", hair_colors=("#141414", "#2a2a2a"), wide=1.08, narrow=True,
               outfit=("suit", CHARCOAL, "#b0141e")),
    "kim": dict(skin="olive", hair="volume", hair_colors=("#101010", "#202020"), wide=1.14, narrow=True,
                outfit=("mao", "#24262c")),
    "putin": dict(skin="fair", hair="receding", hair_colors=("#a8a090", "#c0b8a8"), look="stern", narrow=True,
                  eye_color="#4a6070", outfit=("suit", CHARCOAL, "#6a6a72")),
    "zelensky": dict(skin="light", hair="short", hair_colors=("#2a1e18", "#3a2c22"), facial="stubble",
                     facial_color="#2a1e18", look="stern", outfit=("tee", "#4a5a2c")),
    "belousov": dict(skin="fair", hair="side", hair_colors=("#a0a0a0", "#c4c4c4"), glasses=("#2a2a2a",),
                     outfit=("suit", CHARCOAL, "#4a4a5a")),
    "umerov": dict(skin="olive", hair="short", hair_colors=("#1e1a18", "#2e2a26"), facial="beard",
                   facial_color="#2a2420", outfit=("suit", CHARCOAL, "#2a3a5a")),
    "lukashenko": dict(skin="light", hair="combover", hair_colors=("#8a7a6a", "#a0907e"), mustache="#5a4a3a",
                       look="stern", wide=1.06, outfit=("suit", CHARCOAL, "#3a3a3a")),
    "netanyahu": dict(skin="fair", hair="swept", hair_colors=("#d4d4d0", "#ececea"), look="stern",
                      outfit=("suit", NAVY, "#2a4a8a")),
    "katz": dict(skin="light", hair="receding", hair_colors=("#9a9a96", "#b6b6b2"), wide=1.08,
                 outfit=("open", CHARCOAL, None, "#f4f4f4")),
    "khamenei": dict(skin="light", headwear="turban black", facial="long beard", facial_color="#e6e6e2",
                     glasses=("#3a3a3a", True), mustache="#d6d6d2", outfit=("robe", "#3a2c22")),
    "al-houthi": dict(skin="brown", headwear="turban white", facial="beard", facial_color="#141414",
                      mustache="#141414", outfit=("robe", "#4a4a3a")),
    "erdogan": dict(skin="light", hair="receding", hair_colors=("#9a968e", "#b8b4ac"), mustache="#8a867e",
                    look="stern", outfit=("suit", NAVY, "#5a2a3a")),
    "bin salman": dict(skin="olive", headwear="ghutra", facial="beard", facial_color="#141414", mustache="#141414",
                       outfit=("thobe", "#5a3a1e")),
    "al thani": dict(skin="olive", headwear="ghutra", facial="beard", facial_color="#1e1a18", mustache="#1e1a18",
                     outfit=("thobe", "#f0f0f0")),
    "al-sisi": dict(skin="olive", hair="short", hair_colors=("#3a3a3a", "#4e4e4e"), mustache="#4a4a4a",
                    outfit=("suit", NAVY, "#1e2a5a")),
    "orban": dict(skin="fair", hair="receding", hair_colors=("#2a2420", "#3a332e"), wide=1.1,
                  outfit=("open", CHARCOAL, None, "#f4f4f4")),
}

if __name__ == "__main__":
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    for name, p in PILOTS.items():
        (out / f"{name}.svg").write_text(portrait(p))
    print(len(PILOTS), "portraits")

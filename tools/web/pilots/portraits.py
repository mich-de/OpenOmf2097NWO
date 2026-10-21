#!/usr/bin/env python3
"""Draws the NWO pilots' portraits as 240x240 SVGs: shaded caricatures in three-quarter view.

    portraits.py <out-dir>      writes <out-dir>/<name>.svg for every pilot

Like the original pilots' faces, each head is turned to the right (player 2's is mirrored),
lit from the front and cropped close. The same head is shaped per character (width, chin,
nose) and carries the features that make the character: hair, facial hair, glasses,
headwear, clothes. Convert with ImageMagick (see README).
"""
import random
import sys
from pathlib import Path

# light, mid, shadow
SKIN = {"light": ("#f6d2b4", "#dca684", "#a8705a"), "fair": ("#f8dccb", "#e2b39a", "#ad7b6a"),
        "tan": ("#f4ab62", "#d6863e", "#9a5426"), "olive": ("#e8b88c", "#c48e64", "#8a5a3e"),
        "brown": ("#c88a5e", "#a06640", "#663c24")}


def head_path(nose=1.0, chin=1.0):
    """The head, facing right: the skull's back on the left, the profile (brow, nose, lips,
    chin) on the right."""
    tip = 160 + 10 * nose
    cy = 186 + 6 * (chin - 1)
    cx = 4 * (chin - 1)
    return (f"M 112,34 C 80,34 58,58 58,96 C 58,124 66,142 80,156 C 92,172 112,{cy:.0f} 132,{cy + 2:.0f} "
            f"C 142,{cy + 2:.0f} {150 + cx:.0f},{cy - 4:.0f} {152 + cx:.0f},172 "
            f"C 153,164 150,160 152,156 C 156,152 156,146 153,142 L 156,137 "
            f"C {tip - 4:.0f},137 {tip + 2:.0f},134 {tip:.0f},127 C {tip - 6:.0f},116 158,106 156,98 "
            f"C 156,90 160,82 158,70 C 154,46 136,34 112,34 Z")


def defs(p):
    light, mid, dark = SKIN[p.get("skin", "light")]
    hc, hh = p.get("hair_colors", ("#222222", "#444444"))
    cloth = p.get("outfit", ("suit",))
    color = cloth[1] if len(cloth) > 1 else "#1e2a5a"
    return f'''<defs>
<linearGradient id="skin" x1="0" y1="0.2" x2="1" y2="0"><stop offset="0" stop-color="{dark}"/>
<stop offset=".35" stop-color="{mid}"/><stop offset=".7" stop-color="{light}"/><stop offset="1" stop-color="{light}"/></linearGradient>
<radialGradient id="lit" cx=".72" cy=".38" r=".45"><stop offset="0" stop-color="#fff" stop-opacity=".28"/>
<stop offset="1" stop-color="#fff" stop-opacity="0"/></radialGradient>
<linearGradient id="hair" x1="0" y1="1" x2=".8" y2="0"><stop offset="0" stop-color="{hc}"/>
<stop offset=".6" stop-color="{hc}"/><stop offset="1" stop-color="{hh}"/></linearGradient>
<linearGradient id="clothshade" x1="0" y1="0" x2="1" y2="0"><stop offset="0" stop-color="#000" stop-opacity=".5"/>
<stop offset=".6" stop-color="#000" stop-opacity="0"/></linearGradient>
<pattern id="check" width="12" height="12" patternUnits="userSpaceOnUse">
<rect width="12" height="12" fill="#f4f0ea"/><rect width="6" height="6" fill="#c8202a"/><rect x="6" y="6" width="6" height="6" fill="#c8202a"/></pattern>
</defs><rect width="240" height="240" fill="none"/>'''.replace("{color}", color)


def neck(p):
    _, mid, dark = SKIN[p.get("skin", "light")]
    return (f'<path d="M 86,140 L 140,170 L 144,224 L 84,224 Z" fill="{mid}"/>'
            f'<path d="M 86,140 L 140,170 L 140,190 C 120,192 100,180 86,170 Z" fill="{dark}"/>')


def head(p):
    light, mid, dark = SKIN[p.get("skin", "light")]
    d = head_path(p.get("nose", 1.0), p.get("chin", 1.0))
    return f'''<path d="{d}" fill="url(#skin)" stroke="{darken(dark, 0.6)}" stroke-width="2"/>
<path d="{d}" fill="url(#lit)"/>
<path d="M 60,80 C 58,120 70,150 92,168 C 110,182 124,188 138,190 C 116,176 102,156 98,136 C 96,120 100,100 108,84 C 96,70 76,66 60,80 Z" fill="{dark}" opacity=".75"/>
<path d="M 116,60 C 130,52 148,56 154,70 C 140,64 128,64 116,60 Z" fill="#fff" opacity=".35"/>
<path d="M 134,112 C 144,116 148,124 144,130 C 140,124 136,120 134,112 Z" fill="#fff" opacity=".3"/>
<path d="M 102,124 C 112,136 126,140 140,138 C 128,134 116,128 106,118 Z" fill="{dark}" opacity=".35"/>
<ellipse cx="80" cy="112" rx="10" ry="16" fill="{mid}" stroke="{dark}" stroke-width="2"/>
<path d="M 78,102 C 84,104 86,116 80,124" fill="none" stroke="{dark}" stroke-width="3"/>'''


def nose(p):
    _, _, dark = SKIN[p.get("skin", "light")]
    tip = 160 + 10 * p.get("nose", 1.0)
    return (f'<path d="M 150,100 C 152,114 {tip - 6:.0f},124 {tip - 2:.0f},130 C {tip - 8:.0f},136 150,135 145,131 '
            f'C 150,126 150,112 150,100 Z" fill="{dark}" opacity=".45"/>'
            f'<ellipse cx="{tip - 7:.0f}" cy="132" rx="3.5" ry="1.8" fill="{dark}"/>')


def eyes(p):
    color, brows = p.get("eye_color", "#3a3a46"), p.get("brows", "#3a2a20")
    h = 3.2 if p.get("narrow") else 5
    t = {"even": 0, "stern": 4, "worried": -4}[p.get("look", "even")]
    return f'''<ellipse cx="122" cy="105" rx="10" ry="{h}" fill="#f4f0ea"/><ellipse cx="149" cy="104" rx="5.5" ry="{h * 0.9:.1f}" fill="#f4f0ea"/>
<circle cx="126" cy="105" r="{min(h, 4.2):.1f}" fill="{color}"/><circle cx="151" cy="104" r="{min(h, 3.4):.1f}" fill="{color}"/>
<circle cx="127" cy="104" r="1.6" fill="#000"/><circle cx="151.5" cy="103.5" r="1.3" fill="#000"/>
<circle cx="125" cy="103" r="1.2" fill="#fff"/><circle cx="150.5" cy="102.5" r="1" fill="#fff"/>
<path d="M 111,{103 - h:.0f} C 118,{98 - h:.0f} 128,{98 - h:.0f} 133,{103 - h:.0f}" fill="none" stroke="#2a1410" stroke-width="3.5"/>
<polygon points="106,{95 - t} 134,{89 + t} 134,{94 + t} 106,{99 - t}" fill="{brows}"/>
<polygon points="143,{90 + t} 157,{91 - t} 157,{95 - t} 143,{95 + t}" fill="{brows}"/>'''


def details(p):
    """Before the eyes: the features that make a caricature."""
    dark = SKIN[p.get("skin", "light")][2]
    out = []
    if p.get("eye_patches"):  # paler skin around the eyes (a tanning bed's goggles)
        out.append('<ellipse cx="122" cy="106" rx="16" ry="10" fill="#f6dcb6"/>'
                   '<ellipse cx="149" cy="105" rx="9" ry="9" fill="#f6dcb6"/>')
    if p.get("jowls"):
        out.append(f'<path d="M 92,148 C 96,178 116,196 140,194 C 148,192 152,186 150,180 '
                   f'C 136,186 116,178 104,158 Z" fill="{dark}" opacity=".7"/>')
    if p.get("cheeks"):
        out.append('<ellipse cx="128" cy="128" rx="13" ry="8" fill="#e06a5a" opacity=".35"/>')
    return "".join(out)


def lines(p):
    """Over the eyes: wrinkles, eye bags, heavy lids."""
    _, mid, dark = SKIN[p.get("skin", "light")]
    out = []
    if p.get("age"):
        out.append(f'<path d="M 118,76 C 130,72 144,72 154,74 M 120,83 C 132,80 144,80 154,82" fill="none" stroke="{dark}" stroke-width="2"/>'
                   f'<path d="M 146,134 C 136,138 130,146 130,156" fill="none" stroke="{dark}" stroke-width="2.5"/>'
                   f'<path d="M 108,104 L 102,100 M 108,108 L 101,108" stroke="{dark}" stroke-width="1.5"/>')
    if p.get("bags"):
        out.append(f'<path d="M 113,112 C 120,117 128,117 133,112 M 144,110 C 147,114 152,114 155,110" '
                   f'fill="none" stroke="{dark}" stroke-width="2.5"/>')
    if p.get("lids"):
        out.append(f'<path d="M 110,98 L 135,98 L 134,104 L 111,104 Z M 142,98 L 156,98 L 156,103 L 143,103 Z" fill="{mid}"/>'
                   f'<path d="M 111,104 L 134,104 M 143,103 L 156,103" stroke="{dark}" stroke-width="2"/>')
    return "".join(out)


def mouth(p):
    kind = p.get("mouth", "neutral")
    lip = "#8a4038"
    if kind == "smile":
        return ('<path d="M 126,148 C 134,160 148,160 155,148 C 146,152 134,152 126,148 Z" fill="#6a2a24"/>'
                '<path d="M 130,149 C 138,153 146,153 152,149 L 152,151 C 144,154 136,154 131,151 Z" fill="#f4f0ea"/>')
    if kind == "pursed":
        return ('<path d="M 136,149 C 138,142 152,141 154,147 C 154,154 140,156 136,149 Z" fill="#a8483e"/>'
                '<ellipse cx="146" cy="148" rx="4" ry="2.5" fill="#5a1e1a"/>')
    if kind == "smirk":
        return f'<path d="M 126,152 C 136,153 146,150 154,144" fill="none" stroke="{lip}" stroke-width="3.5"/>'
    if kind == "small":
        return f'<path d="M 136,150 L 154,149" stroke="{lip}" stroke-width="3"/>'
    if kind == "grin":
        return ('<path d="M 122,144 C 130,162 150,162 156,144 Z" fill="#6a2a24"/>'
                '<path d="M 124,145 L 155,145 L 153,150 L 127,150 Z" fill="#f4f0ea"/>')
    return f'<path d="M 130,150 C 140,151 148,150 154,149" fill="none" stroke="{lip}" stroke-width="3.5"/>'


def glasses(color="#202020", round_=False):
    if round_:
        lenses = '<ellipse cx="123" cy="105" rx="13" ry="12"/><ellipse cx="150" cy="104" rx="7" ry="11"/>'
    else:
        lenses = '<rect x="108" y="95" width="29" height="19" rx="4"/><rect x="143" y="95" width="13" height="18" rx="3"/>'
    return (f'<g fill="#bcd8f0" fill-opacity=".2" stroke="{color}" stroke-width="3">{lenses}</g>'
            f'<path d="M 137,102 L 143,102 M 108,100 L 86,104" stroke="{color}" stroke-width="3"/>')


# Hair, in front of the head: the skull's top and back, the hairline on the forehead, the
# sideburn in front of the ear. Gradient "hair" for the mass; {h} highlights.
HAIR = {
    "short": '''<path d="M 56,108 C 46,60 74,26 116,26 C 148,26 166,44 162,72 C 150,62 136,62 122,64 C 106,68 96,76 92,88 L 90,110 L 84,98 C 74,96 66,104 66,118 Z" fill="url(#hair)"/>
<path d="M 92,46 C 110,34 140,34 156,50 C 136,42 112,42 92,46 Z" fill="{h}"/>''',
    "swept": '''<path d="M 52,112 C 40,58 76,20 124,20 C 158,20 182,36 180,62 C 182,76 172,84 160,80 C 146,74 128,74 112,80 C 100,86 94,96 92,110 L 86,98 C 74,96 66,104 64,120 Z" fill="url(#hair)"/>
<path d="M 84,52 C 108,30 160,28 178,54 C 156,40 116,40 84,52 Z M 110,72 C 130,62 160,62 178,66 C 160,70 132,70 110,72 Z" fill="{h}"/>''',
    "side": '''<path d="M 56,108 C 46,58 76,28 116,28 C 148,28 168,46 164,74 C 150,66 140,60 132,52 C 124,62 106,70 94,86 L 90,110 L 84,98 C 74,96 66,104 66,118 Z" fill="url(#hair)"/>
<path d="M 132,52 C 120,46 100,48 88,58 C 104,50 120,50 132,52 Z" fill="{h}"/>''',
    "bald": '''<path d="M 58,112 C 54,94 58,82 66,72 C 70,86 76,94 88,96 L 88,104 C 76,98 68,104 66,118 Z" fill="url(#hair)"/>
<ellipse cx="118" cy="50" rx="24" ry="9" fill="#ffffff" opacity=".3"/>''',
    "receding": '''<path d="M 56,110 C 48,70 66,40 98,32 C 104,46 100,58 96,70 C 90,80 88,92 88,104 L 84,98 C 74,96 66,104 66,118 Z" fill="url(#hair)"/>
<path d="M 98,32 C 118,28 136,32 146,40 C 132,40 114,42 100,46 Z" fill="url(#hair)"/>''',
    "long": '''<path d="M 54,112 C 42,56 76,24 118,24 C 156,24 172,46 166,80 C 158,64 146,56 130,54 C 116,64 102,72 94,86 L 90,110 L 84,98 C 74,96 66,104 66,118 Z" fill="url(#hair)"/>
<path d="M 130,54 C 140,64 150,72 166,80 C 160,60 150,50 130,54 Z M 96,40 C 116,30 144,32 160,50 C 140,40 116,38 96,40 Z" fill="{h}"/>''',
    "long back": '<path d="M 60,90 C 40,140 36,190 30,240 L 112,240 C 96,200 84,170 80,140 Z" fill="url(#hair)"/>',
    "undercut": '''<path d="M 58,108 C 54,84 58,66 68,56 L 96,62 L 94,104 C 80,96 66,100 66,114 Z" fill="#6a625c"/>
<path d="M 66,64 C 60,22 92,2 126,4 C 156,6 174,24 170,62 C 166,70 160,70 150,64 C 132,56 112,56 96,62 C 84,66 74,68 66,64 Z" fill="url(#hair)"/>
<path d="M 86,20 C 108,8 146,8 164,26 C 142,16 110,14 86,20 Z" fill="{h}"/>''',
    "coif": '''<path d="M 46,140 C 30,70 66,14 122,14 C 166,14 188,42 180,82 C 170,66 156,56 136,54 C 118,58 104,70 96,88 C 92,104 92,120 96,136 C 80,128 64,130 56,150 Z" fill="url(#hair)"/>
<path d="M 72,40 C 100,18 156,18 178,50 C 154,32 108,30 72,40 Z M 100,80 C 112,64 136,58 160,64 C 136,62 116,68 100,80 Z" fill="{h}"/>''',
    "slick": '''<path d="M 56,108 C 46,56 78,26 118,26 C 146,26 164,38 164,62 C 150,56 134,54 118,58 C 104,64 94,76 90,92 L 90,110 L 84,98 C 74,96 66,104 66,118 Z" fill="url(#hair)"/>
<path d="M 160,58 C 130,44 100,46 76,62 M 158,48 C 128,34 96,38 70,56" fill="none" stroke="{h}" stroke-width="3"/>''',
    "combover": '''<path d="M 56,110 C 50,80 60,56 80,46 C 106,40 140,44 160,64 C 140,58 116,58 100,62 C 92,72 88,90 88,104 L 84,98 C 74,96 66,104 66,118 Z" fill="url(#hair)"/>
<path d="M 80,50 C 110,44 140,48 160,64 M 84,58 C 110,52 136,54 156,66" fill="none" stroke="{h}" stroke-width="3"/>''',
}


def darken(color, k=0.55):
    return "#" + "".join(f"{int(int(color[i:i + 2], 16) * k):02x}" for i in (1, 3, 5))


def strands(clip, dark, light, seed):
    """Painted hair: strokes from the crown to the hairline, clipped to the hair's mass."""
    rnd = random.Random(seed)
    out = [f'<clipPath id="clip{seed}">{clip}</clipPath><g clip-path="url(#clip{seed})" fill="none">']
    for i in range(70):
        x, y = rnd.uniform(40, 190), rnd.uniform(0, 150)
        dx, dy = rnd.uniform(30, 60), rnd.uniform(-10, 30)
        color, width = (light, 1.5) if i % 3 == 0 else (dark, 2.5)
        out.append(f'<path d="M {x - dx:.0f},{y - dy:.0f} Q {x:.0f},{y - dy - 10:.0f} {x + dx:.0f},{y + dy:.0f}" '
                   f'stroke="{color}" stroke-width="{width}" stroke-opacity=".55"/>')
    return "".join(out) + "</g>"


def hair(style, highlight, dark="#000000"):
    if not style:
        return ""
    art = HAIR[style].replace("{h}", highlight)
    mass = art.split("/>")[0].replace('fill="url(#hair)"', "") + "/>"
    return art + strands(mass, dark, highlight, sum(map(ord, style)))


def facial(kind, color):
    if kind in ("stubble", "beard"):
        op = ' opacity=".5"' if kind == "stubble" else ""
        return (f'<path d="M 84,112 C 82,150 104,190 134,194 C 148,196 156,188 156,176 C 156,166 154,160 152,156 '
                f'C 140,158 130,158 124,152 C 118,146 112,138 108,130 C 100,126 92,120 84,112 Z" fill="{color}"{op}/>')
    if kind == "long beard":
        return (f'<path d="M 84,112 C 76,170 100,232 130,236 C 156,236 166,210 158,176 C 156,166 154,160 152,156 '
                f'C 140,158 130,158 124,152 C 118,146 112,138 108,130 C 100,126 92,120 84,112 Z" fill="{color}"/>'
                '<path d="M 100,170 C 108,200 118,220 128,230 M 118,176 C 124,196 132,212 140,226" fill="none" stroke="#000" stroke-opacity=".15" stroke-width="3"/>')
    if kind == "mustache":
        return f'<path d="M 124,148 C 128,140 144,136 158,138 C 158,144 150,146 140,146 C 134,147 128,149 124,148 Z" fill="{color}"/>'
    return ""


HEADWEAR = {
    "turban black": ('<path d="M 52,98 C 44,40 86,14 124,16 C 160,18 180,40 172,82 C 156,68 138,62 118,62 C 96,64 72,78 52,98 Z" fill="#16161a"/>'
                     '<path d="M 58,84 C 86,58 136,50 172,66 M 62,64 C 94,40 140,36 170,46" fill="none" stroke="#34343e" stroke-width="5"/>'),
    "turban white": ('<path d="M 52,98 C 44,40 86,14 124,16 C 160,18 180,40 172,82 C 156,68 138,62 118,62 C 96,64 72,78 52,98 Z" fill="#e8e4d8"/>'
                     '<path d="M 58,84 C 86,58 136,50 172,66 M 62,64 C 94,40 140,36 170,46" fill="none" stroke="#bab4a2" stroke-width="5"/>'),
    "cloth": ('<path d="M 30,240 C 30,160 36,80 64,46 C 90,22 140,20 168,44 C 176,56 178,66 176,74 '
              'C 162,64 140,60 120,62 C 100,64 90,74 86,96 C 82,130 86,180 104,240 Z" fill="{c}"/>'
              '<path d="M 40,200 C 44,140 52,90 72,58" fill="none" stroke="#000" stroke-opacity=".2" stroke-width="4"/>'),
    "agal": '<path d="M 64,64 C 96,44 150,44 176,62 L 174,74 C 146,58 98,58 68,78 Z" fill="#141414"/>',
}


def outfit(kind, color="#1e2a5a", tie="#c8152a", shirt="#f4f4f4"):
    body = "M 0,240 L 8,214 C 30,196 62,188 86,186 L 150,196 C 186,198 214,206 236,222 L 240,240 Z"
    shoulders = f'<path d="{body}" fill="{color}"/><path d="{body}" fill="url(#clothshade)"/>'
    if kind == "suit":
        return (shoulders + f'<polygon points="98,186 148,196 140,240 106,240" fill="{shirt}"/>'
                f'<polygon points="118,196 132,198 138,240 112,240" fill="{tie}"/><polygon points="116,188 134,192 130,204 118,202" fill="{tie}"/>'
                '<polygon points="92,186 112,222 104,240 84,240" fill="#000" fill-opacity=".25"/>')
    if kind == "open":
        return shoulders + f'<polygon points="98,186 148,196 128,226" fill="{shirt}"/>'
    if kind == "tee":
        return shoulders + '<path d="M 92,186 C 104,200 130,204 146,196" fill="none" stroke="#000" stroke-opacity=".35" stroke-width="4"/>'
    if kind == "mao":
        return (shoulders + f'<path d="M 90,178 L 146,190 L 146,204 L 92,192 Z" fill="{color}" stroke="#000" stroke-opacity=".35" stroke-width="2"/>'
                '<line x1="122" y1="200" x2="118" y2="240" stroke="#000" stroke-opacity=".35" stroke-width="2"/>')
    if kind == "thobe":
        return shoulders + '<polygon points="94,184 150,196 140,240 104,240" fill="#fafafa"/>'
    return shoulders


def pearls():
    return "".join(f'<circle cx="{96 + 7 * i}" cy="{190 + 0.25 * (i - 4) ** 2:.0f}" r="3.5" fill="#fbf6ea" stroke="#c8bca0"/>'
                   for i in range(9))


def portrait(p):
    body = [defs(p)]
    if p.get("hair_back"):
        body.append(hair(p["hair_back"], ""))
    if p.get("headwear") in ("shemagh", "ghutra"):
        body.append(HEADWEAR["cloth"].replace("{c}", "url(#check)" if p["headwear"] == "shemagh" else "#f2f2f2"))
    body.append(neck(p))
    body.append(outfit(*p.get("outfit", ("suit",))))
    if p.get("pearls"):
        body.append(pearls())
    face = [head(p), details(p), facial(p.get("facial", ""), p.get("facial_color", "#2a1e18")), nose(p), eyes(p),
            lines(p), mouth(p)]
    if p.get("mustache"):
        face.append(facial("mustache", p["mustache"]))
    if p.get("glasses"):
        face.append(glasses(*p["glasses"]))
    if p.get("hair"):
        face.append(hair(p["hair"], p["hair_colors"][1], darken(p["hair_colors"][0])))
    if p.get("headwear") in ("turban black", "turban white"):
        face.append(HEADWEAR[p["headwear"]])
    if p.get("headwear") in ("shemagh", "ghutra"):
        face.append(HEADWEAR["agal"])
    # caricature: the head (not the shoulders) widened around its middle
    w = p.get("wide", 1.0)
    return ('<svg xmlns="http://www.w3.org/2000/svg" width="240" height="240" viewBox="0 0 240 240">'
            + "".join(body) + f'<g transform="rotate(8 116 120) translate(116,0) scale({w},1) translate(-116,0)">' + "".join(face)
            + "</g></svg>")


NAVY, CHARCOAL, BLACK = "#1e2a5a", "#2a2c34", "#16161c"
PILOTS = {
    "trump": dict(skin="tan", hair="swept", hair_colors=("#d8a840", "#f6d987"), mouth="pursed", wide=1.1, chin=1.2,
                  eye_patches=True, narrow=True, jowls=True, look="stern", age=True, outfit=("suit", NAVY, "#c8152a")),
    "musk": dict(skin="fair", hair="short", hair_colors=("#3a2a22", "#5e4636"), mouth="smirk", eye_color="#3c5878",
                 chin=1.3, nose=0.9, bags=True, outfit=("tee", BLACK)),
    "meloni": dict(skin="fair", hair="long", hair_back="long back", hair_colors=("#c8a050", "#ecd08e"), mouth="smile",
                   brows="#8a6a40", eye_color="#4a6a8a", nose=0.8, chin=0.9, cheeks=True,
                   outfit=("open", BLACK, None, "#f4f4f4")),
    "macron": dict(skin="fair", hair="side", hair_colors=("#4a3428", "#6e5444"), eye_color="#2c6aa8", mouth="smirk",
                   nose=1.2, outfit=("suit", NAVY, "#1c2440")),
    "scholz": dict(skin="fair", hair="bald", hair_colors=("#8a8a8a", "#bbbbbb"), wide=1.08, mouth="smirk",
                   lids=True, age=True, nose=0.9, outfit=("suit", CHARCOAL, "#5a6a8a")),
    "starmer": dict(skin="fair", hair="side", hair_colors=("#a8a8a4", "#e0e0dc"), glasses=("#1a1a1a",),
                    brows="#2a2a2a", look="worried", mouth="small", outfit=("suit", NAVY, "#3a5a9a")),
    "leyen": dict(skin="fair", hair="coif", hair_colors=("#d0b078", "#f2dcaa"), mouth="smile", eye_color="#4a6a8a",
                  brows="#a08a60", nose=0.8, chin=0.9, pearls=True, outfit=("open", "#c84a7a", None, "#f4f4f4")),
    "rutte": dict(skin="fair", hair="receding", hair_colors=("#8a7a6a", "#b0a090"), mouth="grin", eye_color="#3c5878",
                  cheeks=True, nose=1.1, outfit=("suit", NAVY, "#3a5a9a")),
    "xi": dict(skin="olive", hair="slick", hair_colors=("#101010", "#3a3a3a"), wide=1.16, narrow=True, mouth="smile",
               jowls=True, nose=0.8, outfit=("suit", CHARCOAL, "#b0141e")),
    "kim": dict(skin="olive", hair="undercut", hair_colors=("#0c0c0c", "#303030"), wide=1.22, narrow=True,
                jowls=True, cheeks=True, mouth="small", nose=0.7, outfit=("mao", "#24262c")),
    "putin": dict(skin="fair", hair="receding", hair_colors=("#b0a48a", "#d6caae"), look="stern", narrow=True,
                  lids=True, eye_color="#4a6a8a", mouth="small", age=True, outfit=("suit", CHARCOAL, "#6a6a72")),
    "zelensky": dict(skin="light", hair="short", hair_colors=("#2a1e18", "#4a3a2e"), facial="stubble",
                     facial_color="#2a1e18", look="stern", bags=True, nose=1.1, outfit=("tee", "#4a5a2c")),
    "belousov": dict(skin="fair", hair="side", hair_colors=("#909090", "#c4c4c4"), glasses=("#2a2a2a",),
                     wide=1.08, age=True, outfit=("suit", CHARCOAL, "#4a4a5a")),
    "umerov": dict(skin="olive", hair="short", hair_colors=("#1e1a18", "#3e3a36"), facial="beard",
                   facial_color="#2a2420", bags=True, outfit=("suit", CHARCOAL, "#2a3a5a")),
    "lukashenko": dict(skin="light", hair="combover", hair_colors=("#7a6a5a", "#a0907e"), mustache="#5a4a3a",
                       look="stern", wide=1.1, cheeks=True, age=True, outfit=("suit", CHARCOAL, "#3a3a3a")),
    "netanyahu": dict(skin="fair", hair="swept", hair_colors=("#c4c4c0", "#f0f0ee"), look="stern", jowls=True,
                      bags=True, age=True, nose=1.2, outfit=("suit", NAVY, "#2a4a8a")),
    "katz": dict(skin="light", hair="receding", hair_colors=("#8a8a86", "#b6b6b2"), wide=1.16, jowls=True,
                 nose=1.1, outfit=("open", CHARCOAL, None, "#f4f4f4")),
    "khamenei": dict(skin="light", headwear="turban black", facial="long beard", facial_color="#e6e6e2",
                     glasses=("#3a3a3a", True), mustache="#d6d6d2", age=True, nose=1.2, outfit=("robe", "#3a2c22")),
    "al-houthi": dict(skin="brown", headwear="turban white", facial="beard", facial_color="#141414",
                      mustache="#141414", look="stern", outfit=("robe", "#4a4a3a")),
    "erdogan": dict(skin="light", hair="receding", hair_colors=("#8a867e", "#b8b4ac"), mustache="#8a867e",
                    look="stern", lids=True, age=True, chin=1.1, outfit=("suit", NAVY, "#5a2a3a")),
    "bin salman": dict(skin="olive", headwear="shemagh", facial="beard", facial_color="#141414", mustache="#141414",
                       wide=1.06, outfit=("thobe", "#5a3a1e")),
    "al thani": dict(skin="olive", headwear="ghutra", facial="beard", facial_color="#1e1a18", mustache="#1e1a18",
                     nose=1.1, outfit=("thobe", "#d8d8d8")),
    "al-sisi": dict(skin="olive", hair="short", hair_colors=("#2e2e2e", "#5e5e5e"), mustache="#4a4a4a",
                    look="stern", bags=True, outfit=("suit", NAVY, "#1e2a5a")),
    "orban": dict(skin="fair", hair="receding", hair_colors=("#2a2420", "#4a433e"), wide=1.18, jowls=True,
                  look="stern", nose=1.1, outfit=("open", CHARCOAL, None, "#f4f4f4")),
}


if __name__ == "__main__":
    out = Path(sys.argv[1])
    out.mkdir(parents=True, exist_ok=True)
    for name, p in PILOTS.items():
        (out / f"{name}.svg").write_text(portrait(p))
    print(len(PILOTS), "portraits")

# The glyph renderers (src/game/gfx/glyphrender: 0x6b0430-0x6b2a30, 0x6b3c20, 0x6b4a30-0x6b4a70, 0x6b5410) against the originals.
# Span writers: a surface at SURF (+0x18 the bits at DST, +0x24 the pitch, a whole number of pixels past the drawn width plus 0..3 bytes
# now and then), a span at SPAN (the point (x, y) at POINT, the coverage nibbles at GLYPH, the rectangle at RECT, the style at STYLE with
# the colour at +0x44: alpha 0, 0xff or between; the row length in pixels at +0x14, at least the width), the status at STATUS, `this`
# random (unused). Rectangles of 0..16 by 0..9 pixels; GlyphSolid_Span565's counts are the differences' low bytes, so its rectangles
# are 256 wider or taller now and then. The coverage bytes 0, 0xff, 0x88 / 0x77 (the threshold) or random.
# The patterned writers: the pattern (0..6 by 0..5 pixels, 0 now and then: a fault in both) at PAT, its format gFormatArgb8888's
# or not, the global at FORMAT, the origin small or random.
# GlyphRenderer_ClipSpan: the clip at CLIP in order or not, a glyph rectangle and a point near the clip's corner or anywhere.
# Deleting destructors: an object of random bytes at OBJ, flags 0, 1 or random (Mem_Free counts the block in the heap region).
# usage: glyphrender.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region  # noqa: E402

B = Bench([(0x5f9ca0, 0x5f9cc0), (0x62f810, 0x62f8a0), (0x6add30, 0x6add40), (0x6b0430, 0x6b0480), (0x6b0730, 0x6b4aa0),
           (0x6b5340, 0x6b5430)])
OUT = RAM + 0x300
SURF, SPAN, POINT, RECT, STYLE, STATUS, OBJ = RAM, RAM + 0x40, RAM + 0x60, RAM + 0x70, RAM + 0x80, RAM + 0xd0, RAM + 0xe0
GLYPH, PAT, DST = RAM + 0x100, RAM + 0x200, RAM + 0x400
FORMAT = 0xfaa280   # gFormatArgb8888: the patterned writers' pattern must have its format
ARGB8888 = (0xff0000, 0xff00, 0xff, 0xff000000, 32, 0)
# name: (address, bytes a pixel)
SPANS = {'blend1555': (0x6b0760, 2), 'blend565': (0x6b0c50, 2), 'blend4444': (0x6b1130, 2), 'blend8888': (0x6b15f0, 4),
         'blendbgra': (0x6b19f0, 4), 'solid1555': (0x6b1e30, 2), 'solid565': (0x6b2100, 2), 'solid4444': (0x6b21f0, 2),
         'solid8888': (0x6b24b0, 4), 'solidbgra': (0x6b2770, 4),
         'pat1555': (0x6b2a60, 2), 'pat565': (0x6b2e20, 2), 'pat4444': (0x6b31e0, 2), 'pat8888': (0x6b3540, 4), 'patbgra': (0x6b3890, 4),
         'fill1555': (0x6b3c50, 2), 'fill565': (0x6b3f40, 2), 'fill4444': (0x6b4140, 2), 'fill8888': (0x6b4420, 4),
         'fillbgra': (0x6b4700, 4)}
DTORS = {'del70a148': 0x6b0430, 'del70a160': 0x6b0450, 'delblend': 0x6b0730, 'delsolid': 0x6b1e00, 'delpattern1': 0x6b2a30,
         'delpattern2': 0x6b3c20, 'del70a210': 0x6b4a30, 'delbase': 0x6b4a70, 'del70a258': 0x6b5410}
CLIP = 0xfa76c0   # the display's clip (Display_GetClip)
OPS = [n for n in list(SPANS) + list(DTORS) + ['clipspan'] if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def glyph_bytes(r, n):
    kind = r.choice(['zero', 'full', 'edge', 'rand', 'rand'])
    if kind == 'zero':
        return bytes(n)
    if kind == 'full':
        return b'\xff' * n
    if kind == 'edge':
        return bytes(r.choice([0x88, 0x77, 0x87, 0x78, 0x08, 0x80]) for _ in range(n))
    return r.bytes(n)


def span(r, name):
    address, bpp = SPANS[name]
    cols = r.choice([0, 1, 2, 3, r.randrange(1, 17), r.randrange(1, 17)])
    rows = r.choice([0, 1, 2, r.randrange(1, 10), r.randrange(1, 10)])
    wide, tall = 0, 0
    if name == 'solid565' and r.random() < 0.2:
        wide, tall = r.choice([(256, 0), (0, 256), (256, 256)])
    left, top = r.randrange(6), r.randrange(4)
    right, bottom = left + cols + wide, top + rows + tall
    length = max(cols, r.choice([right, right + 1, right + r.randrange(6), cols]))
    x, y = r.randrange(4), r.randrange(3)
    pitch = (x + cols) * bpp + r.choice([0, bpp, r.randrange(8) * bpp, r.randrange(4)])
    glyph = glyph_bytes(r, ((top + rows) * length + left + cols) // 2 + 4)
    colour = r.getrandbits(24) | r.choice([0, 0xff, 0xff, r.randrange(1, 0xff)]) << 24
    style = bytearray(r.bytes(0x4c))
    struct.pack_into('<I', style, 0x44, colour)
    surf = bytearray(r.bytes(0x28))
    struct.pack_into('<I', surf, 0x18, DST)
    struct.pack_into('<i', surf, 0x24, pitch)
    sp = bytearray(r.bytes(0x18))
    struct.pack_into('<IIIII', sp, 0, POINT, GLYPH, r.getrandbits(32), RECT, STYLE)
    sp[0x14] = length & 0xff   # (GlyphSolid_Span565's wide rectangles: it does not read it)
    regions = {SURF: surf, SPAN: sp, POINT: struct.pack('<ii', x, y), RECT: struct.pack('<4i', left, top, right, bottom),
               STYLE: style, STATUS: r.bytes(4), GLYPH: glyph, DST: r.bytes((y + rows) * pitch + (x + cols) * bpp + 8)}
    if name.startswith(('pat', 'fill')):
        regions.update(pattern(r, style))
    return Call(address, [r.getrandbits(32), STATUS, SURF, SPAN], regions, conv='t', ret='a')


def pattern(r, style):
    """The patterned writers' pattern in the style (+0x14), and the format global it must match."""
    game = list(ARGB8888)
    fmt = list(ARGB8888)
    if r.random() < 0.3:   # the paletted flags need only agree on zero
        game[5] = r.choice([0, r.getrandbits(32) | 1])
        fmt[5] = r.choice([0, r.getrandbits(32) | 1]) if r.random() < 0.3 else (r.getrandbits(32) | 1 if game[5] else 0)
    if r.random() < 0.1:
        fmt[r.randrange(5)] ^= 1 << r.randrange(32)
    width = 0 if r.random() < 0.02 else r.randrange(1, 7)
    height = 0 if r.random() < 0.02 else r.randrange(1, 6)
    pitch = max(width, 1) * 4 + r.choice([0, 0, 4, 8])
    ox, oy = (r.choice([0, r.randrange(8), r.getrandbits(32)]) for _ in range(2))
    struct.pack_into('<12I', style, 0x14, *fmt, PAT, width, height, pitch, ox, oy)
    return {STYLE: style, PAT: r.bytes(pitch * max(height, 1)), FORMAT: struct.pack('<6I', *game)}


def clipspan(r):
    """GlyphRenderer_ClipSpan: a glyph rectangle placed at a point inside, across or outside the clip's edges."""
    def coord():
        return r.choice([r.randrange(-20, 60), r.randrange(-20, 60), r.choice([0, 10, 40]), r.getrandbits(32) - (1 << 31)])
    clip = [coord(), coord(), coord(), coord()]
    if r.random() < 0.7:
        clip = [min(clip[0], clip[2]), min(clip[1], clip[3]), max(clip[0], clip[2]), max(clip[1], clip[3])]
    gl, gt = r.randrange(-4, 20), r.randrange(-4, 20)
    glyph = [gl, gt, gl + r.randrange(-2, 30), gt + r.randrange(-2, 30)]
    point = [r.choice([clip[0] + r.randrange(-30, 30), coord()]), r.choice([clip[1] + r.randrange(-30, 30), coord()])]
    point = [p & 0xffffffff for p in point]
    regions = {OUT: r.bytes(16), RECT: struct.pack('<4I', *(v & 0xffffffff for v in glyph)), POINT: struct.pack('<2I', *point),
               CLIP: struct.pack('<4I', *(v & 0xffffffff for v in clip))}
    return Call(0x6b5340, [r.getrandbits(32), OUT, RECT, POINT, r.getrandbits(32)], regions, conv='t', ret='a')


def case(r):
    name = r.deal(OPS)
    if name == 'clipspan':
        return clipspan(r)
    if name in SPANS:
        return span(r, name)
    regions = {OBJ: r.bytes(0x20), LOG: log_region(), HEAP: heap(0x40)}
    return Call(DTORS[name], [OBJ, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

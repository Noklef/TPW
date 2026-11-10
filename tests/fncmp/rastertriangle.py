# The rasteriser's cseg triangle set-ups (src/game/render/rastertriangle) against the original, which jumps on into the span routine
# [0xfbe044] (dealt from the 21; rasterspans.py's fixture for the globals, texels, pixels and depths): three vertices (x within a
# 24-pixel row, y within 6 rows, any depth words, colour, u and v; a third of the cases one depth for all, the buffer's within 1 of it;
# one case in 16 a vertex twice; an eighth of the unbanded ones a sliver whose cross product is about 0x80), the order table [0xfbe710] the original's or dealt 0 / 0x10, the row and depth pitches either way.
# Where the span reads texels unmasked or picks the blend tables' row by the shaded word, the triangles are at least 32 pixels and u,
# v (and the colours) kept in a band, so the interpolated values stay within the texels and tables.
# usage: rastertriangle.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, Call, RAM  # noqa: E402
from rasterspans import SPANS, UNMASKED, TABLED, LIT, G, TEXELS, PIXELS, DEPTHS, WIDTH  # noqa: E402

B = Bench([(0x6ef7f0, 0x6f1372), (0x6f1d70, 0x6f6156)])
SETUPS = [0x6ef7f0, 0x6f02e0, 0x6f0a50]
VERTS, ROWS = RAM, 6
PAIRS = [0xfbe240, 0xfbe274, 0xfbe240, 0xfbe2a8, 0xfbe274, 0xfbe240, 0xfbe2a8, 0xfbe240]   # (grpoly_d's own 0xfbe6f0-0xfbe70f)
ORDER = [0x10, 0, 0, 0x10, 0, 0x10, 0, 0]   # (and 0xfbe710-0xfbe72f)


def vertices(r, banded, lit, z):
    while True:
        out = []
        for _ in range(3):
            colour = bytes(r.randrange(0x50, 0x90) for _ in range(4)) if lit else r.randbytes(4)
            uv = (r.randrange(0x30, 0xb0), r.randrange(0x30, 0xb0)) if banded else (r.getrandbits(32), r.getrandbits(32))
            out.append((r.randrange(3 << 4, (WIDTH - 3) << 4), r.randrange(8, 8 + ROWS * 16), r.getrandbits(32) if z is None else z, colour, uv))
        if not banded and r.randrange(8) == 0:   # (a sliver about the area threshold 0x80: v2 - v1 = (0, 1), v2 - v0 = (k, any))
            x, y, k = r.randrange(0xc0, (WIDTH - 3) << 4), r.randrange(8 + 16, 8 + ROWS * 16), r.choice([0x7f, 0x80, 0x81, r.randrange(0x100)])
            out[2] = (x, y) + out[2][2:]
            out[1] = (x, y - 1) + out[1][2:]
            out[0] = (x - k, r.randrange(8, y + 1)) + out[0][2:]
        (x0, y0, *_), (x1, y1, *_), (x2, y2, *_) = out
        if not banded or abs((x2 - x1) * (y2 - y0) - (y2 - y1) * (x2 - x0)) >= 0x2000:
            return b''.join(struct.pack('<III', x, y, z) + c + struct.pack('<II', *uv) for x, y, z, c, uv in out)


def case(r):
    address, span = r.deal(SETUPS), r.deal(SPANS)
    small, lit = span in UNMASKED, span in LIT
    banded = small or lit
    flat = r.randrange(3) == 0
    z = r.getrandbits(32) if flat else None
    pitch, zpitch = WIDTH * 4 + r.randrange(0, 3) * 4, (WIDTH + 1) * 2 + r.randrange(0, 2) * 2
    up = r.randrange(2)   # (the pitches negative, the base at the last row)
    mask = r.deal([0x0707, 0x0f0f, 0x1f1f, 0x3f3f, 0x0f3f, 0xffff0f0f])
    alpha = r.deal([-1, 0, 20, 40, 63])
    g = bytearray(r.randbytes(0x730))
    struct.pack_into('<I', g, 0x08, mask)
    struct.pack_into('<i', g, 0x10, alpha)
    struct.pack_into('<II', g, 0x1b8, *[r.deal([0x3fff, 0x3fc0, 0x1fff] if span in TABLED else [0x3fff, 0x3fc0, 0xffff, 0xffffffff]) for _ in range(2)])
    if lit:   # (the flat set-up leaves the edges' shade and its steps)
        struct.pack_into('<4h', g, 0x150, *[r.randrange(-0x100, 0x100) for _ in range(4)])
        for e in (0x240, 0x274, 0x2a8):
            struct.pack_into('<4H4h', g, e + 0x18, *[r.randrange(0x2800, 0x4800) for _ in range(4)], *[r.randrange(-0x100, 0x100) for _ in range(4)])
    if small:
        struct.pack_into('<II', g, 0xe4, r.randrange(0x1000000), r.randrange(0x1000000))
    struct.pack_into('<IIii', g, 0x24, PIXELS + (ROWS - 1) * pitch * up, DEPTHS + (ROWS - 1) * zpitch * up, -pitch if up else pitch, -zpitch if up else zpitch)
    struct.pack_into('<I', g, 0x34, r.deal([0, 1, 0x80000000]))
    struct.pack_into('<I', g, 0x38, TEXELS)
    struct.pack_into('<I', g, 0x44, span)
    order = ORDER if banded or r.randrange(2) else [r.choice([0, 0x10]) for _ in range(8)]
    struct.pack_into('<16I', g, 0x6f0, *PAIRS, *order)
    count = ROWS * zpitch // 2 + 1
    depths = struct.pack('<%dH' % count, *[((z & 0xffff) + r.randrange(-1, 2)) & 0xffff for _ in range(count)]) if flat else r.randbytes(count * 2)
    texels = bytearray(r.randbytes(0x4000 if small else ((mask & 0xffff) + 0x102) * 4))   # (0x6f4140's neighbours past the mask)
    if r.randrange(2):   # (the texels' top 6 bits within 1 of the alpha threshold)
        texels[3::4] = bytes(texels[3::4]).translate(bytes(((alpha + b % 3 - 1) & 63) << 2 | (b & 3) for b in range(256)))
    regions = {G: bytes(g), VERTS: vertices(r, banded, lit, z), TEXELS: bytes(texels), PIXELS: r.randbytes(ROWS * pitch), DEPTHS: depths}
    args = [VERTS, VERTS + 0x18, VERTS + 0x30]
    if r.randrange(16) == 0:
        args[r.randrange(3)] = args[r.randrange(3)]
    return Call(address, args, regions, ignore=[(0xfbe218, 4)])


if __name__ == '__main__':
    B.main(case)

# The rasteriser's cseg span routines (src/game/render/rasterspans) against the original, jumped to (Call(jumped=True)) with the row pointer
# in EBP and six words for their POPs: the two parts' row counts (-1 to 5) and edges (x 17.15 within a 24-pixel row, any u, v, shade
# and depth; a third of the cases one depth, step 0, the buffer's within 1 of it), the steps, both depth tests ([0xfbe034]), the texel
# mask, the alpha threshold [0xfbe010] (half the cases the texels' top 6 bits within 1 of it), the row and depth pitches either way. The
# globals 0xfbe000-0xfbe6e0 (the blend tables 0xfbe2e0 and 0xfbe4e0, the filter's 0xfbe0e4 and 0xfbe1b8 too) are the case's own;
# [0xfbe218] (the original's saved ESP) is left out. Where a filtered or shaded word picks the blend tables' row, the weights' masks
# and the shade words keep it within the tables.
# usage: rasterspans.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402
import x87emu  # noqa: E402

assert x87emu.MMX['PUNPCKHBW'](0x8877665544332211, 0xffeeddccbbaa9988) == 0xff88ee77dd66cc55   # (the high bytes interleaved)

B = Bench([(0x6f1d70, 0x6f5980)])
SPANS = [0x6f1d70, 0x6f2140, 0x6f2570, 0x6f29d0, 0x6f2de0, 0x6f31c0, 0x6f36d0, 0x6f3be0, 0x6f4140, 0x6f4690, 0x6f49c0, 0x6f4d40,
         0x6f50d0, 0x6f5590, 0x6f5840]
UNMASKED = [0x6f29d0, 0x6f5590, 0x6f5840]   # (the texel index not masked: u and v kept to top bytes below 0xe, the texels 0x1000)
TABLED = [0x6f3be0, 0x6f4140]   # (the blend tables' row from the filtered word: the weights' masks at most 0x3fff)
LIT = 0x6f4140   # (the row from the shaded word: the shade words kept positive)
G, EDGES, TEXELS, PIXELS, DEPTHS = 0xfbe000, RAM, RAM + 0x1000, RAM + 0x20000, RAM + 0x30000
WIDTH = 24


def edge(r, z, small, lit):
    x = r.randrange(3 << 15, (WIDTH - 3) << 15)
    depth = struct.pack('<II', z, 0) if z is not None else r.bytes(8)
    if small:
        uv = struct.pack('<IIii', r.randrange(0x3000000, 0xb000000), r.randrange(0x3000000, 0xb000000), r.randrange(-0x100000, 0x100000), r.randrange(-0x100000, 0x100000))
    else:
        uv = r.bytes(0x10)
    shade = struct.pack('<4H4h', *[r.randrange(0x1000, 0x6000) for _ in range(4)], *[r.randrange(-0x100, 0x100) for _ in range(4)]) if lit else r.bytes(0x10)
    return struct.pack('<Ii', x, r.randrange(-0x2000, 0x2000)) + uv + shade + depth


def case(r):
    address = r.deal(SPANS)
    small = address in UNMASKED
    flat = r.randrange(3) == 0   # (one depth and step 0, the buffer's within 1 of it: the tests' equal case)
    z = r.getrandbits(32) if flat else None
    rows = [r.deal([-1, 0, 1, 2, 3, 5]), r.deal([0, 1, 2, 4])]
    total = max(1, sum(n for n in rows if n > 0))
    pitch, zpitch = WIDTH * 4 + r.randrange(0, 3) * 4, (WIDTH + 1) * 2 + r.randrange(0, 2) * 2
    up = r.randrange(2)   # (the rows go down the buffer: the pitches negative)
    row = PIXELS + (0 if up else (total - 1) * pitch)
    zrow = DEPTHS + (0 if up else (total - 1) * zpitch)
    mask = r.deal([0x0707, 0x0f0f, 0x1f1f, 0x3f3f, 0x0f3f, 0xffff0f0f])
    alpha = r.deal([-1, 0, 20, 40, 63])
    g = bytearray(r.bytes(0x6e0))
    struct.pack_into('<I', g, 0x08, mask)
    struct.pack_into('<i', g, 0x10, alpha)
    struct.pack_into('<II', g, 0x1b8, *[r.deal([0x3fff, 0x3fc0, 0x1fff] if address in TABLED else [0x3fff, 0x3fc0, 0xffff, 0xffffffff]) for _ in range(2)])
    if address == LIT:
        struct.pack_into('<4h', g, 0x150, *[r.randrange(-0x100, 0x100) for _ in range(4)])
    if flat:
        struct.pack_into('<I', g, 0x148, 0)
    if small:
        struct.pack_into('<ii', g, 0x138, r.randrange(-0x100000, 0x100000), r.randrange(-0x100000, 0x100000))
        struct.pack_into('<4h', g, 0x160, *[r.randrange(-0x20, 0x20) for _ in range(4)])
    struct.pack_into('<ii', g, 0x2c, -pitch if up else pitch, -zpitch if up else zpitch)
    struct.pack_into('<I', g, 0x34, r.deal([0, 1, 0x80000000]))
    struct.pack_into('<I', g, 0x38, TEXELS)
    struct.pack_into('<ii', g, 0x200, rows[1], rows[0])
    struct.pack_into('<IIII', g, 0x208, EDGES, EDGES + 0x30, EDGES + 0x60, EDGES + 0x90)
    struct.pack_into('<I', g, 0x228, zrow)
    depths = (struct.pack('<%dH' % (total * zpitch // 2 + 1), *[((z >> 16) + r.randrange(-1, 2)) & 0xffff for _ in range(total * zpitch // 2 + 1)])
              if flat else r.randbytes(total * zpitch + 2))
    texels = bytearray(r.randbytes(0x4000 if small else ((mask & 0xffff) + 0x102) * 4))   # (0x6f4140's neighbours past the mask)
    if r.randrange(2):   # (the texels' top 6 bits within 1 of the alpha threshold)
        texels[3::4] = bytes(texels[3::4]).translate(bytes(((alpha + b % 3 - 1) & 63) << 2 | (b & 3) for b in range(256)))
    regions = {G: bytes(g), EDGES: b''.join(edge(r, z, small, address == LIT) for _ in range(4)), TEXELS: bytes(texels),
               PIXELS: r.randbytes(total * pitch), DEPTHS: depths}
    return Call(address, [row] + [r.getrandbits(32) for _ in range(6)], regions, conv='ebp', jumped=True, ignore=[(0xfbe218, 4)])


if __name__ == '__main__':
    B.main(case)

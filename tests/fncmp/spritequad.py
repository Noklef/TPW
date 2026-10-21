# A sprite frame's quad (src/game/gfx/spriteframe: 0x5890e0, with render/quad's turns 0x591430 and 0x591510) against the original:
# sizes given or 0 (a NaN now and then: the frame's scale), the image's sizes and offsets any (0 full sizes too), the flags 0x10,
# 0x20 / 0x40, 0x200 (with or without bit 0x1000000 of 0x8bcbc8; the pointer near vertex 0 or not; a frame picked before or not),
# 0x80000, 0x80 and 0x1000 each set or not (the picked frame's w a NaN now and then); vertices any (special values now and then). The x-z turn's angles stay under 1e7 radians
# (fsin / fcos reduce larger ones by their own 66-bit pi, see quad.py).
# FillItem (0x589b50): a draw item filled from such a frame, see fill().
# usage: spritequad.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Quad,FillItem)
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, SPECIAL, Bench, Call, bf, fb, log_region, put, vtable  # noqa: E402

B = Bench([(0x5890e0, 0x589410), (0x591430, 0x5915c0), (0x589b50, 0x589f8f), (0x590c40, 0x590ca8), (0x590fb0, 0x590fcb)])
FRAME, IMAGE, QUAD, PICKED = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
GAME_SINES = b''.join(struct.pack('<f', math.sin(i * math.pi / 128)) for i in range(256))


def angle(r):
    a = r.choice([fb(r.uniform(-7, 7)), fb(r.uniform(-1, 1)), r.f(), r.choice(SPECIAL)])
    return fb(r.uniform(-1e6, 1e6)) if 1e6 < abs(bf(a)) < 1e40 else a


def quad(r):
    flags = r.getrandbits(32)
    for bit in (0x10, 0x20, 0x40, 0x200, 0x80000, 0x80, 0x1000):
        flags = flags | bit if r.random() < 0.4 else flags & ~bit
    frame = bytearray(r.bytes(0x48))
    put(frame, 0x10, 'I', flags)
    put(frame, 0x28, 'I', IMAGE)
    put(frame, 0x2c, 'II', r.choice([fb(1.0), r.f(2), r.f()]), angle(r))
    image = bytearray(r.bytes(0x18))
    sizes = [r.choice([0, 1, 16, 64, r.randrange(1, 300), r.getrandbits(16)]) for _ in range(4)]
    put(image, 0xc, 'HHHH', *sizes)
    put(image, 0x14, 'hh', *(r.choice([0, 0, -8, 4, r.randrange(-0x8000, 0x8000)]) for _ in range(2)))
    quad = bytearray(b''.join(struct.pack('<I', r.f(r.choice([1, 50, 600]))) for _ in range(16)))
    if r.random() < 0.7:
        put(quad, 0xc, 'I', fb(r.uniform(0.2, 4)))
    size = [r.choice([0, 0, fb(r.uniform(1, 64)), r.f(), 0x7fc00000]) for _ in range(2)]
    picked = bytearray(r.bytes(0x10))
    put(picked, 0xc, 'I', r.choice([fb(r.uniform(0.2, 4)), struct.unpack_from('<I', quad, 0xc)[0], r.f(), 0x7fc00000]))
    near = r.random() < 0.5
    pointer = [r.f(100), r.f(100)]
    if near:   # the pointer at vertex 0's x, y seen through 1 / w, give or take
        w = bf(struct.unpack_from('<I', quad, 0xc)[0])
        for k in range(2):
            v = bf(struct.unpack_from('<I', quad, 4 * k)[0])
            if w and math.isfinite(v) and math.isfinite(w) and abs(v / w) < 1e30:
                pointer[k] = fb(v / w + r.uniform(-2, 2))
    regions = {FRAME: frame, IMAGE: image, QUAD: quad, PICKED: picked, 0x8c63d8: GAME_SINES,
               0x8bcbcc: struct.pack('<I', r.choice([fb(1.0), r.f(2)])),
               0x8bcbc8: struct.pack('<I', r.getrandbits(32) | 0x1000000 if r.random() < 0.7 else r.getrandbits(32) & ~0x1000000),
               0x8bd4ec: struct.pack('<IIIII', r.f(10), pointer[0], pointer[1], r.getrandbits(32), r.choice([0, PICKED])),
               0x87af60: struct.pack('<I', angle(r))}
    return Call(0x5890e0, [FRAME, QUAD] + size, regions, conv='t', ret='a')


TEX, TERRAIN, GRID, ITEM, XF, VT, HEIGHTS = RAM + 0x380, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x700, RAM + 0x800, RAM + 0x1000


def fill(r):
    """0x589b50 (thiscall (item), pops 4): quad()'s frame and quad, the frame's flags 0x10000000 (mostly), 0x400, 0x40000, 0x80000, 0x200 and
    0x8000 each set or not, its vertex +0 and position +0x14 floats; the image's first dword a texture (flags 0x1000, 0x100000, 0x300,
    0x30000, 0x4000000 each set or not; [0x8bd524] 1 or not) or 0; the terrain ([0x879d60]) a grid of 1-16 by 1-16 cells, the position over
    it (or a cell off it); the item's quad (+4) quad()'s; the transformer ([0x87b1d8]) a recorder (slot 1 cdecl)."""
    c = quad(r)
    regions = dict(c.regions)
    frame = bytearray(regions[FRAME])
    flags = struct.unpack_from('<I', frame, 0x10)[0]
    for bit in (0x10000000, 0x400, 0x40000, 0x80000, 0x200, 0x8000):
        flags = flags | bit if r.random() < (0.9 if bit == 0x10000000 else 0.4) else flags & ~bit
    put(frame, 0x10, 'I', flags)
    put(frame, 0, '4I', *(r.f(r.choice([1, 50, 600])) for _ in range(4)))
    width, depth = r.randrange(1, 17), r.randrange(1, 17)
    scale = [r.choice([1.0, 0.5, 0.25, r.uniform(0.1, 2)]) for _ in range(2)]
    origin = [r.uniform(-100, 100), r.uniform(-100, 100)]
    cell = [r.choice([1 / scale[k], r.uniform(0.2, 4)]) for k in range(2)]
    x = origin[0] + r.uniform(-0.9, width + 0.9) / scale[0]
    z = origin[1] + r.uniform(-0.9, depth + 0.9) / scale[1]
    off = r.choice([fb(r.uniform(-300, 300)), 0x7fc00000, 0xff800000])   # (off the grid; far ones would index any memory)
    put(frame, 0x14, '3I', r.choice([fb(x), fb(x), off]), r.f(50), r.choice([fb(z), fb(z), off]))
    regions[FRAME] = frame
    image = bytearray(regions[IMAGE])
    put(image, 0, 'I', TEX if r.random() < 0.85 else 0)
    regions[IMAGE] = image
    tex = bytearray(r.bytes(0x30))
    tflags = r.getrandbits(32)
    for bit in (0x1000, 0x100000, 0x100, 0x200, 0x10000, 0x20000, 0x4000000):
        tflags = tflags | bit if r.random() < 0.4 else tflags & ~bit
    put(tex, 8, 'I', tflags)
    put(tex, 0x20, '4I', *(r.f(r.choice([1, 300])) for _ in range(4)))
    regions[TEX] = tex
    regions[0x8bd524] = struct.pack('<I', r.choice([1, 1, 0, 2]))
    regions[TERRAIN] = struct.pack('<4I2f', GRID, r.getrandbits(32), width, depth, *scale)
    grid = bytearray(r.bytes(0x30))
    put(grid, 8, '4f', origin[0], origin[1], *cell)
    put(grid, 0x28, 'I', HEIGHTS + 0x400)
    regions[GRID] = grid
    regions[HEIGHTS] = b''.join(struct.pack('<I', r.choice([fb(r.uniform(-20, 60)), fb(r.uniform(0, 10)), r.f()])) for _ in range(0x400))
    regions[0x879d60] = struct.pack('<I', TERRAIN)
    item = bytearray(r.bytes(0x30))
    put(item, 4, 'I', QUAD)
    regions[ITEM] = item
    regions.update({XF: struct.pack('<I', VT), VT: vtable({}, cdecl={1: 8}), LOG: log_region(), 0x87b1d8: struct.pack('<I', XF),
                    0x87b088: r.bytes(4)})
    return Call(0x589b50, [FRAME, ITEM], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Quad,FillItem').split(',') if k]


def case(r):
    return fill(r) if r.deal(OPS) == 'FillItem' else quad(r)


if __name__ == '__main__':
    B.main(case)

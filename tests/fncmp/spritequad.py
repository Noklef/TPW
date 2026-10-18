# A sprite frame's quad (src/game/gfx/spriteframe: 0x5890e0, with render/quad's turns 0x591430 and 0x591510) against the original:
# sizes given or 0 (a NaN now and then: the frame's scale), the image's sizes and offsets any (0 full sizes too), the flags 0x10,
# 0x20 / 0x40, 0x200 (with or without bit 0x1000000 of 0x8bcbc8; the pointer near vertex 0 or not; a frame picked before or not),
# 0x80000, 0x80 and 0x1000 each set or not (the picked frame's w a NaN now and then); vertices any (special values now and then). The x-z turn's angles stay under 1e7 radians
# (fsin / fcos reduce larger ones by their own 66-bit pi, see quad.py).
# usage: spritequad.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, SPECIAL, Bench, Call, bf, fb, put  # noqa: E402

B = Bench([(0x5890e0, 0x589410), (0x591430, 0x5915c0)])
FRAME, IMAGE, QUAD, PICKED = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
GAME_SINES = b''.join(struct.pack('<f', math.sin(i * math.pi / 128)) for i in range(256))


def angle(r):
    a = r.choice([fb(r.uniform(-7, 7)), fb(r.uniform(-1, 1)), r.f(), r.choice(SPECIAL)])
    return fb(r.uniform(-1e6, 1e6)) if 1e6 < abs(bf(a)) < 1e40 else a


def case(r):
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


if __name__ == '__main__':
    B.main(case)

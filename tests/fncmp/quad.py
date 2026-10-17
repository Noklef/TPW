# A quad turned about its centre (src/game/render/quad: 0x591430, 0x591510) against the originals: any vertices (special values now
# and then, NaNs tying on significand too), angles in turns for the table one (the game's sine table or any floats in it, out of
# range angles too) and given or computed centres; and angles in radians for the fsin one.
# Wild angles from 1e7 radians to 2^63 are left out of the fsin one: there fsin / fcos reduce the argument with their own 66-bit pi,
# which neither x87emu (Python's math.sin) nor the C++ CRT reproduces (see tests/fncmp/matrix.py).
# usage: quad.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=XY,XZ)
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x591430, 0x5915c0)])
OPS = [k for k in os.environ.get('OPS', 'XY,XZ').split(',') if k]
QUAD, CENTRE, SINES = RAM, RAM + 0x100, 0x8c63d8
NANS = [0x7fc00000, 0xffc00000, 0x7fc00001, 0xffc00001]
GAME_SINES = b''.join(struct.pack('<f', math.sin(i * math.pi / 128)) for i in range(256))


def case(r):
    op = r.deal(OPS)
    scale = r.choice([1, 10, 300, None])
    quad = r.floats(bytearray(0x40), range(0, 0x40, 4), scale)
    if r.random() < 0.1:
        for off in r.sample(range(0, 0x40, 4), 4):
            put(quad, off, 'I', r.choice(NANS))
    regions = {QUAD: quad}
    if op == 'XY':
        angle = r.choice([fb(r.uniform(-2, 2)), fb(r.uniform(-1e6, 1e6)), fb(r.randrange(-512, 512) / 256), r.f()])
        sines = GAME_SINES if r.random() < 0.8 else r.floats(bytearray(0x400), range(0, 0x400, 4))
        regions[SINES] = sines
        centre = 0
        if r.random() < 0.5:
            centre = CENTRE
            regions[CENTRE] = struct.pack('<2I', r.f(scale), r.f(scale))
        return Call(0x591430, [QUAD, angle, centre], regions)
    angle = r.choice([fb(r.uniform(-7, 7)), fb(r.uniform(-1e6, 1e6)), fb(r.uniform(1e19, 1e30)) | r.choice([0, 0x80000000]),
                      r.choice([0x7f800000, 0xff800000, 0, 0x80000000] + NANS)])
    return Call(0x591510, [QUAD, angle], regions)


if __name__ == '__main__':
    B.main(case)

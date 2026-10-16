# The wavelet codec's coefficient writer (src/game/gfx/wavelet: 0x591d30) against the original:
# values at and around the byte range's edges, cursors into a buffer and over the cursor itself.
# usage: wavelet.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x591d30, 0x591d6c)])
CURSOR = RAM + 8


def case(r):
    mem = bytearray(r.bytes(0x40))
    edge = r.choice([-0x81, -0x80, -0x7f, -0x7e, 0, 0x7e, 0x7f, 0x80, 0x81, -0x8000, 0x7fff])
    value = r.choice([edge, r.randrange(-0x200, 0x200), r.getrandbits(16) << 16 | edge & 0xffff, r.getrandbits(32)])
    put(mem, CURSOR - RAM, 'I', RAM + r.choice([0x10, 0x20, 0x3d, r.randrange(5, 12)]))   # 5-11: over the cursor
    return Call(0x591d30, [value & 0xffffffff, CURSOR], {RAM: mem}, conv='s')


if __name__ == '__main__':
    B.main(case)

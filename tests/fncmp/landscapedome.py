# The landscape's dome built (src/game/terrain/landscape: 0x585720) against the original: the 18 rims' ring, the 16 by 16 grid with its colours from rand (the harness's generator on a seed) and the triangles above the ring's lowest level, from
# the sizes (+0x21e0, +0x21e4) and the centre (+0x1f64); a NaN and a very big or negative size now and then.
# usage: landscapedome.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, RAND_SEED, Bench, Call, put  # noqa: E402

B = Bench([(0x585720, 0x585ce0)])
LANDSCAPE = RAM


def case(r):
    land = bytearray(r.bytes(0x2d00))
    r.floats(land, [0x1f64, 0x1f68, 0x1f6c], r.choice([1.0, 100.0, 5000.0]))
    if r.randrange(16) == 0:
        put(land, r.choice([0x1f64, 0x1f68, 0x1f6c]), 'I', r.choice([0x7fc00001, 0xffc00002, 0x7f800000, 0xff800000]))
    put(land, 0x21e0, '2i', r.choice([0, 1, 16, 256, 256, 1024, 3000, -256, r.getrandbits(31)]), r.choice([0, 1, 16, 256, 256, 1024, 3000, -256, r.getrandbits(31)]))
    return Call(0x585720, [LANDSCAPE], {LANDSCAPE: land, RAND_SEED: struct.pack('<I', r.getrandbits(32))}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

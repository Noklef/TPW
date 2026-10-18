# The float pair made unit length (src/game/math/vec2: 0x5787b0) against the original: any floats, zero pairs, NaNs and infinities;
# and the integer pair's length (0x549020): any ints, small ones, ones about the 0x8000 / 0x7fff limits that halve them and INT_MIN.
# usage: vec2.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x5787b0, 0x578800), (0x549020, 0x54906e), (0x67a830, 0x67a860)])
EDGE = [0, 0x80000000, 0x7fc00000, 0x7f800001, 0x7f800000, 0xff800000, 1, 0x00800000]


def length(r):
    def one():
        k = r.choice([0, 1, 2, 3, 4])
        v = [r.getrandbits(32), r.randrange(-300, 300), r.choice([0x7fff, 0x8000, 0x8001, 0x10000, 0xffff, 0x80000000, 0x7fffffff]), r.randrange(-0x20000, 0x20000), 0][k]
        return (v if r.random() < 0.5 else -v) & 0xffffffff
    return Call(0x549020, [one(), one()], {}, ret='a')


def case(r):
    if r.deal([0x5787b0, 0x549020]) == 0x549020:
        return length(r)
    pair = [r.f(), r.f()]
    for i in range(2):
        if r.random() < 0.15:
            pair[i] = r.choice(EDGE)
    return Call(0x5787b0, [RAM], {RAM: struct.pack('<2I', *pair)}, conv='t')


if __name__ == '__main__':
    B.main(case)

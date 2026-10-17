# The float pair made unit length (src/game/math/vec2: 0x5787b0) against the original: any floats, zero pairs, NaNs and infinities.
# usage: vec2.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x5787b0, 0x578800)])
EDGE = [0, 0x80000000, 0x7fc00000, 0x7f800001, 0x7f800000, 0xff800000, 1, 0x00800000]


def case(r):
    pair = [r.f(), r.f()]
    for i in range(2):
        if r.random() < 0.15:
            pair[i] = r.choice(EDGE)
    return Call(0x5787b0, [RAM], {RAM: struct.pack('<2I', *pair)}, conv='t')


if __name__ == '__main__':
    B.main(case)

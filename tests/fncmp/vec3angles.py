# Three unit directions from an angle (src/game/math/vector: 0x565110) against the original: angles small, about 0..2pi, large past the
# x87's 2^63 trig limit, and special (zeros, NaNs, infinities); none from about 1e6 / 2pi to 2^63, whose sines (of it or pi times it)
# the x87 reduces by its 66-bit pi, as in mat4.py.
# usage: vec3angles.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, bf, fb  # noqa: E402

B = Bench([(0x565110, 0x56517c)])


def case(r):
    angle = r.choice([r.f(), r.f(10), fb(r.uniform(-7, 7)), fb(r.uniform(-1, 1)), fb(r.choice([1e19, -1e19, 3e38, 2.0 ** 63, 0.5, -1.0, 1.0])),
                      r.choice([0, 0x80000000, 0x7fc00000, 0x7f800001, 0x7f800000, 0xff800000])])
    if 1e6 / math.pi / 2 < abs(bf(angle)) < 2.0 ** 63:   # (left out: an angle, or pi times it (plus 1), from about 1e6 to 2^63)
        angle = fb(r.uniform(-1e5, 1e5))
    return Call(0x565110, [RAM, angle], {RAM: r.bytes(0x30)}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

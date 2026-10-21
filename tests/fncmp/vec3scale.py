# A float triple scaled in place (src/game/math/vector: 0x6bb240) against the original: ordinary, tiny (denormal products), huge
# (overflowing products) and odd values (NaNs quiet and signalling, infinities, zeros) in the components and the scale.
# usage: vec3scale.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

B = Bench([(0x6bb240, 0x6bb265)])
ODD = [0x7fc00000, 0xffc00000, 0x7f800001, 0xff800001, 0x7fa00000, 0x7f800000, 0xff800000, 0x80000000, 0]


def value(r):
    return r.choice([fb(r.uniform(-100, 100)), fb(r.uniform(-1, 1)), r.choice(ODD), r.f(), fb(r.uniform(-1, 1) * 1e-20),
                     fb(r.uniform(-1, 1) * 1e20), r.getrandbits(32)])


def case(r):
    v = [value(r) for _ in range(3)]
    return Call(0x6bb240, [RAM, value(r)], {RAM: struct.pack('<3I', *v) + bytes(r.bytes(4))}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

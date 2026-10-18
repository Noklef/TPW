# A float triple clamped in place (src/game/math/vector: 0x5ebe50) against the original: components below, at, between and above the
# bounds, bounds the wrong way round, and NaNs (quiet and signalling) and infinities in the components or the bounds.
# usage: vec3clamp.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

B = Bench([(0x5ebe50, 0x5ebedf)])
ODD = [0x7fc00000, 0xffc00000, 0x7f800001, 0xff800001, 0x7f800000, 0xff800000, 0x80000000, 0]


def bound(r):
    return r.choice(ODD) if r.random() < 0.08 else fb(r.choice([0.0, 1.0, -1.0, 255.0, r.uniform(-10, 10)]))


def case(r):
    lo, hi = bound(r), bound(r)
    if r.deal([0, 1, 1, 1]) and struct.unpack('<f', struct.pack('<I', lo))[0] > struct.unpack('<f', struct.pack('<I', hi))[0]:
        lo, hi = hi, lo
    v = [r.choice([lo, hi, r.choice(ODD), r.f(), fb(r.uniform(-12, 12))]) for _ in range(3)]
    return Call(0x5ebe50, [RAM, lo, hi], {RAM: struct.pack('<3I', *v) + bytes(r.bytes(4))}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

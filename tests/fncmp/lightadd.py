# The light list's angle and its add (src/game/render/lightlist: 0x584b20, 0x584be0) against the originals, with the C library's asin and acos real. Directions any way round, zero and NaN ones among them; the global vector at 0x768ab0 varied.
# usage: lightadd.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

B = Bench([(0x584b20, 0x584bd7), (0x584be0, 0x584e2e), (0x67a400, 0x690000)])
CRT_FLAG = 0xfb1330   # (set: asin and acos past 1 return their NaN without the math error path, whose start-up tables a bench does not have)
NAN = struct.unpack('<f', struct.pack('<I', 0x7fc00000))[0]


def comp(r):
    k = r.random()
    if k < 0.08:
        return 0.0
    if k < 0.12:
        return r.choice([NAN, float('inf'), -float('inf'), 1e30, -1e30, 1e-30])
    if k < 0.3:
        return r.choice([-1.0, 1.0, 0.5, -0.5, 2.0])
    return r.uniform(-100.0, 100.0)


def case(r):
    regions = {CRT_FLAG: b'\1\0\0\0'}
    if r.random() < 0.25:
        return Call(0x584b20, [fb(comp(r)) for _ in range(4)], regions, ret='d')
    mem = bytearray(r.bytes(0x1d0))
    struct.pack_into('<I', mem, 0x1c0, r.choice([0, 0, 1, 2, 3, 3, 4, 5, 0x7fffffff]))
    regions[RAM] = mem
    regions[0x768ab0] = struct.pack('<3f', comp(r), comp(r), comp(r))
    return Call(0x584be0, [RAM, fb(comp(r)), fb(comp(r)), fb(comp(r))], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

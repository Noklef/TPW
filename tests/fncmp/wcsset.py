# The wide string's fill (CRT_wcsset 0x6d76f0, src/game/core/crt) against the original: strings of 0 to 0x40 units, the junk after the terminator kept; the unit any.
# usage: wcsset.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x6d76f0, 0x6d770e)])


def case(r):
    n = r.choice([0, 1, 2, 3, r.randrange(0x40)])
    text = struct.pack('<%dH' % n, *(r.randrange(1, 0x10000) for _ in range(n))) + b'\0\0' + r.bytes(0x20)
    return Call(0x6d76f0, [RAM, r.choice([0, 0x2a, 0x10000 | 0x2a, r.getrandbits(32)])], {RAM: text}, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

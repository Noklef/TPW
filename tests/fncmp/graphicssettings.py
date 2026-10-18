# The graphics settings applied (src/game/render/graphicssettings: 0x44d8f0) against the original: each of the 17 settings (0x785878) one of its
# values, the edges past them or junk; the flag words, the view's depth range and the landscape's detail level over junk.
# usage: graphicssettings.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x44d8f0, 0x44dc52), (0x583280, 0x5832f0), (0x585710, 0x58571d)])


def case(r):
    values = struct.pack('<17I', *(r.choice([0, 0, 1, 1, 2, 2, 3, 3, 4, 5, r.getrandbits(32)]) for _ in range(17)))
    regions = {0x785878: values, 0x8bd550: r.bytes(4), 0x7a1a8c: r.bytes(4), 0x79d86c: r.bytes(4), 0x8c162c: r.bytes(4), 0x8bd4c0: r.bytes(4),
               0x8bcbc8: r.bytes(4), 0x8bcbc8 + 0x8c8: r.bytes(0x18), 0x8c1828 + 0x2ec8: r.bytes(4)}
    return Call(0x44d8f0, [], regions, conv='c')


if __name__ == '__main__':
    B.main(case)

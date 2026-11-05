# A level's loaded things freed (src/game/world/levelfree: 0x457d50 Level_FreeAll) against the original, all its callees recorders; keep's byte
# set or not with its high bytes random, the rides' table set or not, the counts at their limits.
# usage: levelfree.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region  # noqa: E402

CALLEES = {0x53c810: (1, 0), 0x4628a0: (2, 0), 0x598ad0: (3, 0), 0x429e20: (4, 1), 0x454540: (5, 0), 0x437150: (6, 0), 0x430ad0: (7, 0), 0x458580: (8, 0),
           0x450e80: (9, 0), 0x52f190: (10, 0), 0x541ed0: (11, 0), 0x463c80: (12, 1), 0x450a80: (13, 0), 0x463ed0: (14, 0), 0x469550: (15, 0), 0x46ea80: (16, 0)}
B = Bench([(0x457d50, 0x457e03)], hooks={address: _recorder(k, n, cdecl=True) for address, (k, n) in CALLEES.items()})


def case(r):
    table = struct.pack('<8I', *[r.choice([0, r.getrandbits(32), r.getrandbits(32)]) for _ in range(8)])
    a = r.randrange(0, 6)
    regions = {LOG: log_region(), 0x7a4614: table, 0x7a3ebc: struct.pack('<I', a), 0x7a4084: struct.pack('<I', r.randrange(0, 9 - a if a < 6 else 3))}
    keep = r.choice([0, 1, 0x100, 0xff00, r.getrandbits(32), r.getrandbits(8)])
    return Call(0x457d50, [keep], regions)


if __name__ == '__main__':
    B.main(case)

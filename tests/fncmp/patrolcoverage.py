# How much of a class of cells a kind of staff patrols (src/game/world/patrolcoverage: PatrolCoverage_Percent, 0x4cd500) against the original: a map as mapcount.py's, the dword +0x3c of each cell 0, above 0 or negative,
# in a share that changes from case to case. Staff_MarkPatrolCells a stdcall recorder 10 (its own bench has it; driver.cpp's kPatrolCoverageEnv), the cell tests run. The group 0-3 mostly, now and then past them.
# usage: patrolcoverage.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

B = Bench([(0x4cd500, 0x4cd705), (0x536300, 0x536484), (0x53ad20, 0x53aefc), (0x4d0a70, 0x4d0b27), (0x522700, 0x5228ac), (0x5d60c0, 0x5d60c3)], hooks={0x4cd340: _recorder(10, 1, True)})
TYPES = [0, 1, 2, 3, 4, 5, 7, 9, 0xa, 0xb, 0xc, 0xd, 0x10, 0x11, 0x12, 0x15, 0x19, 0x1e, 0x115, 0x111]


def case(r):
    mem = bytearray(r.bytes(0x10ffd8 + 0x28 * 0x400))
    for h in range(0x400):
        put(mem, 0x10ffd8 + 0x28 * h + 8, 'I', r.choice(TYPES))
        put(mem, 0x10ffd8 + 0x28 * h + 0x10, 'H', r.choice([0, 0, r.randrange(0x400)]))
    kinds = r.sample(TYPES, r.choice([1, 3, 6, len(TYPES)]))
    share = r.choice([0, 0.001, 0.1, 0.5, 1])
    for c in range(0x4000):  # (the last cells' fields overlap the first entries: set after them)
        put(mem, 0x44 * c, 'H', r.randrange(0x400))
        put(mem, 0x44 * c + 8, 'I', r.choice(kinds))
        put(mem, 0x44 * c + 0x3c, 'i', r.choice([1, 2, 0x7fffffff]) if r.random() < share else r.choice([0, 0, -1, -0x80000000]))
    world = RAM - 0x2d8
    regions = {LOG: log_region(), 0x80239c: put(put(bytearray(8), 0, 'I', world), 4, 'I', RAM), 0x7cf83c: struct.pack('<I', RAM + 0x10ffd8 - 0x1102b0), RAM: mem}
    return Call(0x4cd500, [r.getrandbits(32), r.choice([0, 1, 1, 2, 3, 3, 4, r.getrandbits(32)])], regions, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)

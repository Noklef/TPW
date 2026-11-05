# The start-up of the collision pools (src/game/world/collobj: 0x544360 CollPool_Startup) against the original: the integer
# sine table, the block of counts and six GlobalAllocs (k 1, each answered in turn), the free list through the last and a
# rand. It has no input but what it overwrites, and each case runs 0x200 sines and 0x400 list entries: 12 cases.
# usage: collpoolstartup.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, RAND_SEED, Bench, Call, answers, log_region, recorder  # noqa: E402

ALLOC = 1
B = Bench([(0x544360, 0x544446), (0x67a830, 0x67a860)])
TABLE, POOL = RAM, RAM + 0x1000
CAP = 12


def case(r):
    log = log_region()
    blocks = [r.getrandbits(32) for _ in range(5)] + [POOL]
    answers(log, ALLOC, TABLE)
    return Call(0x544360, [], {LOG: log, TABLE: struct.pack('<40I', *blocks, *[0] * 34), POOL: r.bytes(0x5000),
                               0x6fd1cc: struct.pack('<I', recorder(ALLOC, 2, True)), 0x877358: r.bytes(0x840),
                               RAND_SEED: r.bytes(4)}, ret='a')


if __name__ == '__main__':
    if len(sys.argv) > 1:
        sys.argv[1] = str(min(int(sys.argv[1]), CAP))
    B.main(case)

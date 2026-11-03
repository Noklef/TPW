# A flying ride's record made (src/game/world/flyinglaunch: FlyingRide_Create 0x55a620) against the original. FlyingRide_Launch 0x55a720 is a cdecl recorder (the driver's FlyingLaunchEnv, 5 of 1); the block
# comes from global_heap() (0x5f5660 runs as it is). The fixture: the table 0x8791f8 with a hole anywhere or none, the allocation failing now and then (a fault in both).
# usage: flyingcreate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, _recorder, global_heap, heap, log_region  # noqa: E402

B = Bench([(0x55a620, 0x55a71e), (0x5f5660, 0x5f5690), (0x5da3c0, 0x5da3c1)], hooks={0x55a720: _recorder(5, 1, cdecl=True)})


def case(r):
    log = log_region()
    table = [r.getrandbits(32) | 1 for _ in range(100)]
    table[0] = r.choice([0, 0x1234])     # (entry 0 is never looked at)
    hole = r.choice([1, 2, 50, 98, 99, 100, r.randrange(1, 100)])
    if hole < 100:
        table[hole] = 0
    regions = {LOG: log, HEAP: heap(0x2000, 0 if r.random() < 0.97 else 1), 0x8791f8: struct.pack('<100I', *table), **global_heap()}
    args = [r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.choice([0, 0x400, r.randrange(0x800)]), r.choice([0, 0, 1, r.getrandbits(32)])]
    return Call(0x55a620, args, regions, ret='a')


if __name__ == '__main__':
    B.main(case)

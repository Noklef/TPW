# A track car freed (src/game/coaster/train: 0x434570 Car_Free) against the original: the car's ride (+0x70) none or one (Ride_Destroy
# 0x463c80 a cdecl recorder, 31), its definition (+0x100) with flag 1 (+4) or not, its blocks (+0x7c, +0x80, +0x88, +0x8c) and itself
# freed through the GlobalFree import (global_heap: the pointers hashed); the other car none or one with flag 8 (+0) or not; the selected
# car (0x79c640) the car, the other or any.
# usage: carfree.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_region, put  # noqa: E402

B = Bench([(0x434570, 0x43464e), (0x45ba00, 0x45ba20), (0x4418d0, 0x4418f0)], hooks={0x463c80: _recorder(31, 1, cdecl=True)})
CAR, OTHER, DEF = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000


def case(r):
    car = bytearray(r.bytes(0x110))
    put(car, 0x70, 'I', r.choice([0, 0, 1, 7, r.getrandbits(32)]))
    for o in (0x7c, 0x80, 0x88, 0x8c):
        put(car, o, 'I', r.choice([0, RAM + 0x10000 + o * 0x10, r.getrandbits(32)]))
    put(car, 0x100, 'I', DEF)
    definition = bytearray(r.bytes(8))
    definition[4] = r.getrandbits(8) & ~1 | r.choice([0, 1])
    other = bytearray(r.bytes(0x10))
    other[0] = r.getrandbits(8) & ~8 | r.choice([0, 8, 8])
    second = r.choice([0, OTHER, OTHER])
    regions = {CAR: car, OTHER: other, DEF: definition, HEAP: heap(), LOG: log_region(), **global_heap(),
               0x79c640: struct.pack('<I', r.choice([CAR, CAR, OTHER, 0, r.getrandbits(32)]))}
    return Call(0x434570, [CAR, second], regions)


if __name__ == '__main__':
    B.main(case)

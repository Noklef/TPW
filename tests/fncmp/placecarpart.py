# A coaster's car placed with no claim, its part's +0x70 given back (src/game/coaster/placecar: 0x4419b0 CoasterEdit_PlaceCarPart) against the original:
# CoasterEdit_PlaceCar (k 3) a recorder, filling the node and car it is given or leaving the arguments there; the out pointer or none.
# usage: placecarpart.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, set_return, strings  # noqa: E402

PLACE = 3
B = Bench([(0x4419b0, 0x4419f9)], hooks={0x435020: _recorder(PLACE, 7, cdecl=True)})
CAR_A, CAR_B, OUT = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    log = log_region()
    set_return(log, PLACE, r.getrandbits(32))
    strings(log, PLACE, 0, out=1 << 3 | 1 << 4)
    if r.random() < 0.7:
        fill(log, PLACE, 3, struct.pack('<I', r.choice([0, r.getrandbits(32) | 1])), which=0)
    if r.random() < 0.7:
        fill(log, PLACE, 4, struct.pack('<I', CAR_B), which=1)
    regions = {LOG: log, CAR_A: r.bytes(0x80), CAR_B: r.bytes(0x80), OUT: r.bytes(4)}
    return Call(0x4419b0, [r.getrandbits(32), CAR_A, r.choice([0, r.getrandbits(32)]), r.choice([0, OUT, OUT, OUT])], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

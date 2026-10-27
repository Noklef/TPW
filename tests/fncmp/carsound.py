# A coaster car's sound mode (src/game/coaster/carsound: 0x439780) against the original, over the sound globals as rssescream.py
# sets them (on 0x802bc8 and started 0x802bd4 each 0 or not, the manager 0x802bcc, its engine's slot 3 recorder, its result now
# and then set); the car's mode +0x38 the new one now and then, the mode 0-17, -1 or any, 0x785a2c 0 or not.
# usage: carsound.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x439780, 0x439812), (0x51bfc0, 0x51c048), (0x51bc40, 0x51bc70), (0x6b5b60, 0x6b5b75),
           (0x6b5b80, 0x6b5b9a), (0x5da3c0, 0x5da3c1)])
MANAGER, VT, ENGINE, EVT, CAR = RAM + 0x100, RAM + 0x200, RAM + 0x800, RAM + 0x900, RAM + 0x1000


def case(r):
    log = log_region()
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1, 1, r.getrandbits(32)]))
    if r.random() < 0.6:
        set_return(log, 3, r.choice([0, r.getrandbits(32)]))
    mode = r.choice(list(range(18)) + [1, 2, 4, 8, 16, -1, r.getrandbits(32)]) & 0xffffffff
    car = bytearray(r.bytes(0x80))
    if r.random() < 0.3:
        put(car, 0x38, 'I', mode)
    regions = {LOG: log, 0x802bc0: g, MANAGER: struct.pack('<II', VT, ENGINE) + r.bytes(8), VT: vtable({}, 4),
               ENGINE: struct.pack('<I', EVT) + r.bytes(0x30), EVT: vtable({3: 3}, 8), CAR: car,
               0x785a2c: struct.pack('<I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))}
    return Call(0x439780, [CAR, mode, r.getrandbits(32) if r.random() < 0.5 else r.randrange(200)], regions)


if __name__ == '__main__':
    B.main(case)

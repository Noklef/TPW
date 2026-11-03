# Guard_SetState (src/game/world/staff, 0x4d65d0) against the original. Visitor_Arrive thiscall recorder 10, Staff_SetState thiscall recorder 11 (their own benches have them; driver.cpp's kGuardStateEnv). The states 0, 7 (the old state 0x12 mostly, the handle
# 0 or its slot empty now and then), 0x10, 0x12, 0x13, others under and over 0x13.
# usage: guardstate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

B = Bench([(0x4d65d0, 0x4d6759), (0x5da3c0, 0x5da3c1)], hooks={0x4feb50: _recorder(10, 1), 0x5054d0: _recorder(11, 1)})
GUARD = RAM + 0x100


def case(r):
    log = log_region()
    guard = bytearray(r.bytes(0x240))
    state = r.choice([0, 7, 7, 7, 7, 0x10, 0x12, 0x13, 1, 2, 6, 8, 0xf, 0x11, 0x14, 0x107, r.getrandbits(32)])
    put(guard, 0x19c, 'I', r.choice([0x12, 0x12, 0x12, 0x12, 0, 7, 0x13, 0x112, r.getrandbits(32)]))
    handle = r.choice([1, 2, 3, 4, 0x100, 0])
    put(guard, 0x212, 'H', handle)
    put(guard, 0x1e4, 'I', r.randrange(4))
    slots = bytearray(r.bytes(0x20 * 20))
    for i in range(1, 5):
        put(slots, 20 * i, 'I', r.choice([RAM + 0x800, RAM + 0x800, RAM + 0x800, 0]))
    regions = {LOG: log, GUARD: guard, 0x7cfb90: slots, 0x75c7f0: r.bytes(4), 0x785498: r.bytes(0x20)}
    if handle == 0x100:
        regions[0x7cfb90 + 20 * 0x100] = struct.pack('<I', r.choice([RAM + 0x800, 0]))
    return Call(0x4d65d0, [GUARD, state], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

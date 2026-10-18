# The challenges' orphan drop, current challenge and schema slot (src/game/world/challenges:
# 0x4d1c80, 0x4d1e60, 0x4d2840) against the originals: ids and prerequisites drawn from a few
# values (so that they match, chains drop in order, and some name themselves); current indexes
# about 0 and 20; and the guard's state checks (world/staff: 0x4d6330, 0x4d6350). The offer accepted or declined (0x4d2df0, 0x4d2ec0):
# waiting or not, target times and prizes about 0 and the wraps, independent or not; the world's calendar clock and its imports
# (funnytime.py's), the money record and park through the world's handles (as stafftrain.py).
# usage: challenges.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE as IAT, LOG, Bench, Call, RAM, fill, import_table, log_region, put, set_return  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from funnytime import IMPORTS, K, RANGES as CLOCK_RANGES  # noqa: E402

B = Bench([(0x4d1c80, 0x4d1cdc), (0x4d1e60, 0x4d1e74), (0x4d2840, 0x4d285d), (0x4d6330, 0x4d6348),
           (0x4d6350, 0x4d635f), (0x4d2df0, 0x4d2eb7), (0x4d2ec0, 0x4d2fac), (0x4d0190, 0x4d01f0), (0x5195d0, 0x519610),
           (0x519510, 0x519550), (0x5da3c0, 0x5da3c1)] + CLOCK_RANGES)
C = RAM + 0x100
MONEY, PARK, WORLD = RAM + 0x1000, RAM + 0x40000, RAM + 0x80000
DAYS = [0, 1, 7, 30, 365, -1, -30, 0x7fffffff, -0x80000000, 0x10000]
AMOUNTS = [0, 1, 2, 3, 1000, 5001, -1, -2, -3, 0x7fffffff, -0x80000000]


def offer(r, mem, op):
    """Accept / Decline: the challenges in mem at C, the world (clock, ticks, handles), the money record and park, the imports."""
    mem[0x100 + 0x3da] = r.choice([0, 1, 1, 1, 0xff])
    for i in range(0x14):
        c = 0x100 + 0xc + i * 0x30
        put(mem, c + 4, 'i', r.choice(DAYS + [r.randrange(-1000, 1000)]))
        put(mem, c + 0x14, 'i', r.choice(AMOUNTS + [r.randrange(-100000, 100000)]))
        mem[c + 0x1d] = r.choice([0, 0, 1, 0xff])
    index = r.choice([0, 1, 19, r.randrange(20)] + ([] if op == 'Decline' and mem[0x100 + 0x3da] else [-1, 20, r.getrandbits(32) - (1 << 31)]))
    put(mem, 0x100 + 0x3cc, 'i', index)   # (out of range only where the decline would not read it: the original faults)
    log = log_region()
    fill(log, K['FileTimeToSystemTime'], 1, r.bytes(16), 0)
    set_return(log, K['FileTimeToSystemTime'], r.choice([0, 1]))
    clock = bytearray(r.bytes(0x20))
    put(clock, 0x1c, 'I', r.choice([15000, 1, 0, r.getrandbits(32)]))
    world = bytearray(r.bytes(0x40))
    put(world, 0xc, 'I', r.choice([0, 4, r.randrange(1 << 20), r.getrandbits(32)]))   # the ticks, +0x1da70c
    put(world, 0x20, 'H', 2)   # the park, +0x1da720 ([0x7cf83c]'s too)
    put(world, 0x26, 'H', 1)   # the money record, +0x1da726
    slots = bytearray(r.bytes(20 * 4))
    put(slots, 20 * 1, 'I', MONEY)
    put(slots, 20 * 2, 'I', PARK)
    regions = {RAM: mem, LOG: log, IAT: import_table(log, IMPORTS), WORLD + 0x2a0: clock, WORLD + 0x1da700: world, 0x7cfb90: slots,
               MONEY: r.bytes(0x200), PARK + 0x1fc90: r.bytes(4), 0x80239c: struct.pack('<I', WORLD), 0x7cf83c: struct.pack('<I', WORLD),
               0x7857c4: struct.pack('<i', r.choice(DAYS))}
    return Call(0x4d2df0 if op == 'Accept' else 0x4d2ec0, [C], regions, conv='t')


def case(r):
    mem = r.bytes(0x600)
    op = r.deal(OPS)
    if op in ('Accept', 'Decline'):
        return offer(r, bytearray(mem), op)
    op = ['Drop', 'Current', 'Schema'].index(op) if op != 'Guard' else 3
    if op == 0:
        k = r.choice([3, 6, 25, 1 << 32])
        for i in range(0x14):
            put(mem, 0x100 + 0xc + i * 0x30, 'I', r.choice([0, r.randrange(k)]) if k < 1 << 32 else r.getrandbits(32))
            put(mem, 0x100 + 0xc + i * 0x30 + 0x18, 'I', r.choice([0, r.randrange(min(k, 30))]))
        return Call(0x4d1c80, [C], {RAM: mem}, conv='t')
    if op == 1:
        put(mem, 0x100 + 0x3cc, 'i', r.choice([-1, 0, 1, 19, 20, 21, r.randrange(-3, 23), r.getrandbits(32) - (1 << 31)]))
        return Call(0x4d2840, [C], {RAM: mem}, conv='t', ret='a')
    if op == 2:
        return Call(0x4d1e60, [C, r.choice([0, 1, r.randrange(30), r.getrandbits(32)])], {RAM: mem}, conv='t', ret='a')
    put(mem, 0x100 + 0x19c, 'I', r.choice([0, 1, 2, 0x12, 0x13, r.randrange(30), r.getrandbits(32), 0x100, 0x112]))
    return Call(r.choice([0x4d6330, 0x4d6350]), [C], {RAM: mem}, conv='t', ret='a')


OPS = ['Drop', 'Current', 'Schema', 'Guard', 'Guard', 'Accept', 'Decline']

if __name__ == '__main__':
    B.main(case)

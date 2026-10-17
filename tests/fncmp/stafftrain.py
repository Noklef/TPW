# The staff member's training of src/game/world/staff (0x505a10) against the original: staff types 4-8 and others, grades 0-4 and beyond, progress bytes either side of the
# 100 mark, amounts of either sign and cost tables (in .bss: laid out here) with costs of 0, -1 and others; the money record and park found through the world's handles.
# usage: stafftrain.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x505a10, 0x505b70), (0x4d01f0, 0x4d0260), (0x519510, 0x519550), (0x5195d0, 0x519610), (0x5da3c0, 0x5da3c1)])
STAFF, MONEY, PARK, WORLD = RAM, RAM + 0x1000, RAM + 0x40000, RAM + 0x80000
COSTS = 0x7853a0


def case(r):
    staff = bytearray(r.bytes(0x200))
    staff[2] = r.choice([4, 5, 6, 7, 8, 4, 5, 6, 7, 8, 0, 3, 9, 0xff])
    put(staff, 0x1e4, 'i', r.choice([0, 1, 2, 3, 4, 0, 1, 2, 3, 5, -1]))
    staff[0x1e8] = r.choice([0, 1, 50, 98, 99, 100, 101, 0xff, r.getrandbits(8)])
    costs = bytearray(0x130)
    for at in range(0, len(costs), 4):
        put(costs, at, 'i', r.choice([1, 10, 100, 250, 1000, 5000, r.randrange(1, 100000)] * 3 + [0, -1, -100]))
    money = bytearray(r.bytes(0x200))
    put(money, 0x114, 'I', r.choice([0, 1, 1]))
    put(money, 0x11c, 'i', r.choice([-1, 0, 5]))
    slots = bytearray(r.bytes(20 * 4))
    put(slots, 20 * 1, 'I', MONEY)
    put(slots, 20 * 2, 'I', PARK)
    world = bytearray(r.bytes(0x40))
    put(world, 0xc, 'I', r.getrandbits(32))     # the time, +0x1da70c
    put(world, 0x20, 'H', 2)                    # the park, +0x1da720 (also [0x7cf83c]'s, for the debit)
    put(world, 0x26, 'H', 1)                    # the money record, +0x1da726
    amount = r.choice([0, 1, 50, 100, 999, 1000, 5000, 100000, -1, -1000, 0x7fffffff, -0x80000000, r.randrange(-1000, 200000)])
    regions = {STAFF: staff, COSTS: costs, MONEY: money, PARK + 0x1f5a0: r.bytes(4), PARK + 0x1fa40: r.bytes(4), PARK + 0x1fc90: r.bytes(4), 0x7cfb90: slots,
               WORLD + 0x1da700: world, 0x80239c: struct.pack('<I', WORLD), 0x7cf83c: struct.pack('<I', WORLD)}
    return Call(0x505a10, [STAFF, amount & 0xffffffff], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

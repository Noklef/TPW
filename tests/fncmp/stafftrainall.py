# The training of all the staff of src/game/world/staff (0x50c800) against the original: a park of up to six things (thingenv.py) of the staff kinds 4-8 and
# others, five amounts (per type) of either sign, each staff member trained (Staff_Train, see stafftrain.py: grades, progress, cost tables in .bss laid out here)
# with its type's amount divided by the type's count; the money record and park the world's handles name are slots 12 and 13.
# usage: stafftrainall.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import POOL, SLOTS, THING, park  # noqa: E402

B = Bench([(0x50c800, 0x50c9b0), (0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1),
           (0x505a10, 0x505b70), (0x4d01f0, 0x4d0260), (0x519510, 0x519550), (0x5195d0, 0x519610)])
MONEY, PARK, AMOUNTS = RAM + 0x1000, RAM + 0x40000, RAM + 0x60000
COSTS = 0x7853a0


def case(r):
    regions, handles, _things = park(r, kinds=(4, 5, 6, 7, 8, 4, 5, 6, 7, 8, 3, 1, 9), defs=False)
    things = regions[POOL]
    for i in range(len(handles)):
        put(things, THING * i + 0x1e4, 'i', r.choice([0, 1, 2, 3, 4, 0, 1, 2, 3, 5]))
        things[THING * i + 0x1e8] = r.choice([0, 1, 50, 98, 99, 100, r.getrandbits(8)])
    slots = bytearray(regions[SLOTS]) + bytearray(r.bytes(20 * 2))
    put(slots, 20 * 12, 'I', MONEY)
    put(slots, 20 * 13, 'I', PARK)
    regions[SLOTS] = slots
    costs = bytearray(0x130)
    for at in range(0, len(costs), 4):
        put(costs, at, 'i', r.choice([1, 10, 100, 250, 1000, 5000, r.randrange(1, 100000)] * 3 + [0, -1, -100]))
    money = bytearray(r.bytes(0x200))
    put(money, 0x114, 'I', r.choice([0, 1, 1]))
    put(money, 0x11c, 'i', r.choice([-1, 0, 5]))
    world = bytearray(r.bytes(0x40))
    put(world, 0x20, 'H', 13)   # the park, +0x1da720
    put(world, 0x26, 'H', 12)   # the money record, +0x1da726
    amounts = b''.join(struct.pack('<i', r.choice([0, 1, 3, 100, 999, 1000, 5000, 100000, -1, -1000, 0x7fffffff, -0x80000000, r.randrange(-1000, 200000)]))
                       for _ in range(5))
    regions.update({COSTS: costs, MONEY: money, PARK + 0x1f5a0: r.bytes(4), PARK + 0x1fa40: r.bytes(4), PARK + 0x1fc90: r.bytes(4),
                    RAM + 0x1da700: world, AMOUNTS: amounts})
    return Call(0x50c800, [AMOUNTS], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

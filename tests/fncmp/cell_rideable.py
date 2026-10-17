# The rideable object search of src/game/world/mapcell (0x42a340 Cell_FindRideableObject) against the original: a cell of type 9 or not whose parent cell has a chain of up to four
# things (model 3 or others, definitions that forbid riding or not), through the world pointer behind [0x7cf6ec].
# usage: cell_rideable.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, POOL, SLOTS, THING, park  # noqa: E402

B = Bench([(0x412e90, 0x412f00), (0x42a340, 0x42a420), (0x4dd4e0, 0x4dd510), (0x536340, 0x536360), (0x5da3c0, 0x5da3c1)])
POINTER, WORLD = RAM + 0x100, RAM + 0x10000


def case(r):
    count = r.choice([1, 2, 3, 4])
    regions, _handles, _things = park(r, count=count, defs=True)
    items = bytearray(regions[ITEMS])
    for i in range(NITEMS):
        put(items, i * ITEM + 0x118, 'I', r.choice([0, 0, 1, 5]))
    regions[ITEMS] = items
    n = struct.unpack_from('<i', regions[0x7890a0], 0x528)[0]
    pool = bytearray(regions[POOL])
    slots = regions[SLOTS]
    live = [k for k in range(1, 12) if POOL <= struct.unpack_from('<I', slots, 20 * k)[0] < POOL + 6 * THING]
    handles = [struct.unpack_from('<H', pool, THING * i)[0] for i in range(len(live))]
    for i in range(len(handles)):
        pool[THING * i + 2] = r.choice([3, 3, 3, 1, 2])
        put(pool, THING * i + 0xe, 'H', struct.unpack_from('<H', items, ITEM * r.randrange(n))[0])
        put(pool, THING * i + 0xa, 'H', handles[i + 1] if i + 1 < len(handles) else 0)
    regions[POOL] = pool
    x, y = r.choice([0, 1, 5, 127]), r.choice([0, 1, 2])
    cell_id = (((y << 7) + x + 1) & 0xffff)
    parent = r.choice([cell_id, 1, 2, 7, 0x20])
    cell = put(bytearray(r.bytes(0x44)), 8, 'I', r.choice([9, 9, 9, 3, 0]))
    put(cell, 0x10, 'H', parent)
    regions[WORLD + 0x294 + cell_id * 0x44] = cell
    if parent != cell_id:
        pcell = bytearray(r.bytes(0x44))
    else:
        pcell = cell
    put(pcell, 0x24, 'H', r.choice(handles + [0]) if handles else 0)
    if parent == cell_id:
        put(pcell, 8, 'I', r.choice([9, 9, 3]))
        regions[WORLD + 0x294 + cell_id * 0x44] = pcell
    else:
        regions[WORLD + 0x294 + parent * 0x44] = pcell
    regions[0x7cf6ec] = struct.pack('<I', POINTER)
    regions[POINTER] = struct.pack('<I', WORLD)
    return Call(0x42a340, [x, y], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

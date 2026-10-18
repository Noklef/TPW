# The rideable object search of src/game/world/mapcell (0x42a340 Cell_FindRideableObject) against the original: a cell of type 9 or not whose parent cell has a chain of up to four
# things (model 3 or others, definitions that forbid riding or not), through the world pointer behind [0x7cf6ec]. Snap: the camera's move to
# the ride there (src/game/world/camera: Camera_SnapToRide 0x42a1d0, over MapCell_Direction 0x522850): the point about the cell (often within it,
# else negative, huge or not a number), the map's size about it, the things' cells (+0x38) among 0x280 cells whose directions are often one bit.
# usage: cell_rideable.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, POOL, SLOTS, THING, park  # noqa: E402

B = Bench([(0x412e90, 0x412f00), (0x42a340, 0x42a420), (0x4dd4e0, 0x4dd510), (0x536340, 0x536360), (0x5da3c0, 0x5da3c1), (0x42a1d0, 0x42a2e6),
           (0x522850, 0x522854)])
POINTER, WORLD = RAM + 0x100, RAM + 0x10000
NCELLS = 0x280   # Snap's cells (the things' cells and those the clamped point reaches)


def case(r):
    op = r.deal(OPS)
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
    if op == 'Find':
        return Call(0x42a340, [x, y], regions, conv='c', ret='a')
    cells = bytearray(r.bytes(0x44 * NCELLS))
    for k in range(NCELLS):
        cells[0x44 * k + 0xd] = r.choice([1, 4, 0x10, 0x40, 0x41, 0x14, 0, r.getrandbits(8)])
    for at in [a for a in regions if WORLD + 0x294 <= a < WORLD + 0x294 + len(cells)]:
        cells[at - WORLD - 0x294:at - WORLD - 0x294 + 0x44] = regions.pop(at)
    regions[WORLD + 0x294] = cells
    for i in range(len(handles)):
        put(pool, THING * i + 0x38, 'H', r.choice([0, 1, cell_id, parent, r.randrange(NCELLS)]))
    fx, fz = (r.choice([v * 10 + r.uniform(0, 10)] * 4 + [-0.5, -25.0, 1e12, float('nan')]) for v in (x, y))
    regions[0x7908f0] = struct.pack('<fff', fx, r.uniform(-10, 10), fz)
    regions[0x790a38] = r.bytes(4)
    regions[0x7a0314] = struct.pack('<i', r.choice([128, 128, x + 1, 1, 200]))
    regions[0x7a0374] = struct.pack('<i', r.choice([y + 1, y + 1, 3, 4, 1]))
    return Call(0x42a1d0, [], regions, conv='c')


OPS = [k for k in os.environ.get('OPS', 'Find,Snap').split(',') if k]


if __name__ == '__main__':
    B.main(case)

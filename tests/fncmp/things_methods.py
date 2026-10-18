# The thing's own methods of src/game/world/things (0x50afe0 the constructor, 0x50b6a0 the move between cells, 0x4fa930 the placement, 0x4fcac0 the sprite choice, 0x4e2200 the
# ingredient record) against the originals:
#   SetPos: things in a consistent table (thingenv's), a cell grid whose chains hold none or a thing, positions in the same cell or another;
#   Construct: things.py's table of slots (a free one with its own id) and a short row of cells;
#   Place: SetPos' world, through the navigation block at +0xd4 (ResetPlace, 0x4f7e20: the same after setting +0x39c and +0x10);
#   Choice: a thing with sprite folders that are empty or not and a world seed;
#   Record: a thing whose definition has the ingredient value 0 to 5 and a table of records (some with a terminating zero word, some not).
# usage: things_methods.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetPos,Construct,Place,ResetPlace,Choice,Record)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import things  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import CELLS, ITEM, ITEMS, NITEMS, POOL, THING, park  # noqa: E402

B = Bench([(0x4f7e20, 0x4f7e4c), (0x4217f0, 0x421800), (0x4d91f0, 0x4d93c0),(0x4e2200, 0x4e22a0), (0x4fa930, 0x4fa9a0), (0x4fcac0, 0x4fcb20), (0x50afe0, 0x50b090), (0x50b350, 0x50b370), (0x50b6a0, 0x50b790),
           (0x5100e0, 0x510110), (0x510160, 0x510190), (0x412e90, 0x412f00), (0x516270, 0x516320), (0x516330, 0x516390), (0x541f60, 0x541fb0), (0x6ace60, 0x6aceb0), (0x5da3c0, 0x5da3c1)])
NEWCELLS, NEWTHING = RAM + 0x6000, RAM + 0x5000
STREAM, WORLD, SPRITE = RAM, RAM + 0x2000, RAM + 0x3000
TABLE, ARRAY, RECORDS = RAM + 0x4000, RAM + 0x4100, RAM + 0x4200


def setpos(r):
    count = r.choice([1, 2, 3, 4])
    regions, handles, things_ = park(r, count=count, defs=False, cells=True)
    pool = bytearray(regions[POOL])
    grid = bytearray(regions[CELLS])
    for i in range(count):
        put(pool, THING * i + 8, 'HH', r.choice(handles + [0]), r.choice(handles + [0]))
        pool[THING * i + 5], pool[THING * i + 7] = r.choice([0, 1, 127, 128, 255, r.randrange(256)]), r.randrange(4)
    for i in range(len(grid) // 0x44):
        put(grid, 0x44 * i + 0x24, 'H', r.choice(handles + [0]))
    regions[POOL], regions[CELLS] = pool, grid
    me = r.randrange(count)
    x, y = r.getrandbits(32), r.getrandbits(32)
    if r.random() < 0.3:  # the same cell
        x = x & ~0xff00 | pool[THING * me + 5] << 8
        y = y & ~0xff00 | pool[THING * me + 7] << 8
    else:
        x = x & ~0xff00 | r.choice([0, 1, 127, 128, 255, r.randrange(256)]) << 8
        y = y & ~0xff00 | r.randrange(4) << 8
    return Call(0x50b6a0, [things_[me], x, y], regions, conv='t', ret='v')


def construct(r):
    things.OPS = ['Add']
    c = things.case(r)
    regions = dict(c.regions)
    cells = bytearray(r.bytes(0x44 * 0x41))
    for k in range(0x41):
        put(cells, 0x44 * k + 0x24, 'H', 0)
    k = r.choice([3, 7])  # the free slot (id k), else the handle is a random word and the cell chain write faults in both
    regions[0x801bbc] = struct.pack('<I', things.SLOTS + 20 * k)
    regions[things.SLOTS + 20 * k] = struct.pack('<IIIII', 0, k, 0, 0, 0)
    regions.update({NEWCELLS: cells, 0x8023a0: struct.pack('<I', NEWCELLS), 0x80239c: struct.pack('<I', RAM), NEWTHING: r.bytes(0x20)})
    cell = r.choice([1, 2, 3, 0x40, 0x41, 0x20, r.randrange(1, 0x41)]) | r.choice([0, 0, 0x10000])
    return Call(0x50afe0, [NEWTHING, cell, r.getrandbits(32)], regions, conv='t', ret='a')


def place(r):
    c = setpos(r)
    c.address = 0x4fa930
    return c


def reset_place(r):
    c = setpos(r)
    c.address = 0x4f7e20
    return c


def choice(r):
    counts = bytearray(0x30)
    for folder in range(4):
        put(counts, 12 * folder, 'I', r.choice([0, 1, 2, 5, 9]) if folder == 3 else r.choice([1, 2, 3, 7, 20]))
    thing = bytearray(r.bytes(0x40))
    put(thing, 0x24, 'I', r.choice([2, 2, 0, 1, 5]))
    world = RAM + 0x10000 - 0x1da708
    regions = {0x763f90: counts, SPRITE: thing, WORLD: r.bytes(0x20), 0x80239c: struct.pack('<I', world), 0x7cf83c: struct.pack('<I', world), RAM + 0x10000: r.bytes(0x20)}
    return Call(0x4fcac0, [SPRITE], regions, conv='t', ret='a')


def record(r):
    regions, handles, things_ = park(r, count=1, defs=True)
    items = bytearray(regions[ITEMS])
    for i in range(NITEMS):
        put(items, i * ITEM + 0x158, 'I', r.choice([0, 1, 2, 3, 4, 5, 0xffffffff]))
    regions[ITEMS] = items
    n = r.choice([0, 2, 4, 4])
    table = bytearray(r.bytes(0x10))
    put(table, 8, 'I', n)
    put(table, 0xc, 'I', ARRAY)
    array = bytearray(r.bytes(0x20))
    records = bytearray(r.bytes(0x40 * 4))
    for k in range(4):
        length = r.choice([0, 1, 4, 5, 8])
        put(array, 4 * k, 'I', RECORDS + 0x40 * k)
        put(records, 0x40 * k, 'I', (length << 8) | r.choice([1, 1, 1, 0, 2]))
        put(records, 0x40 * k + 4 + 4 * ((length + 3) >> 2), 'I', r.choice([0, 0, 1]))
    regions.update({0x78ba8c: struct.pack('<I', TABLE), TABLE: table, ARRAY: array, RECORDS: records})
    return Call(0x4e2200, [things_[0]], regions, conv='t', ret='a')


CASES = {'SetPos': setpos, 'Construct': construct, 'Place': place, 'ResetPlace': reset_place, 'Choice': choice, 'Record': record}


def case(r):
    return CASES[r.choice(OPS)](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

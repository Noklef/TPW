# The object functions of src/game/world/object that read the object definitions and cells (0x4dd7f0 Object_SetCapacity, 0x52c2b0 Object_TrackNeighboursOk) against the originals:
#   Capacity: ridelist.py's definitions and ride records (the limits around the capacity, a sum of limits above or not);
#   Track: an object of track type 0, 1, 2 or others at an angle of 0, 0x5a, 0xb4, 0x10e or another, the cells it looks at laid out one by one with one neighbour bit or more.
# usage: object_checks.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Capacity,Track)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import ridelist  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, park  # noqa: E402

B = Bench([(0x412e90, 0x412f00), (0x4dd4e0, 0x4dd510), (0x4d8c00, 0x4d8c30), (0x4dd7f0, 0x4dd930), (0x522770, 0x5227f0), (0x52c2b0, 0x52c430), (0x55a070, 0x55a110), (0x55a0b0, 0x55a110), (0x5da3c0, 0x5da3c1)])
OBJ, WORLD = RAM, RAM + 0x10000


def capacity(r):
    ridelist.OPS = ['Dur']
    c = ridelist.case(r)
    c.address = 0x4dd7f0
    obj = c.regions[RAM]
    n = struct.unpack_from('<i', c.regions[0x7890a0], 0x528)[0]   # only the first n definitions are searched
    put(obj, 0xe, 'H', struct.unpack_from('<H', c.regions[0x30008000], 0x500 * r.randrange(n))[0])
    for i in range(16):
        put(c.regions[0x30008000], 0x500 * i + 0x124, 'i', r.choice([0, 1, 5, 100, 0x7f, 0x100, -1, 300]))
        put(c.regions[0x30008000], 0x500 * i + 0x128, 'i', r.choice([0, 1, 5, 100, 0x7f, 0x100, -1, 300]))
    return c


def track(r):
    regions, _h, _a = park(r, count=0, defs=True)
    items = bytearray(regions[ITEMS])
    for i in range(NITEMS):
        put(items, i * ITEM + 0x9c, 'I', r.choice([0, 1, 2, 1, 2, 3, 0xffffffff]))
    regions[ITEMS] = items
    obj = bytearray(r.bytes(0x40))
    n = struct.unpack_from('<i', regions[0x7890a0], 0x528)[0]
    ident = struct.unpack_from('<H', items, ITEM * r.randrange(n))[0]
    put(obj, 0xe, 'H', ident)
    col, row = r.choice([0, 1, 5, 127, 128, 200, 255, r.randrange(256)]), r.randrange(4)
    obj[5], obj[7] = col, row
    angle = r.choice([0, 0x5a, 0xb4, 0x10e, 0x5a, 0, 1, 0x5b, -1])
    put(obj, 0x10, 'i', angle)
    regions[OBJ] = obj
    regions[0x7cf83c] = struct.pack('<I', WORLD)
    regions[0x81b4cc] = struct.pack('<I', r.choice([0, 1]))
    x, y = col & 0x7f, row + (col >> 7)
    offsets = {0: ((4, 1), (-2, 1)), 0x5a: ((1, -5), (1, 1)), 0xb4: ((-5, -2), (1, -2)), 0x10e: ((-2, 4), (-2, -2))}.get(angle, ((0, 0), (0, 0)))
    for dx, dy in offsets:
        ident_ = (((y + dy) << 7) + x + dx + 1) & 0xffff
        cell = bytearray(r.bytes(0x28))
        cell[0xc] = r.choice([1, 2, 4, 0x10, 0x80, 3, 0, 0xff, 0x55])
        cell[0x22] = r.choice([1, 2, 4, 0x10, 0x80, 3, 0, 0xff, 0x55])
        cell[2] = r.choice([2, 2, 0, 1])
        regions[WORLD + (ident_ + 0x6cde) * 0x28] = cell
    return Call(0x52c2b0, [OBJ], regions, conv='c', ret='b')


def case(r):
    return (capacity if r.choice(OPS) == 'Capacity' else track)(r)


OPS = [k for k in os.environ.get('OPS', 'Capacity,Track').split(',') if k]

if __name__ == '__main__':
    B.main(case)

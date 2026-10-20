# The object functions of src/game/world/object that read the object definitions and cells (0x4dd7f0 Object_SetCapacity, 0x52c2b0 Object_TrackNeighboursOk) against the originals:
#   Capacity: ridelist.py's definitions and ride records (the limits around the capacity, a sum of limits above or not);
#   Track: an object of track type 0, 1, 2 or others at an angle of 0, 0x5a, 0xb4, 0x10e or another, the cells it looks at laid out one by one with one neighbour bit or more.
#   Score, Percent (0x4df450 Object_DefRowScore, 0x4df640 Object_DefRowPercent): definition rows and limits about the inputs, the ride's load in and outside them;
#   Stand (0x4dedf0 Object_CellStandPoint): entrance or exit points turned by the angle; History (0x4e1f20 Object_HistoryPercent): the two rings' sums.
# usage: object_checks.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Capacity,Track,Score,Percent,Stand,History)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import ridelist  # noqa: E402
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, park  # noqa: E402

B = Bench([(0x412e90, 0x412f00), (0x4dd4e0, 0x4dd510), (0x4d8c00, 0x4d8c30), (0x4dd7f0, 0x4dd930), (0x4df450, 0x4df66d), (0x522770, 0x5227f0), (0x52c2b0, 0x52c430),
           (0x55a070, 0x55a110), (0x55a0b0, 0x55a110), (0x55a390, 0x55a3f0), (0x5da3c0, 0x5da3c1),
           (0x495cf0, 0x495d50), (0x4dedf0, 0x4df160), (0x4e1f20, 0x4e2190)])
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


def score(r):
    """0x4df450: definitions with small limits (+0x128, +0x138: 0 too) and rows (+0x19c, +0x1ac, +0x1b0) around the inputs, the row byte 0-3, the ride record's
    element 5 (the load) below 0, inside or above the limit (or no record, no element 5, the list system off); a, b and the flag byte."""
    regions, _h, _a = park(r, count=0, defs=True)
    items = bytearray(regions[ITEMS])
    for i in range(NITEMS):
        put(items, i * ITEM + 0x128, 'i', r.choice([0, 1, 4, 10, 100, 1000, -3]))
        put(items, i * ITEM + 0x138, 'i', r.choice([0, 1, 5, 10, 100, 7, -2]))
        for row in range(4):
            for off in (0x19c, 0x1ac, 0x1b0):
                put(items, i * ITEM + 0x40 * row + off, 'i', r.choice([0, 1, 2, 5, 10, 50, 100, -1, 0x7fffff, 0x7fffffff]))
    regions[ITEMS] = items
    obj = bytearray(r.bytes(0x60))
    n = struct.unpack_from('<i', regions[0x7890a0], 0x528)[0]
    put(obj, 0xe, 'H', struct.unpack_from('<H', items, ITEM * r.randrange(n))[0])
    obj[0x50] = r.choice([0, 1, 2, 3])
    nodes, arrays = RAM + 0x1000, RAM + 0x2000
    node = bytearray(r.bytes(0x100))
    put(node, 0, 'I', 0)
    put(node, 8, 'I', 7)
    put(node, 0x1c, 'I', arrays)
    put(node, 0x8c, 'i', r.choice([6, 6, 6, 8, 5]))
    elements = bytearray(r.bytes(0x20))
    put(elements, 0x14, 'i', r.choice([-5, -1, 0, 1, 3, 10, 50, 1000, 0x7fffffff]))
    put(obj, 0x24, 'I', 7 if r.random() < 0.9 else 8)
    regions.update({OBJ: obj, nodes: node, arrays: elements, 0x8791a0: struct.pack('<I', 1 if r.random() < 0.95 else 0), 0x8791b0: struct.pack('<I', nodes)})
    scale = r.choice([1.0, 10.0, 100.0, 1000.0])
    whole = [fb(float(v)) for v in (0, 1, 2, 5, 10, 50, 100)]  # a on a row limit (+0x1ac) exactly
    args = [OBJ, r.choice(whole) if r.random() < 0.3 else r.f(scale), r.f(scale), r.getrandbits(32), r.choice([0, 1, 0x100, 0xff])]
    return Call(0x4df450, args, regions, conv='t', ret='d')


def percent(r):
    c = score(r)
    c.address, c.args, c.ret = 0x4df640, c.args[:4], 'b'
    return c


def stand(r):
    """0x4dedf0: the entrance or exit cell id (0 too), an angle of a quarter turn or not, the definition's points in (0, 1) or anywhere."""
    regions, _h, _a = park(r, count=0, defs=True)
    items = bytearray(regions[ITEMS])
    for i in range(NITEMS):
        for off in (0xd4, 0xd8, 0xdc, 0xe0):
            put(items, i * ITEM + off, 'I', fb(r.random()) if r.random() < 0.7 else r.f(4.0))
    regions[ITEMS] = items
    obj = bytearray(r.bytes(0x60))
    n = struct.unpack_from('<i', regions[0x7890a0], 0x528)[0]
    put(obj, 0xe, 'H', struct.unpack_from('<H', items, ITEM * r.randrange(n))[0])
    put(obj, 0x10, 'i', r.choice([0, 0x5a, 0xb4, 0x10e, 0, 0x5a, 0xb4, 0x10e, 1, -0x5a]))
    for off in (0x36, 0x38):
        put(obj, off, 'H', r.choice([0, 1, 0x80, 0x81, 0x3fff, 0x4000, r.getrandbits(16)]))
    regions.update({OBJ: obj, RAM + 0x1000: r.bytes(8)})
    return Call(0x4dedf0, [OBJ, r.choice([0, 1, 0x10000, 0x10001]), RAM + 0x1000, RAM + 0x1004], regions, conv='t', ret='v')


def history(r):
    """0x4e1f20: the rings at +0x1a8 and +0x2b8: cursor, size and wrapped around the 30 entries, values small (or all 0) or large."""
    obj = bytearray(r.bytes(0x400))  # (a size of 31 reads up to +0x3ac)
    for base in (0x1a8, 0x2b8):
        size = r.choice([0, 1, 5, 29, 30, 30, 31])
        put(obj, base + 0x7c, 'i', r.choice([-1, 0, 1, 4, 28, 29, size - 1]))
        put(obj, base + 0x80, 'i', size)
        obj[base + 0x84] = r.choice([0, 1, 1, 0x80])
        kind = r.choice(['small', 'small', 'zero', 'large'])
        for i in range(30):
            put(obj, base + 4 + 4 * i, 'I', {'small': r.randrange(0, 50), 'zero': 0, 'large': r.getrandbits(32)}[kind])
    return Call(0x4e1f20, [OBJ], {OBJ: obj}, conv='t', ret='a')


CASES = {'Capacity': capacity, 'Track': track, 'Score': score, 'Percent': percent, 'Stand': stand, 'History': history}


def case(r):
    return CASES[r.deal(OPS)](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The object functions of src/game/world/object that read the object definitions and cells (0x4dd7f0 Object_SetCapacity, 0x52c2b0 Object_TrackNeighboursOk) against the originals:
#   Capacity: ridelist.py's definitions and ride records (the limits around the capacity, a sum of limits above or not);
#   Track: an object of track type 0, 1, 2 or others at an angle of 0, 0x5a, 0xb4, 0x10e or another, the cells it looks at laid out one by one with one neighbour bit or more.
#   Score, Percent (0x4df450 Object_DefRowScore, 0x4df640 Object_DefRowPercent): definition rows and limits about the inputs, the ride's load in and outside them (or huge, with rows about load * 0.1f);
#   Stand (0x4dedf0 Object_CellStandPoint): entrance or exit points turned by the angle (some n / 255 outside (0, 1)); History (0x4e1f20 Object_HistoryPercent): the two rings' sums.
#   Fill (0x4dda40 Object_FillRate): the flag, the count and the bytes 0 or not, definition rows of limits 0 or below and floats of either sign;
#   Pay (0x4e1920 Object_PayRunningCost): stafftrain.py's money record and table, definitions of +0x4ac 0-3, a slots object whose selected slot matches or not;
#   Settings, Current (0x4e1a30, 0x4e1b40): settings on and about the ends of the range (or any floats, NaN; bytes), definitions of kinds 0-4, the multiplier any;
#   Diff, Average (0x4e1c70, 0x4e1e30): History's rings at +0x70 and +0xf8, at +0x340 (signed values there);
#   Reset (0x4dfd80 Object_ResetCondition): Score's ride record, the float +0x44 about 25 and its byte's wrap, mapstep.py's spread records and cells;
#   WontPay (0x4fde50 Visitor_WontPay, person.cpp's): a visitor's stats and cash about an object's price, definitions of kinds 0-3, their effects and
#   costs small or any, the controls' figure (cached), the park analyser's rings;
#   Rating, RatingOf (0x4e0560 Object_ComputeRating, 0x4e0860 Object_Rating): definitions of kinds 0-3 and track types with bases and row limits,
#   the ride list's coaster index, the coasters' floats, a track ride's pieces; the arguments (the object's goods and settings) about the limits; Poor (0x4e0390 Object_InPoorCondition): Reset's fields.
#   Gap (0x4fd4e0 Visitor_RatingGap): RatingOf's object with Poor's fields, a visitor whose byte +0x1f0 picks its row of wanted ratings (0x7850e4).
#   QueueFull (0x4ddb60, src/game/world/objectmsg): Fill's object tuned to rates of 4-7 mostly, with objectmsg.py's queue of 0-6 visitors.
# usage: object_checks.py [CASES [SEED]] [--wild|--both] [--show N]
#   (env OPS=Capacity,Track,Score,Percent,Stand,History,Fill,Pay,Settings,Current,Diff,Average,Reset,WontPay,Rating,RatingOf,Poor,Gap,QueueFull)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import ridelist  # noqa: E402
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, POOL, THING, park  # noqa: E402

B = Bench([(0x412e90, 0x412f00), (0x4dd4e0, 0x4dd510), (0x4d8c00, 0x4d8c30), (0x4dd7f0, 0x4dd930), (0x4df450, 0x4df66d), (0x522770, 0x5227f0), (0x52c2b0, 0x52c430),
           (0x55a070, 0x55a110), (0x55a0b0, 0x55a110), (0x55a390, 0x55a3f0), (0x5da3c0, 0x5da3c1),
           (0x495cf0, 0x495d50), (0x4dedf0, 0x4df160), (0x4e1f20, 0x4e2190),
           (0x4dda40, 0x4ddb70), (0x4e1910, 0x4e1a00), (0x4e1a30, 0x4e1e10), (0x4e1e30, 0x4e1f30), (0x409cd0, 0x409cf0), (0x4d01f0, 0x4d0270),
           (0x4d27a0, 0x4d2850), (0x519510, 0x519560), (0x5195d0, 0x519660), (0x4dfd80, 0x4dfe30), (0x4d8440, 0x4d8480), (0x4d8480, 0x4d8620),
           (0x4fde50, 0x4fe1b0), (0x4fd4e0, 0x4fd560), (0x4c74b0, 0x4c7500), (0x4d3d10, 0x4d3d90), (0x4e1a00, 0x4e1a30), (0x4e21b0, 0x4e21d0), (0x4e2740, 0x4e2780),
           (0x4e0560, 0x4e0860), (0x4e0860, 0x4e0900), (0x43e0b0, 0x43e0f0), (0x545310, 0x545460), (0x55a4e0, 0x55a540), (0x4e0390, 0x4e03c4),
           (0x4ddb60, 0x4ddb82), (0x4ddf50, 0x4de040), (0x502430, 0x502460), (0x50b350, 0x50b360)])
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
    if r.random() < 0.15:  # a huge load under a huge limit, the rows' +0x19c about load * 0.1f: loads where 0.1f's error moves the 24-bit
        # rounded product past an integer (load * 0.1 would round to 1e8, 12345679 or 2e8), the limit between the two roundings
        load, tenth = r.choice([(1000000026, 100000004), (123456794, 12345680), (2000000051, 200000008), (1000000000, 100000000)])
        put(elements, 0x14, 'i', load)
        for i in range(NITEMS):
            put(items, i * ITEM + 0x128, 'i', 0x7fffffff)
            for row in range(4):
                put(items, i * ITEM + 0x40 * row + 0x19c, 'i', r.choice([tenth, tenth, tenth - 1, tenth + 1, load // 10]))
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
            # (or n / 255 outside (0, 1): times 255 within half a 24-bit step of n, which the x87's rounding carries to n before __ftol)
            k = r.random()
            put(items, i * ITEM + off, 'I', fb(r.random()) if k < 0.6 else fb(r.randrange(-1020, 1021) / 255.0) if k < 0.75 else r.f(4.0))
    regions[ITEMS] = items
    obj = bytearray(r.bytes(0x60))
    n = struct.unpack_from('<i', regions[0x7890a0], 0x528)[0]
    put(obj, 0xe, 'H', struct.unpack_from('<H', items, ITEM * r.randrange(n))[0])
    put(obj, 0x10, 'i', r.choice([0, 0x5a, 0xb4, 0x10e, 0, 0x5a, 0xb4, 0x10e, 1, -0x5a]))
    for off in (0x36, 0x38):
        put(obj, off, 'H', r.choice([0, 1, 0x80, 0x81, 0x3fff, 0x4000, r.getrandbits(16)]))
    regions.update({OBJ: obj, RAM + 0x1000: r.bytes(8)})
    return Call(0x4dedf0, [OBJ, r.choice([0, 1, 0x10000, 0x10001]), RAM + 0x1000, RAM + 0x1004], regions, conv='t', ret='v')


def ring(r, obj, base):
    """a CircBufU32 of 30 at base: cursor, size and wrapped around the 30 entries, values small (or all 0) or large"""
    size = r.choice([0, 1, 5, 29, 30, 30, 31])
    put(obj, base + 0x7c, 'i', r.choice([-1, 0, 1, 4, 28, 29, size - 1]))
    put(obj, base + 0x80, 'i', size)
    obj[base + 0x84] = r.choice([0, 1, 1, 0x80])
    kind = r.choice(['small', 'small', 'zero', 'large'])
    for i in range(30):
        put(obj, base + 4 + 4 * i, 'I', {'small': r.randrange(0, 50), 'zero': 0, 'large': r.getrandbits(32)}[kind])


def history(r):
    """0x4e1f20: the rings at +0x1a8 and +0x2b8."""
    obj = bytearray(r.bytes(0x400))  # (a size of 31 reads up to +0x3ac)
    for base in (0x1a8, 0x2b8):
        ring(r, obj, base)
    return Call(0x4e1f20, [OBJ], {OBJ: obj}, conv='t', ret='a')


def defs_obj(r, fields, size=0x200):
    """park's definitions with fields {offset: choices} (dwords) in each, an object of size with the id (+0xe) of one"""
    regions, _h, _a = park(r, count=0, defs=True)
    items = bytearray(regions[ITEMS])
    for i in range(NITEMS):
        for off, choices in fields.items():
            put(items, i * ITEM + off, 'I', r.choice(choices) & 0xffffffff)
    regions[ITEMS] = items
    obj = bytearray(r.bytes(size))
    n = struct.unpack_from('<i', regions[0x7890a0], 0x528)[0]
    put(obj, 0xe, 'H', struct.unpack_from('<H', items, ITEM * r.randrange(n))[0])
    regions[OBJ] = obj
    return regions, obj


def fill(r):
    """0x4dda40: flag 8 of +0x32 or not, the count +0x58, the row byte, the bytes +0x5c / +0x5d (0 too); the rows' limits (0, below) and floats."""
    floats = [fb(v) for v in (0.0, 1.0, 0.5, 2.5, -1.0, 100.0)] + [r.f(10.0)]
    fields = {}
    for row in range(4):
        fields[0x40 * row + 0x1a8] = [0, 1, 2, 5, 10, 100, -1, -3]
        fields[0x40 * row + 0x1b4] = floats
    regions, obj = defs_obj(r, fields)
    obj[0x32] = r.choice([8, 8, 0xc, 0xff, 0, 0xf7])
    put(obj, 0x58, 'I', r.choice([0, 1, 5, 100, 0xffffffff, r.getrandbits(32)]))
    obj[0x50] = r.randrange(4)
    obj[0x5c] = r.choice([0, 1, 2, 10, 255])
    obj[0x5d] = r.choice([0, 1, 10, 100, 255])
    regions[0x7cdc54] = r.bytes(4)
    return Call(0x4dda40, [OBJ], regions, conv='t', ret='a')


def queue_full(r):
    """0x4ddb60: Fill's object (flag 8 mostly, the count 0 or the row's limit mostly, the bytes +0x5c / +0x5d equal mostly and the rows'
    floats 4-7 mostly: rates about the queue's length) with objectmsg.py's queue of 0-6 visitors (states 11 mostly) at +0x3c"""
    count = r.choice([1, 2, 3, 4, 6])
    regions, handles, _things = park(r, count=count, kinds=(1,), defs=True)
    items = bytearray(regions[ITEMS])
    floats = [fb(v) for v in (4.0, 5.0, 6.0, 7.0, 4.5)] * 3 + [fb(v) for v in (1.0, 100.0, -1.0)] + [r.f(10.0)]
    for i in range(NITEMS):
        for row in range(4):
            put(items, i * ITEM + 0x40 * row + 0x1a8, 'i', r.choice([1, 2, 5, 10, 0, -3]))
            put(items, i * ITEM + 0x40 * row + 0x1b4, 'I', r.choice(floats))
    regions[ITEMS] = items
    obj = bytearray(r.bytes(0x200))
    n = struct.unpack_from('<i', regions[0x7890a0], 0x528)[0]
    put(obj, 0xe, 'H', struct.unpack_from('<H', items, ITEM * r.randrange(n))[0])
    obj[0x32] = r.choice([8, 8, 8, 0xc, 0xff, 0, 0xf7])
    obj[0x50] = r.randrange(4)
    limit = struct.unpack_from('<i', items, 0x1a8)[0]   # (row 0 of the first: a match now and then)
    put(obj, 0x58, 'I', r.choice([0, 0, limit & 0xffffffff, 1, 5, r.getrandbits(32)]))
    obj[0x5c] = r.choice([1, 2, 10, 255])
    obj[0x5d] = obj[0x5c] if r.random() < 0.7 else r.choice([0, 1, 10, 255])
    regions[0x7cdc54] = r.bytes(4)
    pool = bytearray(regions[POOL])
    order = r.sample(range(count), r.randrange(0, count + 1))
    for k, i in enumerate(order):
        put(pool, THING * i + 0x228, 'HH', handles[order[k + 1]] if k + 1 < len(order) else 0, handles[order[k - 1]] if k else 0)
        put(pool, THING * i + 0x220, 'i', r.choice([11, 12, 14, 8, 3, 15]) if r.random() < 0.2 else 11)
    regions[POOL] = pool
    put(obj, 0x3c, 'H', handles[order[0]] if order else 0)
    regions[OBJ] = obj
    return Call(0x4ddb60, [OBJ], regions, conv='t', ret='a')


def pay(r):
    """0x4e1920: stafftrain.py's money record (the table entry of the world's +0x1da726) and a slots object (+0x1da736's), its selected slot of id 0xc / 0xd or
    not, kind 0 or not, the limit word 0 or not; definitions of +0x4ac 0-3; amounts of either sign."""
    regions, obj = defs_obj(r, {0x4ac: [0, 1, 2, 3, 1, 2]})
    money, slots_obj, world, park_ = RAM + 0x3000, RAM + 0x4000, RAM + 0x10000, RAM + 0x40000
    rec = bytearray(r.bytes(0x200))
    put(rec, 0x114, 'I', r.choice([0, 1, 1]))
    put(rec, 0x11c, 'i', r.choice([-1, 0, 5]))
    so = bytearray(r.bytes(0x400))
    index = r.choice([0, 1, 5, 19, 20, -1])
    put(so, 0x3cc, 'i', index)
    so[0x3d9] = r.choice([0, 1, 1, 7])
    if 0 <= index < 20:
        put(so, 0xc + 48 * index, 'I', r.choice([0xc, 0xd, 0xc, 0xd, 5]))
        put(so, 0xc + 48 * index + 0xc, 'H', r.choice([0, 0, 1]))
    put(so, 0x3e0, 'H', r.choice([0, 0, 5]))
    table = bytearray(r.bytes(20 * 4))
    put(table, 20 * 1, 'I', money)
    put(table, 20 * 2, 'I', park_)
    put(table, 20 * 3, 'I', slots_obj)
    game = bytearray(r.bytes(0x40))
    put(game, 0xc, 'I', r.getrandbits(32))   # the time, +0x1da70c
    put(game, 0x20, 'H', 2)                  # the park, +0x1da720 (the debit's)
    put(game, 0x26, 'H', 1)
    put(game, 0x36, 'H', 3)
    regions.update({money: rec, slots_obj: so, 0x7cfb90: table, world + 0x1da700: game, 0x80239c: struct.pack('<I', world), 0x7cf83c: struct.pack('<I', world),
                    park_ + 0x1f5a0: r.bytes(4), park_ + 0x1fa40: r.bytes(4), park_ + 0x1fc90: r.bytes(4)})
    amount = r.choice([0, 1, 50, 1000, -1, -1000, 0x7fffffff, -0x80000000, r.randrange(-1000, 200000)])
    return Call(0x4e1920, [OBJ, amount & 0xffffffff], regions, conv='t', ret='v')


def settings_obj(r):
    regions, obj = defs_obj(r, {0x158: [0, 1, 2, 3, 4, 1, 3]})
    put(obj, 0x188, 'I', r.choice([0, 1, 100, 1000, 0xffffffff, r.getrandbits(32)]))
    return regions, obj


def setting(r):
    if r.random() < 0.6:
        return fb(r.choice([0.0, 50.0, 100.0, 25.0, 75.0, -50.0, 150.0, -49.0, 151.0, 49.5, 1.0, 99.0, float('nan')]))
    return r.f(r.choice([100.0, 1000.0, 1e30]))


def settings(r):
    """0x4e1a30: settings on and about 0-100 and their limits (-50, 150), or any floats; History's kinds and multipliers."""
    regions, _obj = settings_obj(r)
    return Call(0x4e1a30, [OBJ, setting(r), setting(r)], regions, conv='t', ret='a')


def current(r):
    regions, obj = settings_obj(r)
    obj[0x18c] = r.choice([0, 50, 100, 255, r.getrandbits(8)])
    obj[0x198] = r.choice([0, 50, 100, 255, r.getrandbits(8)])
    return Call(0x4e1b40, [OBJ], regions, conv='t', ret='a')


def diff(r):
    obj = bytearray(r.bytes(0x400))
    for base in (0x70, 0xf8):
        ring(r, obj, base)
    return Call(0x4e1c70, [OBJ], {OBJ: obj}, conv='t', ret='a')


def average(r):
    obj = bytearray(r.bytes(0x400))
    ring(r, obj, 0x340)
    return Call(0x4e1e30, [OBJ], {OBJ: obj}, conv='t', ret='a')


def reset(r):
    """0x4dfd80: Score's ride record (element 8 there or not), flag 1 of +0x32, the float +0x44 about 25 (its __ftol's low byte: 256 and above wrap), the object's
    cell anywhere on the map with mapstep.py's spread records (6 and 1) and cells."""
    c = score(r)
    obj = bytearray(c.regions[OBJ]) + bytearray(r.bytes(0x20))
    obj[0x32] = r.choice([1, 1, 0xff, 0, 0xfe])
    put(obj, 0x44, 'I', fb(r.choice([0.0, 10.0, 24.0, 24.9, 25.0, 25.5, 26.0, 100.0, 255.0, 256.0, 280.0, -1.0])) if r.random() < 0.7 else r.f(300.0))
    obj[5], obj[7] = r.randrange(128), r.randrange(128)
    c.regions[OBJ] = obj
    node = bytearray(c.regions[RAM + 0x1000])
    put(node, 0x8c, 'i', r.choice([9, 9, 10, 8, 6]))
    c.regions[RAM + 0x1000] = node
    c.regions[RAM + 0x2000] = bytearray(c.regions[RAM + 0x2000]) + bytearray(r.bytes(0x20))
    world = RAM + 0x100000
    records = bytearray(r.bytes(12 * 8))
    for k in range(8):
        records[12 * k + 0xa] = r.choice([0, 1, 2, 3])
        struct.pack_into('<5h', records, 12 * k, *[r.choice([0, 1, -1, 100, -100, 0x7fff, -0x8000, r.randrange(-300, 300)]) for _ in range(5)])
    c.regions.update({world + 0x1d9104: records, world + 0x1b1104: r.bytes(10 * 0x4000), 0x8023a0: struct.pack('<I', world)})
    c.address, c.args, c.ret = 0x4dfd80, [OBJ], 'v'
    return c


def wont_pay(r):
    """0x4fde50: a visitor's stats about whole bytes (or any) and cash about the price; definitions of kinds 0-3, ingredients 0-5, appearances 0-3, effects
    and costs small or any; the object's price (0 too), byte +0x190 and +0x188; the controls' (cached) +0xc; the park analyser's rings sized 1, 2 or 0x30."""
    effects = [0, 1, 5, 20, 100, -10, 0x7fffffff, -0x80000000]
    regions, obj = defs_obj(r, {0x4ac: [0, 1, 2, 3, 1, 2], 0x158: [0, 1, 2, 3, 4, 5, 1], 0x15c: [0, 1, 2, 3, 1, 2], 0x140: [0, 1, 10, 50, 200, -5, 0x40000000],
                                0x144: effects, 0x148: effects, 0x14c: effects, 0x150: effects})
    price = r.choice([0, 1, 5, 20, 50, 100, 1000, 0xffffffff, r.getrandbits(32)])
    put(obj, 0x194, 'I', price)
    obj[0x190] = r.choice([0, 1, 50, 100, 255])
    put(obj, 0x188, 'I', r.choice([0, 1, 10, 100, 1000, 0xffffffff, r.getrandbits(32)]))
    visitor = bytearray(r.bytes(0x1c0))
    for off in (0x19c, 0x1a4, 0x1a8, 0x1b0):
        put(visitor, off, 'I', fb(r.choice([0.0, 1.0, 50.5, 100.0, 255.0, 255.9, 256.0, -1.0])) if r.random() < 0.7 else r.f(1000.0))
    put(visitor, 0x1a0, 'I', (price + r.choice([0, 0, 1, -1, 100, -100])) & 0xffffffff if r.random() < 0.7 else r.getrandbits(32))
    world, analyser, record = WORLD, RAM + 0x80000, RAM + 0x7000
    tail = bytearray(r.bytes(0x50))              # the world from the controls' count (+0x1d943c + 0x12c0 = +0x1da6fc)
    put(tail, 0, 'i', 0)                         # the count: the cached id is the one found
    put(tail, 4, 'H', struct.unpack_from('<H', obj, 0xe)[0])
    put(tail, 8, 'I', record)
    put(tail, 0x24, 'H', 2)                      # +0x1da720: the analyser's table entry
    rec = bytearray(r.bytes(0x20))
    put(rec, 0xc, 'I', r.choice([0, 10, 50, -50, -100, 1000, r.getrandbits(32)]) & 0xffffffff)
    rings = bytearray(r.bytes(0x280))
    for k in range(10):
        size = r.choice([1, 2, 0x30])
        put(rings, k * 0x40 + 0x38, 'i', size)
        put(rings, k * 0x40 + 0x34, 'i', r.randrange(size))
    table = bytearray(r.bytes(20 * 4))
    put(table, 20 * 2, 'I', analyser)
    regions.update({RAM + 0x6000: visitor, world + 0x1da6fc: tail, record: rec, analyser + 0x216fc: rings, 0x7cfb90: table,
                    0x80239c: struct.pack('<I', world), 0x7cf83c: struct.pack('<I', world)})
    return Call(0x4fde50, [RAM + 0x6000, OBJ], regions, conv='t', ret='a')


def rating_env(r):
    """definitions of kinds 0-3 (+0x4ac) and track types (+0x9c, 3 a coaster) with a base (+0x13c) and row limits (+0x1a0, +0x1a8) small or any; an object
    of row 0-3 whose ride (+0x24) is in the ride list (or not) with a coaster index 0-3 (+0xe0), coasters' +0x104 floats, a track ride (+0x28, its handle or 0)
    of 0-5 pieces"""
    limits = [0, 1, 2, 5, 10, 50, 100, -1, -7]
    fields = {0x4ac: [0, 1, 2, 2, 3], 0x9c: [0, 1, 3, 3, 4], 0x13c: [0, 1, 5, 20, 50, 99, 100, 200, -10, 0x7fffffff]}
    for row in range(4):
        fields[0x40 * row + 0x1a0] = limits
        fields[0x40 * row + 0x1a8] = limits
    regions, obj = defs_obj(r, fields)
    obj[0x50] = r.randrange(4)
    # the ride list (0x8791a0 the system, 0x8791b0 the head): nodes +0 next, +8 id, +0xe0 the coaster index
    ids = [r.getrandbits(32) for _ in range(3)]
    nodes = bytearray(r.bytes(0x100 * 3))
    for k, ident in enumerate(ids):
        put(nodes, 0x100 * k, 'I', RAM + 0x20000 + 0x100 * (k + 1) if k < 2 else 0)
        put(nodes, 0x100 * k + 8, 'I', ident)
        put(nodes, 0x100 * k + 0xe0, 'I', r.choice([0, 1, 2, 3, 3]))
    put(obj, 0x24, 'I', r.choice(ids + ids + [r.getrandbits(32)]))
    regions.update({RAM + 0x20000: nodes, 0x8791a0: struct.pack('<I', r.choice([1, 1, 1, 0])), 0x8791b0: struct.pack('<I', RAM + 0x20000)})
    # the coasters (0x790bd0's records): the float +0x104 of each (and +0x100)
    coasters, table = bytearray(r.bytes(0x800)), bytearray(16)
    for k in range(4):
        put(table, 4 * k, 'I', RAM + 0x22000 + 0x200 * k)
        put(coasters, 0x200 * k + 0x104, 'I', fb(r.choice([0.0, 1.0, -1.0, 20.0, 99.5, 101.0, -100.0, 512.0])) if r.random() < 0.8 else r.getrandbits(32))
    regions.update({RAM + 0x22000: coasters, 0x790bd0: table})
    # the track rides (trackrides.py's Count): the module's magic at 0x877b58, the pool at 0x877b60, a ride's chain of pieces at +0xbc
    rides = bytearray(r.bytes(0x10 * 0xd0))
    index, kind = r.randrange(0x10), r.choice([0, 1, 5])
    put(rides, index * 0xd0, 'i', kind)
    n = r.choice([0, 1, 2, 3, 5, 26, 30])   # (long chains of kind 11 reach the figure's cap of 40)
    pieces = bytearray(r.bytes(0x40 * 30))
    kinds = [8, 9, 10, 11, 11, 12, 13, 13, 14, 0] if n < 26 else [11, 11, 11, 11, 9, 0]
    for k in range(n):
        put(pieces, 0x40 * k + 4, 'H', r.choice(kinds))
        put(pieces, 0x40 * k + 8, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        put(pieces, 0x40 * k + 0x24, 'I', RAM + 0x23000 + 0x40 * (k + 1) if k + 1 < n else 0)
    put(rides, index * 0xd0 + 0xbc, 'I', RAM + 0x23000 if n else 0)
    globals_ = bytearray(r.bytes(0x40))
    put(globals_, 0, 'I', 0x4a454647)
    put(globals_, 8, 'I', RAM + 0x24000)
    put(obj, 0x28, 'I', r.choice([index | kind << 8, index | kind << 8, 0, index | (kind ^ 1) << 8]))
    regions.update({RAM + 0x23000: pieces, RAM + 0x24000: rides, 0x877b58: globals_})
    return regions, obj


def rating(r):
    """0x4e0560 (Object_ComputeRating): rating_env's object and definitions; the arguments about each other and the limits (a sideshow's cost over its price,
    a ride's speed and duration), the third any"""
    regions, obj = rating_env(r)
    small = [0, 1, 2, 5, 10, 50, 99, 100, 101, 150, 200, -1, -50]
    a, b = r.choice(small), r.choice(small)
    if r.random() < 0.2:
        a, b = r.getrandbits(32), r.getrandbits(32)
    c = r.choice([0, 1, 10, 50, 100, 255, -1, 0x7fffffff, r.getrandbits(32)])
    return Call(0x4e0560, [OBJ, a & 0xffffffff, b & 0xffffffff, c & 0xffffffff], regions, conv='t', ret='b')


def rating_of(r):
    """0x4e0860 (Object_Rating): rating_env's, the flag byte 0 or not (high bits any); the object's goods (+0x188, +0x194, +0x190) and settings (+0x58,
    +0x5c, +0x5d) about each other"""
    regions, obj = rating_env(r)
    for off in (0x188, 0x194, 0x58):
        put(obj, off, 'I', r.choice([0, 1, 5, 10, 50, 100, 200, 0xffffffff, r.getrandbits(32)]))
    for off in (0x190, 0x5c, 0x5d):
        obj[off] = r.choice([0, 1, 5, 10, 50, 100, 255])
    flag = r.choice([0, 0, 0, 1, 0x100, 0xff]) | (r.getrandbits(24) << 8 if r.random() < 0.3 else 0)
    return Call(0x4e0860, [OBJ, flag & 0xffffffff], regions, conv='t', ret='b')


def gap(r):
    """0x4fd4e0: rating_of()'s object with poor()'s flag byte and condition, a visitor's row (+0x1f0) 0-9 or any, the rows' first bytes any"""
    call = rating_of(r)
    obj = call.regions[OBJ]
    obj[0x32] = r.choice([0, 0, 0, 1, 0xfe])
    put(obj, 0x44, 'I', fb(r.choice([10.0, 24.0, 25.0, 100.0, 255.0, 256.0])))
    visitor = bytearray(r.bytes(0x200))
    visitor[0x1f0] = r.choice([0, 1, 2, 3, 9, r.getrandbits(8)])
    rows = bytearray(r.bytes(12 * 256))
    for k in range(256):
        rows[12 * k] = r.choice([0, 1, 5, 10, 50, 60, 100, 200, 255, r.getrandbits(8)])
    call.regions.update({RAM + 0x6000: visitor, 0x7850e4: rows})
    return Call(0x4fd4e0, [RAM + 0x6000, OBJ], call.regions, conv='t', ret='a')


def poor(r):
    """0x4e0390: reset()'s flag byte and condition float about 25 (its __ftol's low byte wraps from 256)"""
    obj = bytearray(r.bytes(0x60))
    obj[0x32] = r.choice([1, 1, 0xff, 0, 0xfe])
    put(obj, 0x44, 'I', fb(r.choice([0.0, 10.0, 24.0, 24.9, 25.0, 25.5, 26.0, 100.0, 255.0, 256.0, 280.0, -1.0, -230.0, 1e12])) if r.random() < 0.7
        else r.f(300.0))
    return Call(0x4e0390, [OBJ], {OBJ: obj}, conv='t', ret='a')


CASES = {'Capacity': capacity, 'Track': track, 'Score': score, 'Percent': percent, 'Stand': stand, 'History': history, 'Fill': fill, 'Pay': pay,
         'Settings': settings, 'Current': current, 'Diff': diff, 'Average': average, 'Reset': reset,
         'WontPay': wont_pay, 'Rating': rating, 'RatingOf': rating_of, 'Poor': poor, 'Gap': gap, 'QueueFull': queue_full}


def case(r):
    return CASES[r.deal(OPS)](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

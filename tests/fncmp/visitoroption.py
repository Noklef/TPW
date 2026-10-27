# A visitor's score of an option (0x4fcc30 Visitor_ScoreOption, src/game/world/visitoroption) against the original: object_checks.py's
# rating_env (definitions, the object's ride and goods for Object_Rating) with the definitions' stat bytes +0x144 / +0x148 and +0x114,
# +0xc4, +0x1b8 small or any; the object's handle, queue back (near the visitor's map id or anywhere in the first 16 rows; its RE cell's
# word +8 0, small or any) and a queue of 0-6 persons (objectmsg.py's states) in the thing slots, its room +0x40 and flag +0x32; the
# visitor's map id, stats (whole, about the table steps, or any floats), wanted rating row +0x1f0 and last rides +0x1e0-+0x1ee (the
# object's handle, another ride of its id or not, others); the age (funnytime.py's clock: days about the threshold 0x785090) and the
# world's record +0x30 (0x519590's); the weights and multipliers 0x785090-0x7850c8 and the wanted ratings 0x7850e4 small or any (the
# game's are 0 until its settings load).
# The visitor's feelings after a ride (0x4fdcc0, person): feelings_case().
# usage: visitoroption.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=score,feelings)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import funnytime  # noqa: E402
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402
from object_checks import OBJ, rating_env  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, SLOTS  # noqa: E402

B = Bench([(0x412e90, 0x412f00), (0x43e0b0, 0x43e0f0), (0x4d8300, 0x4d8340), (0x4d8410, 0x4d8450), (0x4d96f0, 0x4d97f0), (0x4d99c0, 0x4d99f0),
           (0x4dd4e0, 0x4dd510), (0x4dd670, 0x4dd6e0), (0x4ddf40, 0x4de200), (0x4de670, 0x4de7f0), (0x4e0560, 0x4e0910), (0x4f8690, 0x4f8740),
           (0x4fcc30, 0x4fd4f0), (0x502430, 0x502470), (0x50b350, 0x50b370), (0x519590, 0x5195e0), (0x522770, 0x5227a0), (0x522850, 0x522870),
           (0x536320, 0x536360), (0x545310, 0x545460), (0x55a4e0, 0x55a540), (0x5fc530, 0x5fc580), (0x5fc660, 0x5fc750), (0x67c320, 0x67c3e0),
           (0x67c7f0, 0x67c840), (0x5da3c0, 0x5da3c1), (0x4fdcc0, 0x4fde45)])
VISITOR, RIDE, RECORD, POOL, CLOCK = RAM + 0x6000, RAM + 0x6400, RAM + 0x6800, RAM + 0x2000, RAM + 0x28000
CELLS = RAM + 0x30000   # (the RE cells of map ids 1-0x800: [0x8023a0] + 0x1b1104)
DAY = funnytime.DAY


def stat(r):
    if r.random() < 0.3:
        return r.getrandbits(32) if r.random() < 0.2 else fb(r.uniform(-20, 300))
    return fb(r.choice([0.0, 1.0, 9.0, 9.99, 10.0, 50.0, 55.5, 100.0, 109.0, 110.0, 255.0, 255.9, 256.0, 266.0, -1.0, -0.5]))


def score_case(r):
    regions, obj = rating_env(r)
    for off in (0x188, 0x194, 0x58):
        put(obj, off, 'I', r.choice([0, 1, 5, 10, 50, 100, 200, 0xffffffff, r.getrandbits(32)]))
    for off in (0x190, 0x5c, 0x5d):
        obj[off] = r.choice([0, 1, 5, 10, 50, 100, 255])
    items = bytearray(regions[ITEMS])
    for i in range(NITEMS):
        for off in (0x144, 0x148):
            put(items, i * ITEM + off, 'I', r.choice([0, 9, 10, 50, 99, 100, 109, 110, 255]) | r.choice([0, r.getrandbits(24)]) << 8)
        put(items, i * ITEM + 0x114, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        put(items, i * ITEM + 0xc4, 'i', r.choice([0, 0, 0, 1, 2, 9, 100, -1, 0x7fffffff, -0x80000000]))
        put(items, i * ITEM + 0x1b8, 'i', r.choice([0, 1, 2999, 3000, 3001, 3004, 6000, 30000, -5, 0x7fffffff, r.getrandbits(32) - (1 << 31)]))
    regions[ITEMS] = items
    ident = struct.unpack_from('<H', obj, 0xe)[0]

    # the thing slots: the object, another ride, the world's record and the persons queued (0-6)
    handles = r.sample(range(1, 12), 9)
    h_obj, h_ride, h_rec, persons = handles[0], handles[1], handles[2], handles[3:3 + r.choice([0, 0, 1, 2, 3, 6])]
    slots = bytearray(r.bytes(20 * 12))
    put(slots, 20 * h_obj, 'I', OBJ)
    put(slots, 20 * h_ride, 'I', RIDE)
    put(slots, 20 * h_rec, 'I', RECORD)
    regions[SLOTS] = slots
    regions[RIDE] = put(bytearray(r.bytes(0x40)), 0xe, 'H', r.choice([ident, ident ^ r.randrange(1, 0x10000)]))
    regions[RECORD] = put(bytearray(r.bytes(0x40)), 0x30, 'i', r.choice([0, 1, 5, -1, r.getrandbits(32) - (1 << 31)]))
    pool = bytearray(r.bytes(0x400 * 6))
    for k, h in enumerate(persons):
        put(slots, 20 * h, 'I', POOL + 0x400 * k)
        put(pool, 0x400 * k + 0x228, 'H', persons[k + 1] if k + 1 < len(persons) else 0)
        put(pool, 0x400 * k + 0x220, 'i', r.choice([11, 12, 13, 14, 8, 3]) if r.random() < 0.9 else 3)
        put(pool, 0x400 * k + 0x224, 'i', r.choice([11, 13, 5, 14]))
    regions[POOL] = pool
    put(obj, 0, 'H', h_obj)
    put(obj, 0x3c, 'H', persons[0] if persons else 0)
    put(obj, 0x40, 'I', r.choice([0, 1, 2, 3, 5, 100, 0xffffffff]))
    obj[0x32] = r.choice([0, 1, 1, 0xff, 0xfe])

    # the visitor's map id and the object's queue back (map ids 1-0x800), its RE cell
    visitor = bytearray(r.bytes(0x200))
    x, y = r.choice([0, 1, 5, 126, 127, 128, 200, 255, r.randrange(256)]), r.randrange(15)
    visitor[5], visitor[7] = x, y
    vid = (y << 7) + x + 1
    if r.random() < 0.6:
        back = vid - 1 + r.randrange(-3, 4) + 128 * r.randrange(-3, 4) + 1
    else:
        back = r.randrange(1, 0x801)
    back = min(max(back, 1), 0x800)
    put(obj, 0x3a, 'H', back)
    cells = bytearray(r.bytes(0x800 * 10))
    put(cells, (back - 1) * 10 + 8, 'h', r.choice([0, 0, 0, 1, 2, 3, -1, -2, 7, r.randrange(-0x8000, 0x8000)]))
    regions[CELLS] = cells
    regions[0x8023a0] = struct.pack('<I', CELLS - 0x1b1104)

    for off in (0x1a4, 0x1a8, 0x1ac, 0x1b0):
        put(visitor, off, 'I', stat(r))
    visitor[0x1f0] = r.choice([0, 1, 2, 3, 4, 5, 6, 7, r.getrandbits(8)])
    put(visitor, 0x1e0, 'H', r.choice([0, 0, 0, h_obj, h_ride, h_ride, h_rec] + persons))
    for off in range(0x1e2, 0x1f0, 2):
        put(visitor, off, 'H', r.choice([h_obj, h_obj, h_ride, 0, r.getrandbits(16)]))
    regions[VISITOR] = visitor

    # the clock (funnytime.py's: the world at CLOCK - 0x2a0, its ticks at +0x1da70c, the record's handle +0x1da724) and the object's date
    funnytime.clock_regions(r, regions)
    clock = bytearray(r.bytes(0x40))
    put(clock, 0x1c, 'I', funnytime.u32(r))
    world = CLOCK - 0x2a0
    regions[CLOCK] = clock
    regions[0x80239c] = struct.pack('<I', world)
    ticks = bytearray(regions.pop(funnytime.TICKS)) + bytearray(r.bytes(0x10))
    put(ticks, 0x18, 'H', h_rec)
    regions[world + 0x1da70c] = ticks
    rate, tick = struct.unpack_from('<I', clock, 0x1c)[0], struct.unpack_from('<I', ticks)[0]
    product = rate * tick
    seconds = product // 4 if product < 1 << 63 else -((2 ** 64 - product) // 4)
    now = struct.unpack_from('<Q', clock)[0] + seconds * 10000000
    threshold = r.choice([0, 1, 7, 30, 100, 0xffffffff])
    days = r.choice([threshold, threshold + 1, threshold - 1, 0, r.randrange(200)]) & 0xffffffff
    age = days * DAY + r.choice([0, 1, DAY - 1, r.randrange(DAY)]) if r.random() < 0.9 else r.getrandbits(64)
    put(obj, 0x18, 'Q', (now - age) % 2 ** 64)
    # (the game's are 0 until its settings load; all seven weights 0 now and then: a fault in both)
    regions[0x785090] = struct.pack('<III', threshold, r.choice([0, 1, 2, 3, 0xffffffff]), r.choice([0, 1, 2, 5, 0x10000]))
    small = [0, 1, 1, 2, 5, 10, 20, 100, 0xffff, 0x12345, r.getrandbits(32)]
    regions[0x7850b0] = struct.pack('<7I', *(r.choice(small) for _ in range(7)))
    rows = bytearray(r.bytes(12 * 256))   # (the wanted ratings: 12-byte rows by the visitor's +0x1f0)
    for k in range(256):
        rows[12 * k] = r.choice([0, 1, 20, 50, 51, 100, 150, 255, r.getrandbits(8)])
    regions[0x7850e4] = rows
    return Call(0x4fcc30, [VISITOR, OBJ], regions, conv='t', ret='a')


def feelings_case(r):
    """Visitor_RideFeelings (0x4fdcc0, person): rating_env's object; the visitor's +0x19c, +0x1a8, +0x1b0 stats as above, its row +0x1f0
    small or any, the rows' wanted excitement 0-255 (about the ratings now and then); the steps 0x785064-0x78506c small, large or negative,
    the divisor 0x785070 small, negative or 0 now and then (a fault in both)."""
    regions, obj = rating_env(r)
    visitor = bytearray(r.bytes(0x200))
    for off in (0x19c, 0x1a8, 0x1b0):
        put(visitor, off, 'I', stat(r))
    visitor[0x1f0] = r.choice([0, 1, 2, 3, 4, 5, 6, 7, r.getrandbits(8)])
    regions[VISITOR] = visitor
    rows = bytearray(r.bytes(12 * 256))
    for k in range(256):
        rows[12 * k] = r.choice([0, 1, 4, 5, 6, 14, 15, 16, 20, 39, 40, 41, 50, 99, 100, 150, 255]) if r.random() < 0.5 else r.getrandbits(8)
    regions[0x7850e4] = rows
    steps = [0, 1, 5, 10, 50, 99, 100, 101, -1, -10, -100, 0x7fffffff, -0x80000000]
    divisor = r.choice([1, 1, 2, 3, 5, 10, 20, 100, 255, 256, -1, -3, 0x7fffffff] * 4 + [0])
    regions[0x785064] = struct.pack('<4i', r.choice(steps), r.choice(steps), r.choice(steps), divisor)
    return Call(0x4fdcc0, [VISITOR, OBJ], regions, conv='t', ret='v')


CASES = {'score': score_case, 'feelings': feelings_case}
OPS = [n for n in CASES if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    return CASES[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)

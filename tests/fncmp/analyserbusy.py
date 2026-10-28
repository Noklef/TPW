# The analyser cells' counts of busy map cells (src/game/world/analyser: 0x4c8c40 on one cell, 0x4c81b0 summed over the 0x400) against
# the originals: a cell's rectangle (+0 column, +4 row, +8 width, +0xc height: small, now and then empty, negative or reaching past the
# map's first rows (a fault in both)), its cache (+0x64 the ticks it was made at, +0x68 the count, +0x6c valid or not) fresh, stale or
# made in the future (the age unsigned); the world's ticks ([0x80239c] +0x1da70c) about 0 and 2^32; the map cells ([0x8023a0], 0x44
# bytes, 128 to a row) of types 0-0xb mostly (1, 3, 4, 9 and 0xa count). Sum: the analyser's 0x400 cells, most of them cached and fresh
# (a few to count again, their rectangles small). Booth (0x4c91c0): the two ticket booths' cells (the balance positions 0x78559c, in the
# map's first rows, now and then the same) each heading a chain (+0xa) of up to six things (the slots at 0x7cfb90) of kind 1 mostly,
# their +0x220 3 or not, the chains one list's tails; its value +0x1f0d0 set. Paths / SumPaths (0x4c8ae0, 0x4c8150: types 1, 3 and 9, the
# cache at +0x4c) and Held / SumHeld (0x4c8ba0, 0x4c8180: the map cells' +0x2c not 0, half of them 0; the cache at +0x58) as Count / Sum.
# Score (0x4c7db0): the analyser's value +0x1f0e8 fresh mostly; 3% of the time stale or not worked out, then the whole map (128 x 128 cells
# from the world [0x80239c] + 0x2d8: types of those counted, flags +0xe 0x40 or not, handles and parents 0-0x40 into the 0x28-byte entries
# of [0x8023a0] and [0x7cf83c]) and the world's chain of 0-6 things (+0x1da746, +0xc; their definitions 1-3 of 0x7890a0's, their +0x19c 4
# or not, +0x44 a float, +0x50 0-3 or 8). LoanOffer (0x4d06b0): see loan_offer(); AnyOffer (0x4d0810): its world, all eight offers asked.
# usage: analyserbusy.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Count,Sum,Booth,Paths,SumPaths,Held,SumHeld,Score,LoanOffer,AnyOffer)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

CALLS = {'Count': 0x4c8c40, 'Sum': 0x4c81b0, 'Booth': 0x4c91c0, 'Paths': 0x4c8ae0, 'SumPaths': 0x4c8150, 'Held': 0x4c8ba0, 'SumHeld': 0x4c8180, 'Score': 0x4c7db0,
         'LoanOffer': 0x4d06b0, 'AnyOffer': 0x4d0810}
CACHE = {'Count': 0x64, 'Sum': 0x64, 'Paths': 0x4c, 'SumPaths': 0x4c, 'Held': 0x58, 'SumHeld': 0x58}   # (the cell's cached value: ticks, count, flag)
SUMS = {'Sum': 'Count', 'SumPaths': 'Paths', 'SumHeld': 'Held'}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x4c8c40, 0x4c8d2c), (0x4c81b0, 0x4c81de), (0x41a960, 0x41a9a7), (0x536300, 0x536390), (0x5d60c0, 0x5d60c3), (0x4c91c0, 0x4c9283), (0x4d8610, 0x4d8750), (0x5da3c0, 0x5da3c1),
           (0x4c8ae0, 0x4c8b96), (0x4c8ba0, 0x4c8c3a), (0x4c8150, 0x4c817e), (0x4c8180, 0x4c81ae),
           (0x4c7db0, 0x4c7f98), (0x412e90, 0x412eea), (0x4d0af0, 0x4d0b20), (0x4e0380, 0x4e03a0),
           (0x535db0, 0x535de0), (0x536390, 0x5363b0), (0x5363f0, 0x536450), (0x53ad30, 0x53ad70), (0x53ad90, 0x53adc0),
           (0x4d06b0, 0x4d074a), (0x519510, 0x519550), (0x67a830, 0x67a860), (0x4d0810, 0x4d0850)])
CELL, ANALYSER, MAP, TICKS, THINGS = RAM, RAM + 0x1000, RAM + 0x40000, RAM + 0x200000, RAM + 0x100000
WORLD = TICKS - 0x1da70c   # (its map cells from +0x294, each one's first thing the word +0x24)
ROWS = 6
TYPES = [0, 0, 1, 1, 2, 3, 4, 5, 9, 0xa, 0xb, 0x109, 0xffffffff]


def rect(r, small=False):
    col = r.choice([0, 1, 5, 120, 126, r.randrange(128)])
    row = r.choice([0, 0, 1, 2, r.randrange(ROWS - 1)])
    w = r.choice([0, 1, 2, 3, 4] if small else [0, 1, 2, 3, 4, 6, 8, -1, -5])
    h = r.choice([0, 1, 2, 3] if small else [0, 1, 2, 3, 4, -1])
    if not small and r.random() < 0.03:   # (past the map's rows, or before the first cell: a fault in both)
        row, col = r.choice([(ROWS + 3, col), (-1, col), (0, -3)])
    return col, row, w, h


def cell(r, ticks, fresh=None, at=0x64):
    c = bytearray(r.bytes(0x7c))
    put(c, 0, '4i', *rect(r, fresh is not None))
    if fresh is None:
        fresh = r.random() < 0.4
    age = r.choice([0, 0, 1, 4, 7, 8, 9, 40, 400]) if fresh else r.choice([400, 4000, 1 << 31, (1 << 32) - 4, r.getrandbits(32)])
    put(c, at, 'I', (ticks - age) & 0xffffffff)
    c[at + 8] = r.choice([1, 1, 1, 0, 0xff]) if fresh else r.choice([1, 0, 0x80])
    return c


def booth(r, regions):
    handles = r.sample(range(1, 12), r.choice([0, 1, 2, 3, 4, 6]))
    slots = bytearray(r.bytes(20 * 12))
    for k, h in enumerate(handles):
        thing = bytearray(r.bytes(0x230))
        thing[2] = r.choice([1, 1, 1, 3, 0, 2])
        put(thing, 0x220, 'I', r.choice([3, 3, 0, 2, 4, 0x103]))
        put(thing, 0xa, 'H', handles[k + 1] if k + 1 < len(handles) and r.random() < 0.85 else 0)
        regions[THINGS + 0x400 * k] = thing
        put(slots, 20 * h, 'I', THINGS + 0x400 * k)
    put(slots, 0, 'I', r.choice([0, THINGS]))   # (only asserted on)
    regions[0x7cfb90] = slots
    spots = [(r.randrange(128), r.randrange(4)) for _ in range(2)]
    if r.random() < 0.2:
        spots[1] = spots[0]
    regions[0x78559c] = struct.pack('<4I', *spots[0], *spots[1])
    cells = bytearray(r.bytes(0x44 * 0x290))
    for x, y in spots:
        put(cells, 0x44 * (y * 128 + x + 1) + 0x24, 'H', r.choice(handles + [0]) if handles else 0)
    regions[WORLD + 0x294] = cells
    regions[0x80239c] = struct.pack('<I', WORLD)
    regions[ANALYSER + 0x1f0d0] = r.bytes(0xc)
    return Call(CALLS['Booth'], [ANALYSER], regions, conv='t', ret='a')


SW = RAM + 0x400000       # (Score: the world [0x80239c]; its map cells from +0x2d8 (id 1), its ticks +0x1da70c and things' head +0x1da746)
SCORE_AT = RAM + 0x20000  # (Score: the analyser, only its value +0x1f0e8 mapped)
MONEY = RAM + 0x60000     # (LoanOffer: the money record, the out 0x400 on)
E28, W28, DEFS = RAM + 0x700000, RAM + 0x900000, RAM + 0x300000   # (the 0x28-byte entries by [0x8023a0] and [0x7cf83c]; the definitions)
HANDLES = 0x41            # (the map cells' handles and parents 0-0x40)
SCORE_TYPES = TYPES + [0xb, 0xc, 0xd, 0x10, 0x11, 0x12, 0x19, 0x1e, 0x15]


def score(r, regions, fixed=None):
    """Score (0x4c7db0): see above. fixed: LoanOffer's limit; then the regions are returned, the analyser the table 0x7cfb90's entry 0
    for the world's word +0x1da720."""
    ticks = struct.unpack_from('<I', regions[TICKS])[0]
    if fixed is not None:
        ticks |= 0x10000   # (LoanOffer: the stamp at most fixed quarters back without wrapping, so a scan stays rare)
    scan = r.random() < 0.03
    value = bytearray(r.bytes(12))
    ages =[0, 0, 1, 4, 9, 400] if fixed is None else [0, 1, 4, 9, fixed]
    stamp = (ticks - (r.choice(ages if not scan else [400, 4000, 1 << 29]) << 2)) & 0xffffffff
    put(value, 0, 'I', stamp)
    age = ((ticks >> 2) - (stamp >> 2)) & 0xffffffff   # (in quarters, unsigned: Game1da70c_MinusAt0Quarters)
    if fixed is not None and age > fixed:
        scan = True
    if not scan:   # (worked out and fresh: the limit at least its age)
        value[8] = r.choice([1, 1, 0x40])
        limit = r.choice([age, min(age + 1, 0xffffffff), 0xffffffff, min(age + r.randrange(100), 0xffffffff)]) if fixed is None else fixed
    elif fixed is None:   # (not worked out, or stale: the limit below its age)
        value[8] = r.choice([0, 1]) if age else 0
        limit = r.choice([0, age - 1, r.randrange(age)]) if value[8] else r.getrandbits(32)
    else:
        value[8] = r.choice([0, 1]) if age > fixed else 0
        limit = fixed
    tail = bytearray(r.bytes(0x3c))
    put(tail, 0, 'I', ticks)
    put(tail, 0x14, 'H', 0)
    regions = {0x80239c: struct.pack('<I', SW), SCORE_AT + 0x1f0e8: value}
    slots = bytearray(r.bytes(20 * 12))
    put(slots, 0, 'I', SCORE_AT)
    regions[0x7cfb90] = slots
    if not scan:
        regions[SW + 0x1da70c] = tail
        if fixed is not None:
            return regions
        return Call(CALLS['Score'], [SCORE_AT, limit & 0xffffffff], regions, conv='t', ret='a')
    cells = bytearray(r.bytes(0x44 * 0x4000))
    for i in range(0x4000):
        o = 0x44 * i
        put(cells, o, 'H', r.randrange(HANDLES))
        put(cells, o + 8, 'I', r.choice(SCORE_TYPES))
        put(cells, o + 0xe, 'H', r.choice([0, 0, 0x40, 0xffbf, r.getrandbits(16)]))
    regions[SW + 0x2d8] = cells
    for base, at, key in ((E28, 0x10ffd8, 0x8023a0), (W28, 0x1102b0, 0x7cf83c)):
        e = bytearray(r.bytes(0x28 * HANDLES))
        for h in range(HANDLES):
            put(e, 0x28 * h + 8, 'I', r.choice(SCORE_TYPES))
            put(e, 0x28 * h + 0x10, 'H', r.choice([0, r.randrange(HANDLES)]))
        regions[base + at] = e
        regions[key] = struct.pack('<I', base)
    n = r.randrange(1, 4)
    ids = r.sample(range(0x10000), n)
    table = bytearray(r.bytes(0x740))
    for i in range(n):
        put(table, 4 * i, 'I', DEFS + 0x400 * i)
        regions[DEFS + 0x400 * i] = put(bytearray(r.bytes(0x400)), 0, 'H', ids[i])
    put(table, 0x528, 'i', n)
    k = r.randrange(n)
    put(table, 0x730, 'HHI', r.choice([ids[k], r.getrandbits(16)]), 0, DEFS + 0x400 * k)
    regions[0x7890a0] = table
    handles = r.sample(range(1, 12), r.choice([0, 1, 2, 4, 6]))
    for j, h in enumerate(handles):
        thing = bytearray(r.bytes(0x1a0))
        put(thing, 0xc, 'H', handles[j + 1] if j + 1 < len(handles) else 0)
        put(thing, 0xe, 'H', r.choice(ids) if r.random() < 0.98 else r.getrandbits(16))
        put(thing, 0x44, 'f', r.choice([0.0, 1.5, 99.9, 255.0, 256.5, 1000.0, -3.0, r.uniform(0, 300)]))
        thing[0x50] = r.choice([0, 0, 1, 2, 3, 8])
        put(thing, 0x19c, 'I', r.choice([4, 0, 0, 1, 0x104]))
        regions[THINGS + 0x400 * j] = thing
        put(slots, 20 * h, 'I', THINGS + 0x400 * j)
    regions[0x7cfb90] = slots
    put(tail, 0x3a, 'H', handles[0] if handles else 0)
    regions[SW + 0x1da70c] = tail
    if fixed is not None:
        return regions
    return Call(CALLS['Score'], [SCORE_AT, limit & 0xffffffff], regions, conv='t', ret='a')


def loan_offer(r, regions):
    """LoanOffer (0x4d06b0): see above."""
    regions = score(r, regions, fixed=0x14)
    money = bytearray(r.bytes(0x200))
    for i in range(8):
        e = 0x14 + 0x20 * i
        put(money, e + 4, 'i', r.choice([0, 1000, 5000, 20000, 100000, -1, r.randrange(-0x80000000, 0x80000000)]))
        put(money, e + 0xc, 'iiii', r.choice([0, 1, 12, 36, 60]), r.choice([0, 1, 100, 500, 3000, -7]), r.choice([0, 0, 1, 0x100]),
            r.choice([0, 1, 5, 12, 40]))
    index = r.choice([0, 1, 2, 3, 4, 5, 6, 7] * 3 + [8, 9])
    value = regions[SCORE_AT + 0x1f0e8]
    if SW + 0x2d8 not in regions and r.random() < 0.7:   # (kept, not worked out again: the score small or past 24 bits, the amount about the credit)
        value = bytearray(value)
        put(value, 4, 'i', r.choice([0, 7, 100, 5000, 250000, 0x1000001, 0x1234567, 0x7fffffff, -5, r.randrange(-0x80000000, 0x80000000)]))
        regions[SCORE_AT + 0x1f0e8] = value
        owed = 0
        for i in range(8):
            term, rate, taken, paid = struct.unpack_from('<4I', money, 0x20 + 0x20 * i)
            owed += (term - paid) * rate if taken else 0
        scaled = int(struct.unpack('<f', struct.pack('<f', float(struct.unpack_from('<i', value, 4)[0])))[0])   # (fild, fmul 1.0 at 24 bits)
        credit = (scaled - owed) & 0xffffffff
        put(money, 0x18 + 0x20 * index, 'I', (credit + r.choice([-1, 0, 0, 1])) & 0xffffffff)
    regions[MONEY] = money
    regions[MONEY + 0x400] = r.bytes(0x20)
    return Call(CALLS['LoanOffer'], [MONEY, MONEY + 0x400, index], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    ticks = r.choice([0, 3, 0x1000, 0xfffffffc, r.getrandbits(32)])
    grid = bytearray(r.bytes(0x44 * 128 * ROWS))
    for i in range(128 * ROWS):
        put(grid, 0x44 * i + 8, 'I', r.choice(TYPES))
        if r.random() < 0.5:
            put(grid, 0x44 * i + 0x2c, 'I', 0)
    regions = {MAP: grid, 0x8023a0: struct.pack('<I', MAP), 0x80239c: struct.pack('<I', (TICKS - 0x1da70c) & 0xffffffff),
               TICKS: struct.pack('<I', ticks)}
    limit = r.choice([0, 1, 2, 3, 10, 100, 0xffffffff, r.getrandbits(32)])
    if op == 'Booth':
        return booth(r, regions)
    if op == 'Score':
        return score(r, regions)
    if op == 'LoanOffer':
        return loan_offer(r, regions)
    if op == 'AnyOffer':   # (0x4d0810: LoanOffer's world, all eight offers asked; the money object the only argument)
        c = loan_offer(r, regions)
        return Call(CALLS[op], [MONEY], c.regions, conv='t', ret='a')
    at = CACHE[op]
    if op not in SUMS:
        regions[CELL] = cell(r, ticks, at=at)
        return Call(CALLS[op], [CELL, limit], regions, conv='t', ret='a')
    analyser = bytearray(r.bytes(0x20 + 0x400 * 0x7c))
    again = set(r.sample(range(0x400), r.choice([0, 1, 3, 6])))
    for i in range(0x400):
        if i in again:
            c = cell(r, ticks, fresh=False, at=at)
        else:   # (cached and fresh: the limit past its age)
            c = bytearray(r.bytes(0x7c))
            put(c, at, 'I', ticks)
            c[at + 8] = r.choice([1, 1, 0x40])
        analyser[0x20 + 0x7c * i:0x20 + 0x7c * (i + 1)] = c
    regions[ANALYSER] = analyser
    return Call(CALLS[op], [ANALYSER, limit], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

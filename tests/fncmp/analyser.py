# The park analyser's small methods (src/game/world/analyser: 0x4c5d10, 0x4c74b0, 0x4c74f0,
# 0x4c7b70, 0x4c94a0-0x4c94d0) against the originals: rings with cursors before, at
# and past their sizes, wrapped or not; looks back of every sign about the cursor and size;
# positions 0 and beyond the grid with scales of either sign. The people's average about a cell (0x4cd880): a cell id of any column and
# row (now and then 0, past the grid or with high bits), a radius 0-3 (now and then 4, 5, or past 2^32 - 127, mostly about the cell 1:
# the far edges' unsigned sums wrap and are cut); the map cells it reads (gMapCells [0x8023a0], each one's first thing, runs of ids each a region) and chains of up to eight things
# (the slots at 0x7cfb90, their next +0xa; each thing in one chain or none) of kind 1 mostly, their floats +0x19c 0..100 mostly (else
# negative, past 255 or NaN). RingFull (0x4c9920): the rings 0-19, wrapped or not, their sizes and cursors about 50. RingAverage
# (0x4c9830): RingFull's rings, their values any bytes, mostly small, or all 255.
# usage: analyser.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Record,Tally,Get4a0,Get4c0,Back,Pos,Around,RingFull,
#        RingAverage)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4c5d10, 0x4c5d70), (0x4c74b0, 0x4c74e9), (0x4c74f0, 0x4c751b), (0x4c7b70, 0x4c7bab),
           (0x4c94a0, 0x4c952b), (0x4cd880, 0x4cd9b8), (0x5da3c0, 0x5da3c1), (0x4c9920, 0x4c9956), (0x4c9830, 0x4c9912)])
NAMES = ['Ctor', 'Record', 'Tally', 'Get4a0', 'Get4c0', 'Back', 'Pos', 'Around', 'RingFull', 'RingAverage']
OPS = [k for k in os.environ.get('OPS', ','.join(NAMES)).split(',') if k]
A = RAM  # the analyser; its +0x20800 on is the region at HI
HI = RAM + 0x20800
CELLS, PEOPLE, SLOTS = RAM + 0x100000, RAM + 0x8000, 0x7cfb90


def s32(v):
    return (v & 0xffffffff) - ((v & 0x80000000) << 1)


def around(r):
    """0x4cd880's case (the module comment)"""
    radius = r.choice([0, 1, 1, 2, 3]) if r.random() < 0.8 else r.choice([4, 5, 0xffffffff, 0xfffffffe])
    column, row = r.choice([0, 1, 5, 126, 127, r.randrange(128)]), r.choice([0, 1, 126, 127, r.randrange(128), r.randrange(128, 0x200)])
    if radius > 5 and r.random() < 0.7:   # (column and row 0: the far edges' sums wrap and are cut, the cells past the near ones read)
        column, row = 0, 0
    cell = (row << 7) + column + 1 if r.random() < 0.9 else r.choice([0, r.getrandbits(16)])
    cid = ((cell & 0xffff) - 1) & 0xffffffff
    cx, cy = cid & 0x7f, cid >> 7
    left, top = s32(cx - radius), s32(cy - radius)
    right = 0x7f if (cx + radius) & 0xffffffff > 0x7f else s32(cx + radius)
    bottom = 0x7f if (cy + radius) & 0xffffffff > 0x7f else s32(cy + radius)
    ids = sorted({((y << 7) + x + 1) & 0xffff for x in range(left, right + 1) for y in range(top, bottom + 1)})
    handles = r.sample(range(1, 40), r.randrange(9))
    slots, people = bytearray(r.bytes(20 * 40)), bytearray(r.bytes(0x200 * 8))
    index = {}
    for i, h in enumerate(handles):
        index[h] = i
        put(slots, 20 * h, 'I', PEOPLE + 0x200 * i)
        people[0x200 * i + 2] = r.choice([1] * 6 + [0, 2, 3, 5])
        if r.random() < 0.85:
            f = r.choice([0.0, 1.0, 50.0, 99.9, 100.0, 0.5, r.uniform(0, 100)])
        else:
            f = r.choice([-1.0, -0.5, 255.9, 256.0, 300.0, float('nan'), 1e10])
        put(people, 0x200 * i + 0x19c, 'f', f)
    firsts = {k: 0 for k in ids}
    keys = list(ids)
    while handles and keys:   # (chains of 1-3 things in random cells)
        size = r.randrange(1, 4)
        chain, handles = handles[:size], handles[size:]
        firsts[keys.pop(r.randrange(len(keys)))] = chain[0]
        for a, b in zip(chain, chain[1:] + [0]):
            put(people, 0x200 * index[a] + 0xa, 'H', b)
    for h in handles:
        put(people, 0x200 * index[h] + 0xa, 'H', 0)
    regions = {SLOTS: slots, PEOPLE: people, 0x8023a0: put(bytearray(r.bytes(8)), 0, 'I', CELLS)}
    run = []
    for k in ids + [None]:
        if run and (k is None or k != run[-1] + 1):
            region = bytearray(r.bytes((run[-1] - run[0]) * 0x44 + 2))
            for j in run:
                put(region, (j - run[0]) * 0x44, 'H', firsts[j])
            regions[CELLS + run[0] * 0x44 - 0x20] = region
            run = []
        if k is not None:
            run.append(k)
    high = r.getrandbits(16) << 16 if r.random() < 0.2 else 0
    return Call(0x4cd880, [r.getrandbits(32), cell | high, radius], regions, conv='t', ret='a')


def case(r):
    lo, hi = r.bytes(0x100), r.bytes(0x1500)
    regions = {RAM: lo, HI: hi}
    op = NAMES.index(r.deal(OPS))
    if op == 7:
        return around(r)
    if op in (8, 9):
        ring = r.randrange(20)
        off = 0x216fc + ring * 0x40 - 0x20800
        put(hi, off + 0x38, 'i', r.choice([0, 1, 49, 50, 51, -1, 0x7fffffff, r.randrange(-3, 60)]))
        put(hi, off + 0x34, 'i', r.choice([0, 48, 49, 50, -1, -2, 0x7fffffff, r.randrange(-3, 60)]))
        hi[off + 0x3c] = r.choice([0, 0, 1, 0x80])
        if op == 9:
            kind = r.choice(['any', 'small', 'full'])
            hi[off + 1:off + 0x33] = r.bytes(0x32) if kind == 'any' else bytes(r.randrange(8) for _ in range(0x32)) if kind == 'small' else bytes([255]) * 0x32
            return Call(0x4c9830, [A, ring], regions, conv='t', ret='b')
        return Call(0x4c9920, [A, ring], regions, conv='t', ret='b')
    if op == 0:
        return Call(0x4c5d10, [RAM + 0x1000 + 4 * r.randrange(8)], {RAM + 0x1000: r.bytes(0x100)},
                    conv='t', ret='a')
    if op == 1:
        ring = r.randrange(1, 18)
        off = 0x216fc + ring * 0x40 - 0x20800
        size = r.choice([0, 1, 2, r.randrange(0x34), r.randrange(-3, 3)])
        put(hi, off + 0x38, 'i', size)
        put(hi, off + 0x34, 'i', r.choice([size - 1, size - 2, size, -1, -2, r.randrange(-2, 0x36)]))
        if r.random() < 0.5:
            put(hi, off + 0x3c, 'B', 0)
        return Call(0x4c74b0, [A, ring, r.getrandbits(32)], regions, conv='t')
    if op == 2:
        return Call(0x4c74f0, [A, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t')
    if op == 3:
        return Call(0x4c94a0, [A, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op == 4:
        return Call(0x4c94c0, [A], regions, conv='t', ret='a')
    if op == 5:
        size = r.choice([0, 1, 0x90, r.randrange(0x91), r.randrange(-3, 3)])
        cursor = r.choice([-1, 0, size - 1, size, r.randrange(-3, max(size, 0) + 1)])
        put(hi, 0x20a68 - 0x20800, 'i', size)
        put(hi, 0x20a64 - 0x20800, 'i', cursor)
        put(hi, 0x20a6c - 0x20800, 'B', r.choice([0, 1, r.getrandbits(8)]))
        back = r.choice([0, 1, -1, cursor, cursor + 1, size, size - 1, r.randrange(-4, max(size, 0) + 4),
                         r.getrandbits(32) - (1 << 31)])
        if back < 0 and cursor - back > 0x90:
            back = -1
        return Call(0x4c94d0, [A, back & 0xffffffff], regions, conv='t', ret='a')
    scale = r.choice([1, 2, 4, 8, -1, -3, r.randrange(1, 200), -r.randrange(1, 200)])
    put(lo, 0xc, 'iI', scale, r.choice([32, 16, r.getrandbits(32)]))
    pos = r.choice([0, 1, 0x80, 0x81, 0xffff, r.randrange(0x10000)]) | r.choice([0, r.getrandbits(16) << 16])
    return Call(0x4c7b70, [A, pos], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

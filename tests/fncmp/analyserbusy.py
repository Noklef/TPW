# The analyser cells' counts of busy map cells (src/game/world/analyser: 0x4c8c40 on one cell, 0x4c81b0 summed over the 0x400) against
# the originals: a cell's rectangle (+0 column, +4 row, +8 width, +0xc height: small, now and then empty, negative or reaching past the
# map's first rows (a fault in both)), its cache (+0x64 the ticks it was made at, +0x68 the count, +0x6c valid or not) fresh, stale or
# made in the future (the age unsigned); the world's ticks ([0x80239c] +0x1da70c) about 0 and 2^32; the map cells ([0x8023a0], 0x44
# bytes, 128 to a row) of types 0-0xb mostly (1, 3, 4, 9 and 0xa count). Sum: the analyser's 0x400 cells, most of them cached and fresh
# (a few to count again, their rectangles small). Booth (0x4c91c0): the two ticket booths' cells (the balance positions 0x78559c, in the
# map's first rows, now and then the same) each heading a chain (+0xa) of up to six things (the slots at 0x7cfb90) of kind 1 mostly,
# their +0x220 3 or not, the chains one list's tails; its value +0x1f0d0 set.
# usage: analyserbusy.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Count,Sum,Booth)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

CALLS = {'Count': 0x4c8c40, 'Sum': 0x4c81b0, 'Booth': 0x4c91c0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x4c8c40, 0x4c8d2c), (0x4c81b0, 0x4c81de), (0x41a960, 0x41a9a7), (0x536300, 0x536390), (0x5d60c0, 0x5d60c3), (0x4c91c0, 0x4c9283), (0x4d8610, 0x4d8750), (0x5da3c0, 0x5da3c1)])
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


def cell(r, ticks, fresh=None):
    c = bytearray(r.bytes(0x7c))
    put(c, 0, '4i', *rect(r, fresh is not None))
    if fresh is None:
        fresh = r.random() < 0.4
    age = r.choice([0, 0, 1, 4, 7, 8, 9, 40, 400]) if fresh else r.choice([400, 4000, 1 << 31, (1 << 32) - 4, r.getrandbits(32)])
    put(c, 0x64, 'I', (ticks - age) & 0xffffffff)
    c[0x6c] = r.choice([1, 1, 1, 0, 0xff]) if fresh else r.choice([1, 0, 0x80])
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


def case(r):
    op = r.deal(OPS)
    ticks = r.choice([0, 3, 0x1000, 0xfffffffc, r.getrandbits(32)])
    grid = bytearray(r.bytes(0x44 * 128 * ROWS))
    for i in range(128 * ROWS):
        put(grid, 0x44 * i + 8, 'I', r.choice(TYPES))
    regions = {MAP: grid, 0x8023a0: struct.pack('<I', MAP), 0x80239c: struct.pack('<I', (TICKS - 0x1da70c) & 0xffffffff),
               TICKS: struct.pack('<I', ticks)}
    limit = r.choice([0, 1, 2, 3, 10, 100, 0xffffffff, r.getrandbits(32)])
    if op == 'Booth':
        return booth(r, regions)
    if op == 'Count':
        regions[CELL] = cell(r, ticks)
        return Call(CALLS['Count'], [CELL, limit], regions, conv='t', ret='a')
    analyser = bytearray(r.bytes(0x20 + 0x400 * 0x7c))
    again = set(r.sample(range(0x400), r.choice([0, 1, 3, 6])))
    for i in range(0x400):
        if i in again:
            c = cell(r, ticks, fresh=False)
        else:   # (cached and fresh: the limit past its age)
            c = bytearray(r.bytes(0x7c))
            put(c, 0x64, 'I', ticks)
            c[0x6c] = r.choice([1, 1, 0x40])
        analyser[0x20 + 0x7c * i:0x20 + 0x7c * (i + 1)] = c
    regions[ANALYSER] = analyser
    return Call(CALLS['Sum'], [ANALYSER, limit], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

# The thing counts of src/game/world/thingscan (Things_TopThreeValues 0x4c8430, Things_CountOfKindAround 0x4c8d30, Cell_FirstObject 0x4d99e0, Map_ObjectAtCell 0x527d60 and 0x527e80 by the entrance; and world/mousemode's MouseState_ResolvePick 0x46c110 over it) against the originals: a park of up to six things
# (thingenv.py) of kinds 1 to 3 on analyser cells of every type, with values at +0x30 from 0 to 22; or things chained in the per-cell lists around a cell id, searched for a
# kind with radius 0 to 3 (to the corner of the map too), counting or stopping at the first; or some of them chained (+0xa) from a cell's
# word at +0x24, of kinds 1, 2, 3 and 0xff, for the first of kind 3; that from a cell (in and out of the map) of type 4, 9, 0xa or another,
# or its parent, the objects' cells (+0x34) over the map, 0 and past it; the entrance's columns and row about the cell's (and wrapping).
# The averages 0x4c7bb0 / 0x4c7cc0 (Person / Staff): the park's things of kind 1 mostly (or 4 to 8 mostly) on cells of every type, their
# floats +0x19c (+0x1f8) 0..100 mostly (else negative, past 255, huge or NaN), the world's +0x1da710 mostly 0. The counts above
# 0x4c9340 / 0x4c93f0 / 0x4c9290 (Above1a8 / Above1a4 / Above1ac) and percentages above / below 0x4c9670 / 0x4c9750 (PctAbove / PctBelow): the same things, the
# bound 0..255 mostly about their bytes (or past 255, with a low byte, or negative).
# usage: thingcount.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Top,Area)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import CELLS, POOL, THING, park  # noqa: E402

CALLS = {'Person': 0x4c7bb0, 'Staff': 0x4c7cc0, 'Above1a8': 0x4c9340, 'Above1a4': 0x4c93f0, 'Above1ac': 0x4c9290, 'PctAbove': 0x4c9670, 'PctBelow': 0x4c9750, 'Top': 0x4c8430, 'Area': 0x4c8d30, 'Near': 0x4c8ed0, 'Unlink': 0x4d9280, 'Any': 0x4c8eb0, 'First': 0x4d99e0, 'At': 0x527d60, 'Entrance': 0x527e80, 'Hover': 0x46c110}
B = Bench([(0x4c8430, 0x4c85a8), (0x4c8d30, 0x4c8ea0), (0x4c8ed0, 0x4c9040), (0x4d9280, 0x4d93b0), (0x4c8eb0, 0x4c8ed0), (0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1), (0x4fa990, 0x4fa9fa), (0x536300, 0x53644c), (0x4d99e0, 0x4d9a5b), (0x527d60, 0x527e76), (0x527e80, 0x527ed8), (0x4d8300, 0x4d8328), (0x46c110, 0x46c1c4),
           (0x4c7bb0, 0x4c7cb4), (0x4c7cc0, 0x4c7da7), (0x51a280, 0x51a287), (0x67a830, 0x67a860),
           (0x4c9340, 0x4c93df), (0x4c93f0, 0x4c948f), (0x4c9290, 0x4c932f), (0x4c9670, 0x4c9744), (0x4c9750, 0x4c9824)])
OUT = RAM + 0x1000


def case(r):
    name = r.deal(OPS)
    if name == 'Near':
        cells = bytearray(r.bytes(0x44 * 0x302))
        for idx in range(0x301):
            if r.random() < 0.5:
                put(cells, idx * 0x44 + 0x2c, 'I', 0)
            put(cells, idx * 0x44 + 0x30, 'H', r.choice([0, 0, 1, 2, 5, 0x8000, 0xffff]))
        cell = r.choice([1, 2, 3, 0x80, 0x81, 0xfe, 0x100, 0x101, 0x14a, r.randrange(1, 0x100)])
        radius = r.choice([0, 1, 2, 3, 3])
        if (((cell - 1) >> 7) + radius) > 4:
            cell = cell & 0x7f or 1
        regions = {CELLS: cells, 0x8023a0: struct.pack('<I', CELLS), RAM + 0x20000 + 0x1f000: r.bytes(0x400), OUT: r.bytes(8)}
        return Call(CALLS[name], [RAM + 0x20000, OUT, r.choice([0, 1, 2, 5, 0x8000, 0xffff, 0x10001]), cell, radius], regions, conv='t', ret='a')
    if name in ('First', 'At', 'Entrance', 'Hover'):
        count = r.choice([0, 1, 2, 3, 4, 6])
        regions, handles, things = park(r, count=count, kinds=(1, 2, 3, 3, 0xff), defs=False)
        pool = bytearray(regions[POOL])
        order = r.sample(range(count), r.randrange(count + 1))
        for k, i in enumerate(order):
            put(pool, THING * i + 0xa, 'H', handles[order[k + 1]] if k + 1 < len(order) else 0)
            put(pool, THING * i + 0x34, 'H', r.choice([1, 0x80, 0x81, 0x4000, 0, r.randrange(1, 0x4001), r.getrandbits(16)]))
        regions[POOL] = pool
        cell = bytearray(r.bytes(0x44))
        put(cell, 0x24, 'H', handles[order[0]] if order else 0)
        regions[OUT] = r.bytes(4)
        if name == 'First':
            regions[RAM + 0x20000] = cell
            return Call(CALLS[name], [RAM + 0x20000, OUT], regions, conv='t', ret='a')
        x, y = (r.choice([0, 1, 5, 126, 127, r.randrange(128), r.randrange(128), r.randrange(128), -1, 128, r.getrandbits(32) - (1 << 31)]) for _ in range(2))
        if name == 'Hover':  # the pointer state's tile: column (tile - 1) & 0x7f, row (tile - 1) >> 7
            tile = r.choice([0, 1, 2, 0x80, 0x81, 0x3fff, 0x4000, 0x4001, 0xffff, r.randrange(1, 0x4001), r.randrange(1, 0x4001), r.getrandbits(16)])
            x, y = (tile - 1) & 0x7f, ((tile - 1) & 0xffffffff) >> 7
        world = 0x32000000
        cid = ((y << 7) + x + 1) & 0xffff
        here = bytearray(r.bytes(0x44))
        put(here, 8, 'I', r.choice([4, 9, 0xa, 4, 9, 0xa, 0, 3, 0xb, 0x19]))
        parent = r.choice([cid, cid, r.randrange(1, 0x4001)])
        put(here, 0x10, 'H', parent)
        if parent == cid:
            here[0x24:0x26] = cell[0x24:0x26]
        else:
            regions[world + parent * 0x44 + 0x294] = cell
        if 0 <= x < 128 and 0 <= y < 128:
            regions[world + cid * 0x44 + 0x294] = here
        regions[0x7cf83c] = struct.pack('<I', world)
        if name == 'Entrance':
            near = lambda v: r.choice([v, v - 1, v + 1, v - 2, v + 2, -(1 << 31), (1 << 31) - 1, r.getrandbits(32) - (1 << 31)])  # noqa: E731
            first, last = sorted([near(x), near(x)]) if r.random() < 0.8 else (near(x), near(x))
            regions[0x7855bc] = struct.pack('<3I', first & 0xffffffff, (y if r.random() < 0.6 else near(y)) & 0xffffffff, last & 0xffffffff)
            regions[world + 0x1da732] = r.bytes(2)
            return Call(CALLS[name], [OUT, x & 0xffffffff, y & 0xffffffff], regions, conv='c', ret='a')   # (x and y by value)
        if name == 'Hover':  # (the entrance's columns about the tile's, its row the tile's half the time)
            first, last = sorted([x + r.randrange(-2, 3), x + r.randrange(-2, 3)])
            regions[0x7855bc] = struct.pack('<3I', first & 0xffffffff, y if r.random() < 0.5 else r.randrange(128), last & 0xffffffff)
            regions[world + 0x1da732] = r.bytes(2)
            state = bytearray(r.bytes(0x28))
            put(state, 0, 'I', r.choice([0, 0, RAM + 0x21000]))
            put(state, 0x24, 'H', tile)
            regions[RAM + 0x20000] = state
            regions[RAM + 0x21000] = bytes([r.getrandbits(8), r.getrandbits(8), r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x11, 0x12, 0x13, 0xff])]) + r.bytes(5)
            return Call(CALLS[name], [RAM + 0x20000], regions, conv='t', ret='v')
        regions[RAM + 0x20000] = struct.pack('<ii', x, y)
        return Call(CALLS[name], [OUT, RAM + 0x20000, RAM + 0x20004], regions, conv='c', ret='a')
    if name == 'Unlink':
        count = r.choice([1, 2, 3, 4, 6])
        regions, handles, things = park(r, count=count, kinds=(1,), defs=False)
        pool = bytearray(regions[POOL])
        order = r.sample(range(count), r.randrange(1, count + 1))
        for k, i in enumerate(order):
            put(pool, THING * i + 8, 'HH', handles[order[k - 1]] if k else 0, handles[order[k + 1]] if k + 1 < len(order) else 0)
        for i in range(count):
            if i not in order:
                put(pool, THING * i + 8, 'HH', r.choice([0, 0, 3]), r.choice([0, 0, 3]))
        regions[POOL] = pool
        game = bytearray(r.bytes(0x40))
        put(game, 0x24, 'H', handles[order[0]] if r.random() < 0.9 else 0)
        regions[RAM + 0x20000] = game
        victim = handles[r.choice(order)] if r.random() < 0.85 else handles[r.randrange(count)]
        return Call(CALLS[name], [RAM + 0x20000, victim], regions, conv='t', ret='v')
    count = r.choice([0, 1, 2, 3, 4, 6])
    regions, handles, things = park(r, count=count, kinds=(1, 1, 1, 2, 3), defs=False, cells=True)
    pool = bytearray(regions[POOL])
    grid = bytearray(regions[CELLS])
    if name in ('Person', 'Staff', 'Above1a8', 'Above1a4', 'Above1ac', 'PctAbove', 'PctBelow'):
        kinds = [4, 5, 6, 7, 8] * 2 + [1, 3, 9, 0xff] if name == 'Staff' else [1] * 6 + [0, 2, 3, 4, 8]
        offset = {'Staff': 0x1f8, 'Above1a8': 0x1a8, 'Above1a4': 0x1a4, 'Above1ac': 0x1ac}.get(name, 0x19c)
        for i in range(count):
            pool[THING * i + 2] = r.choice(kinds)
            if r.random() < 0.85:
                f = r.choice([0.0, 1.0, 50.0, 99.9, 100.0, 0.5, r.uniform(0, 100)])
            else:
                f = r.choice([-1.0, -0.5, 255.9, 256.0, 300.0, float('nan'), 1e10, -1e10])
            put(pool, THING * i + offset, 'f', f)
        regions[POOL] = pool
        if name not in ('Person', 'Staff'):
            bound = r.choice([0, 1, 50, 99, 100, 100, 255, r.randrange(101), 0x100, 0x132, 0x10064, -1, -100, r.getrandbits(32)])
            return Call(CALLS[name], [r.getrandbits(32), bound & 0xffffffff], regions, conv='t', ret='a')
        regions[RAM + 0x1da710] = struct.pack('<I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))
        return Call(CALLS[name], [r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='d')
    if name == 'Top':
        for i in range(count):
            put(pool, THING * i + 0x30, 'i', r.choice([0, 1, 2, 3, 4, 5, 10, 21, 22, r.randrange(23)]))
        regions[POOL] = pool
        regions[OUT] = r.bytes(0x10)
        return Call(CALLS[name], [OUT, OUT + 4, OUT + 8], regions, conv='s', ret='v')
    # (Any is Area with first only set)
    # a chain per cell around the id: every thing goes in the list of some cell near it
    cell = r.choice([1, 2, 3, 0x80, 0x81, 0x82, 0xfe, 0x100, 0x101, 0x14a, r.randrange(1, 0x100)])
    radius = r.choice([0, 0, 1, 2, 3, 3])
    heads = {}
    for i in range(count):
        dx, dy = r.randrange(-3, 4), r.randrange(0, 3)
        idx = ((cell - 1) & 0x7f) + dx
        idx = min(max(idx, 0), 0x7f) + ((((cell - 1) >> 7) + dy) << 7) + 1
        idx = min(idx, 0x2ff)
        put(pool, THING * i + 0xa, 'H', heads.get(idx, 0))
        heads[idx] = handles[i]
    for idx, h in heads.items():
        put(grid, idx * 0x44 - 0x20, 'H', h)
    for idx in range(1, 0x301):  # the cells without a thing: no list
        if idx not in heads:
            put(grid, idx * 0x44 - 0x20, 'H', 0)
    regions[POOL] = pool
    regions[CELLS] = grid
    return Call(CALLS[name], [r.choice([1, 1, 2, 3, 0, 0xff]), cell if (((cell - 1) >> 7) + radius + 3) < 5 else cell & 0x7f or 1, radius] + ([] if name == 'Any' else [r.choice([0, 0, 1])]), regions, conv='s', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

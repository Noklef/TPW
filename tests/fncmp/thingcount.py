# The thing counts of src/game/world/thingscan (Things_TopThreeValues 0x4c8430, Things_CountOfKindAround 0x4c8d30) against the originals: a park of up to six things
# (thingenv.py) of kinds 1 to 3 on analyser cells of every type, with values at +0x30 from 0 to 22; or things chained in the per-cell lists around a cell id, searched for a
# kind with radius 0 to 3 (to the corner of the map too), counting or stopping at the first.
# usage: thingcount.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Top,Area)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import CELLS, POOL, THING, park  # noqa: E402

CALLS = {'Top': 0x4c8430, 'Area': 0x4c8d30, 'Near': 0x4c8ed0, 'Unlink': 0x4d9280}
B = Bench([(0x4c8430, 0x4c85a8), (0x4c8d30, 0x4c8ea0), (0x4c8ed0, 0x4c9040), (0x4d9280, 0x4d93b0), (0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1), (0x4fa990, 0x4fa9fa), (0x536300, 0x53644c)])
OUT = RAM + 0x1000


def case(r):
    name = r.choice(OPS)
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
    if name == 'Top':
        for i in range(count):
            put(pool, THING * i + 0x30, 'i', r.choice([0, 1, 2, 3, 4, 5, 10, 21, 22, r.randrange(23)]))
        regions[POOL] = pool
        regions[OUT] = r.bytes(0x10)
        return Call(CALLS[name], [OUT, OUT + 4, OUT + 8], regions, ret='v')
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
    return Call(CALLS[name], [r.choice([1, 1, 2, 3, 0, 0xff]), cell if (((cell - 1) >> 7) + radius + 3) < 5 else cell & 0x7f or 1, radius, r.choice([0, 0, 1])], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

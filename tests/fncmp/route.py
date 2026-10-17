# A route's straightening (src/game/world/route: 0x5110d0) against the original, with LineStep_Init and MapCell_StepBlocked (and its callees) run
# on cellstep.py's cells: a route wandering a small box of cells (at times on the map's edges), with jumps; the first cell to straighten from at
# the start, in the route, at its end or past it; the steps' mode (0x7cf3c0). Every step in the box (and round it) is set up as cellstep.py's.
# usage: route.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import cellstep  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5110d0, 0x51134f), (0x511350, 0x511412), (0x4d8750, 0x4d8b29), (0x4d0af0, 0x4d0b06), (0x522700, 0x5228b5), (0x536300, 0x536484),
           (0x53ad20, 0x53aefc), (0x4dd4e0, 0x4dd510), (0x412e90, 0x412f00), (0x42a440, 0x42a4f0), (0x436840, 0x436870), (0x436880, 0x4368d0),
           (0x44b220, 0x44b280), (0x549d30, 0x549dc0), (0x55a070, 0x55a0c0), (0x55a360, 0x55a3a0), (0x55a570, 0x55a5b0),
           (0x5da3c0, 0x5da3c1)])  # (cellstep.py's)
ROUTE = RAM + 0x40000
GLOBALS = 0x7cf398  # to 0x7cf3c8: LineStep_Init's sums, distances and directions, and the mode at 0x7cf3c0
MOVES = [(0, -1), (1, 0), (0, 1), (-1, 0)]


def case(r):
    mode = r.choice([0, 1, 2, 2, 2, 3])
    size = r.choice([3, 4, 5])
    bx = r.choice([0, 0x7f - size, r.randrange(0x80 - size)])
    by = r.choice([0, 0x7f - size, r.randrange(0x80 - size)])
    x, y = bx + r.randrange(size), by + r.randrange(size)
    start = x | y << 8
    cells = []
    for _ in range(r.choice([0, 1, 2, 3, r.randrange(4, 12), r.randrange(4, 24), r.randrange(4, 24)])):
        if r.random() < 0.15:
            x, y = bx + r.randrange(size), by + r.randrange(size)
        else:
            dx, dy = r.choice(MOVES)
            x, y = min(max(x + dx, bx), bx + size - 1), min(max(y + dy, by), by + size - 1)
        cells.append(x | y << 8)
    steps = []
    for cy in range(by - 1, by + size + 1):
        for cx in range(bx - 1, bx + size + 1):
            for d in range(4):
                target, source = cellstep.target_of(cx & 0xff, cy & 0xff, d)
                steps.append((source, target, d))
    regions = cellstep.world(r, mode, steps)
    route = bytearray(r.bytes(6 + 2 * 0x100))
    route[0] = len(cells)
    put(route, 2, 'H', start)
    for i, c in enumerate(cells):
        put(route, 6 + 2 * i, 'H', c)
    regions[ROUTE] = bytes(route)
    g = bytearray(r.bytes(0x30))
    put(g, 0x7cf3c0 - GLOBALS, 'I', mode)
    regions[GLOBALS] = bytes(g)
    first = r.choice([0, 0, 0, 1, 2, len(cells) - 1, len(cells), len(cells) + 1, r.randrange(len(cells) + 2)]) & 0xffffffff
    return Call(0x5110d0, [ROUTE, first], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

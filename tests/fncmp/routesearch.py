# The route search (src/game/world/route: 0x511470, 0x511ef0 that starts it, 0x5108a0 that refines a route, 0x511420 that does both, 0x4d8b40 a route's length between two cell ids) against
# the original, with LineStep_Init, the wall follower's step, the straightening and MapCell_StepBlocked (and its callees) run on cellstep.py's
# cells: a box of cells (at times on the map's right and bottom edges) with the start, the target and the route's cells in it, every step from
# it set up as cellstep.py's; round it two rings of cells of type 4 (the steps into the first blocked, along and between the rings open) so the
# walk and its walkers stay in the box; the route's count and the steps (0x7cf3a4) low or about their limits; the limit; the mode (0x7cf3c0)
# and the flag 0x7cf3c8 that notes the walk round. A route to refine: a walk of up to 16 of the box's cells (at times diagonal, at times
# jumping).
# usage: routesearch.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import cellstep  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

RANGES = [(0x511470, 0x511c8c), (0x511ef0, 0x511f2f), (0x5108a0, 0x5110cd), (0x511420, 0x51146a), (0x5110d0, 0x51134f), (0x511350, 0x511412), (0x511c90, 0x511ec7), (0x4d8750, 0x4d8b29),
          (0x4d0af0, 0x4d0b06), (0x522700, 0x5228b5), (0x536300, 0x536484), (0x53ad20, 0x53aefc), (0x4dd4e0, 0x4dd510), (0x412e90, 0x412f00),
          (0x42a440, 0x42a4f0), (0x436840, 0x436870), (0x436880, 0x4368d0), (0x44b220, 0x44b280), (0x549d30, 0x549dc0), (0x55a070, 0x55a0c0),
          (0x55a360, 0x55a3a0), (0x55a570, 0x55a5b0), (0x5da3c0, 0x5da3c1), (0x4d8b40, 0x4d8bf5)]
B = Bench(RANGES)  # (cellstep.py's)
ROUTE = RAM + 0x40000
GLOBALS = 0x7cf398  # to 0x7cf3cc: LineStep_Init's sums, distances and directions, the steps, the mode, the notes and their flag 0x7cf3c8
NEIGHBOURS = [(0, -1), (1, 0), (0, 1), (-1, 0)]
MOVES = NEIGHBOURS + [(1, 1), (1, -1), (-1, 1), (-1, -1)] * 2 + [(0, 0)]
LIMITS = [0, 1, 2, 3, 5, 10, 20, 40, 100, 300, 1000, 60000]


def cell_at(regions, r, handle):
    """The address of the cell's type (+8), its page drawn random when new."""
    a = cellstep.WORLD + 0x294 + handle * 0x44
    for p in range(a & ~0xfff, a + 0x44, 0x1000):
        if p not in regions:
            regions[p] = r.bytes(0x1000)
    return a + 8


def set_type(regions, r, handle, value):
    a = cell_at(regions, r, handle)
    for i, b in enumerate(struct.pack('<I', value)):
        regions[(a + i) & ~0xfff][(a + i) & 0xfff] = b


def get_type(regions, r, handle):
    a = cell_at(regions, r, handle)
    return struct.unpack('<I', bytes(regions[(a + i) & ~0xfff][(a + i) & 0xfff] for i in range(4)))[0]


def boxed(r, mode, size=None):
    """cellstep.py's world for a box of cells (of the size given, else 3 to 8 square) ringed by cells of type 4: (regions, box, ring, bx, by, size)."""
    size = size or r.choice([3, 4, 5, 6, 8])
    bx = r.choice([2, 0x80 - size, r.randrange(2, 0x7f - size)])
    by = r.choice([2, 0x80 - size, r.randrange(2, 0x7f - size)])
    box = {(x, y) for x in range(bx, bx + size) for y in range(by, by + size)}
    steps = [(s, t, d) for (x, y) in sorted(box) for d in range(4) for t, s in [cellstep.target_of(x, y, d)]]
    regions = cellstep.world(r, mode, steps)
    inside = {cellstep.target_of(x, y, 0)[1] for x, y in box}
    for x, y in sorted(box):  # (a cell of type 4 in the box would open the way into the rings)
        if get_type(regions, r, cellstep.target_of(x, y, 0)[1]) == 4:
            set_type(regions, r, cellstep.target_of(x, y, 0)[1], r.choice([0, 1, 3, 9]))
    ring = {((x + dx) & 0xff, (y + dy) & 0xff) for x, y in box for dx, dy in NEIGHBOURS} - box
    rings = set()
    for x, y in ring:  # the first ring and every cell a step from it looks at (the corner tests step from it)
        for d in range(4):
            t, s = cellstep.target_of(x, y, d)
            rings |= {t, s}
    for h in sorted(rings - inside):
        set_type(regions, r, h, 4)
    return regions, box, ring, bx, by, size


def case(r):
    op = r.deal(OPS or [0x511470, 0x511ef0, 0x5108a0, 0x511420, 0x4d8b40])
    mode = r.choice([0, 1, 2, 2, 2, 3])
    regions, box, ring, bx, by, size = boxed(r, mode)
    route = bytearray(r.bytes(6 + 2 * 0x100))
    route[0] = r.choice([0, 0, 0, 1, 5, r.randrange(0x100), r.randrange(0xf0, 0xfd), r.randrange(0xf5, 0xfc), r.randrange(0xf8, 0xfb)])
    for i in range(route[0]):  # (the cells already in the route are the box's: the straightening walks from them)
        put(route, 6 + 2 * i, 'BB', *r.choice(sorted(box)))
    sx, sy = r.choice(sorted(box))
    if op == 0x5108a0:  # a route to refine: a walk of the box's cells from the start
        route[0] = r.choice([0, 1, 2, 2, 3, 3, 4, 5, 6, 8, 10, r.randrange(11, 17)])  # (one of 0xf8 cells or so runs past the emulator's step limit)
        x, y = sx, sy
        for i in range(route[0]):
            if r.random() < 0.1:
                x, y = r.choice(sorted(box))
            else:
                dx, dy = r.choice(MOVES)
                x, y = min(max(x + dx, bx), bx + size - 1), min(max(y + dy, by), by + size - 1)
            put(route, 6 + 2 * i, 'BB', x, y)
    tx, ty = r.choice(sorted(box) * 6 + [(sx, sy)] + sorted(ring))
    put(route, 2, 'BB', sx, sy)
    put(route, 4, 'BB', tx, ty)
    regions[ROUTE] = bytes(route)
    g = bytearray(r.bytes(0x34))
    limit = r.choice(LIMITS)
    put(g, 0x7cf3a4 - GLOBALS, 'I', r.choice([0, 0, 0, 1, limit, max(limit - 1, 0), limit + 1, r.randrange(limit + 1)]))
    put(g, 0x7cf3c0 - GLOBALS, 'I', mode)
    put(g, 0x7cf3c8 - GLOBALS, 'I', r.choice([0, 1, r.getrandbits(32)]))
    regions[GLOBALS] = bytes(g)
    if op == 0x4d8b40:  # (the route's own: the start and target as cell ids, at times with high bits)
        ids = [(y << 7) + x + 1 | (r.getrandbits(16) << 16 if r.random() < 0.1 else 0) for x, y in ((sx, sy), (tx, ty))]
        return Call(op, [*ids, mode], regions, ret='a')
    if op in (0x511ef0, 0x511420):
        return Call(op, [ROUTE, mode], regions, ret='a')
    if op == 0x5108a0:
        return Call(op, [ROUTE, r.choice([0x70000000, r.randrange(100), r.getrandbits(32)])], regions, ret='a')
    start = sx | sy << 8 | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)
    return Call(0x511470, [ROUTE, start, limit], regions, ret='a')


OPS = [int(k, 16) for k in os.environ.get('OPS', '').split(',') if k]


if __name__ == '__main__':
    B.main(case)

# The steering behaviours (src/game/world/steering) against the originals: the constructors, base destructor and exit
# thunks (0x50ce50-0x50d140) on any bytes; seek, arrival and avoid_obstacle's steering (0x50dd80,
# 0x50ddf0, 0x50e350), their per-neighbour / per-obstacle callbacks (0x50e190, 0x50e2b0) and the circle hits
# (0x50cba0, 0x50cc20) on navs with 16.16 values near each other (so that every branch is taken), at times of any size; the
# globals (0x7cf048, 0x7cf100, 0x7cf32c) over any bytes; separation's steering and callback (0x50e0e0, 0x50dfc0) and the
# neighbour visit (0x5102f0, with a logging callback) on navs about the map's cells (edges included), with people listed in
# the cells around them (kinds that count or not, the nav itself among them, more than five near).
# usage: steering.py [CASES [SEED]] [--wild|--both] [--show N]
#   (env OPS=Ctor,Dtor,Destroy,Seek,Arrival,Neighbour,Hit,Ray,Obstacle,Avoid,SepNeighbour,Separation,Visit)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG2, LOG, Bench, Call, RAM, log_region  # noqa: E402

B = Bench([(0x50ce50, 0x50d14a), (0x4d49a0, 0x4d49e0), (0x50cba0, 0x50cdc0), (0x50dd80, 0x50dfd0), (0x50dfd0, 0x50e190), (0x50e190, 0x50e4b0),
           (0x50ece0, 0x50ed20), (0x50f280, 0x50f3c0), (0x50f870, 0x50f8f0), (0x5da3c0, 0x5da3c1),
           (0x5102f0, 0x5104b0), (0x4d8300, 0x4d8330), (0x50b350, 0x50b360)])
CTORS = [0x50ce50, 0x50ced0, 0x50cf40, 0x50cfb0, 0x50d020, 0x50d090, 0x50d100]
DESTROYS = [0x50ce90, 0x50cf10, 0x50cf80, 0x50cff0, 0x50d060, 0x50d0d0, 0x50d140]
GLOBALS = 0x7cf040  # the seven objects, 0x7cf050-0x7cf370, and the force, reach and nearest hit
OPS = os.environ.get('OPS', 'Ctor,Dtor,Destroy,Seek,Arrival,Neighbour,Hit,Ray,Obstacle,Avoid,SepNeighbour,Separation,Visit').split(',')
THINGS = RAM + 0x4000  # the people: 0x200 bytes each, their nav at +0xd4
CELLS = 0x31000000  # the map's cells ([[0x7cf6ec]]): only the rows a case reads are laid out
NAV, OTHER, OUT = RAM + 0x100, RAM + 0x20, RAM + 0x40


def near(r, scale=0x80000):
    """A 16.16 value: mostly within scale of 0, at times any."""
    if r.random() < (0.3 if r.wild else 0.05):
        return r.getrandbits(32) - (1 << 31)
    return r.choice([0, r.randrange(-scale, scale), r.randrange(-0x100, 0x100), r.randrange(-0x1000000, 0x1000000)])


def put(mem, at, *values):
    struct.pack_into('<%dI' % len(values), mem, at, *(v & 0xffffffff for v in values))


def neighbours(r, mem, regions):
    """A nav at the map's cells: NAV or one of the people's (index), its position, and people in the cells about it."""
    count = r.choice([0, 1, 3, 6, 10, 12])
    things = bytearray(r.bytes(0x200 * max(count, 1)))
    self_index = r.randrange(count) if count and r.random() < 0.3 else None
    nav = THINGS + 0x200 * self_index + 0xd4 if self_index is not None else NAV
    navmem, base = (things, 0x200 * self_index + 0xd4) if self_index is not None else (mem, 0x100)
    cell = [r.choice([0, 1, 2, 63, 125, 126, 127, 128, 129, -1, -2, r.randrange(0, 128)]) for _ in range(2)]
    pos = [(c << 16) + r.randrange(0x10000) for c in cell]
    put(navmem, base + 8, *pos)
    put(navmem, base + 4, r.choice([0, 0x8000, 0x10000, 0x18000, r.randrange(0, 0x40000), r.randrange(-0x20000, 0x80000)]))
    reach = r.choice([0x8000, 0x10000, 0x20000, 0x40000, 0x60000])
    # the people: positions near the nav's (the s16 at +4 / +6 are pos >> 8), chained in cells about it
    handles = list(range(1, count + 1))
    r.shuffle(handles)
    table = bytearray(r.bytes(20 * 16))
    lists = {}
    for i, h in enumerate(handles):
        t = 0x200 * (h - 1)
        put(table, 20 * h, THINGS + t)
        things[t + 2] = r.choice([1, 4, 5, 6, 7, 8, 0x12, 1, 4, 0x12, 0, 2, 3, 9, 0x11, 0x13, r.getrandbits(8)])
        near_pos = [p + r.choice([0, r.randrange(-reach, reach), r.randrange(-reach // 4, reach // 4), r.randrange(-0x100, 0x100)]) for p in pos]
        if t + 0xd4 != nav - THINGS:
            put(things, t + 0xd4 + 4, r.choice([0, 0x10000, r.randrange(0, 0x40000), r.randrange(-0x20000, 0x80000)]))
            put(things, t + 0xd4 + 8, *near_pos)
        struct.pack_into('<HH', things, t + 4, *[((p >> 8) + r.choice([0, 0, 1, -1])) & 0xffff for p in near_pos])
        if r.random() < 0.05:
            struct.pack_into('<H', things, t + 4, r.getrandbits(16))
        span = 3 if r.random() < 0.8 else 8
        key = tuple(min(127, max(0, c + r.randrange(-span, span + 1))) for c in cell)
        lists.setdefault(key, []).append(h)
    for (x, y), chain in lists.items():
        for a, b in zip(chain, chain[1:] + [0]):
            struct.pack_into('<H', things, 0x200 * (a - 1) + 0xa, b)
    rows = {}
    for (x, y), chain in lists.items():
        rows.setdefault(y, {})[x] = chain[0]
    for y in range(max(0, cell[1] - 17), min(127, cell[1] + 17) + 1):
        lo, hi = max(0, cell[0] - 17), min(127, cell[0] + 17)
        if lo > hi:
            continue
        row = bytearray(r.bytes(0x44 * (hi - lo + 1)))
        for x in range(lo, hi + 1):
            struct.pack_into('<H', row, 0x44 * (x - lo), rows.get(y, {}).get(x, 0 if r.random() < 0.9 else r.choice([0, 0])))
        regions[CELLS + 0x44 * ((y << 7) + lo + 1) + 0x2b8] = row
    regions.update({THINGS: things, 0x7cfb90: table, 0x7cf6ec: struct.pack('<I', RAM + 0x3000), RAM + 0x3000: struct.pack('<I', CELLS)})
    return nav, navmem, base, reach


def case(r):
    op = r.choice(OPS)
    if op == 'Ctor':
        return Call(r.choice(CTORS), [RAM + r.choice([0x10, 0x11, 0x13])], {RAM: r.bytes(0x40)}, conv='t', ret='a')
    if op == 'Dtor':
        return Call(0x50cea0, [RAM + r.choice([0x10, 0x12])], {RAM: r.bytes(0x40)}, conv='t')
    glob = bytearray(r.bytes(0x340))
    if op == 'Destroy':
        return Call(r.choice(DESTROYS), [], {GLOBALS: glob})
    mem = bytearray(r.bytes(0x200))
    put(mem, 0x100, *(near(r) for _ in range(0x30 // 4)))  # mass, radius, pos, vel, max_force/speed, the axes
    put(mem, 0x104, r.randrange(0, 0x40000))  # radius
    put(mem, 0x11c, r.choice([0x3333, 0x10000, r.randrange(0, 0x100000), near(r)]))  # max_speed
    put(mem, 0x120, *(r.choice([0x10000, -0x10000, 0, r.randrange(-0x10000, 0x10000)]) for _ in range(4)))
    put(mem, 0x130, r.choice([OTHER, NAV + 8, NAV + 0x10]))  # the target
    pos = struct.unpack_from('<ii', mem, 0x108)
    if r.random() < 0.8:  # the other / the target near the nav
        scale = r.choice([0x40000, 0x40000, 0x2000, 0x400000])  # (the length's tiers: under 0x1000, 0x400000 and beyond)
        put(mem, 0x20, pos[0] + near(r, scale), pos[1] + near(r, scale))
    else:
        put(mem, 0x20, near(r), near(r))
    put(mem, 0x28, r.randrange(0, 0x40000) if r.random() < 0.9 else near(r))  # the other's radius
    if op == 'Seek':
        return Call(0x50dd80, [RAM, r.choice([OUT, NAV + 8, NAV + 0x10]), NAV], {RAM: mem}, conv='t', ret='a')
    if op == 'Arrival':
        return Call(0x50ddf0, [RAM, r.choice([OUT, NAV + 8, NAV + 0x10]), NAV], {RAM: mem}, conv='t', ret='a')
    if op == 'Avoid':
        return Call(0x50e350, [RAM, r.choice([OUT, NAV + 8, NAV + 0x10]), NAV], {RAM: mem, GLOBALS: glob}, conv='t', ret='a')
    if op == 'Neighbour':
        return Call(0x50e190, [NAV, OTHER], {RAM: mem, GLOBALS: glob})
    if op == 'SepNeighbour':  # the other a nav: its radius at +4 (0x1c), its pos at +8 (the pair at 0x20)
        put(mem, 0x1c, r.randrange(0, 0x40000) if r.random() < 0.9 else near(r))
        return Call(0x50dfc0, [NAV, r.choice([OTHER - 8, OTHER - 8, OTHER - 8, NAV])], {RAM: mem, GLOBALS: glob})
    if op in ('Separation', 'Visit'):
        regions = {RAM: mem, GLOBALS: glob}
        nav, navmem, base, reach = neighbours(r, mem, regions)
        if op == 'Visit':
            regions[LOG] = log_region()
            radius = r.choice([reach, reach, r.randrange(-0x20000, 0x80000), 0, 0xff, 0x100, 0x10000, 0x10001])
            return Call(0x5102f0, [nav, radius & 0xffffffff, CDECL_LOG2], regions)
        # the path: its points (five at +0x64), count (+0x54), point (+0x5c), done flag (+0x60)
        pos = struct.unpack_from('<ii', navmem, base + 8)
        radius = struct.unpack_from('<i', navmem, base + 4)[0]
        for k in range(5):
            put(navmem, base + 0x64 + 8 * k, *[p + r.choice([0, r.randrange(-abs(radius) - 0x20000, abs(radius) + 0x20000), near(r)]) for p in pos])
        n = r.choice([1, 2, 3, 5, 0, r.getrandbits(32)])
        put(navmem, base + 0x54, n)
        put(navmem, base + 0x5c, r.choice([(n - 1) & 0xffffffff, (n - 1) & 0xffffffff, 0, 1, 2, 4]) if n and n <= 5 else r.randrange(5))
        navmem[base + 0x60] = r.choice([0, 0, 1, r.getrandbits(8)])
        point = struct.unpack_from('<I', navmem, base + 0x5c)[0]
        if point < 5 and r.random() < 0.3:  # the point at the edge: an octagonal distance of radius + 1.0 (and 1 either side)
            put(navmem, base + 0x64 + 8 * point, pos[0] + r.choice([-1, 1]) * (radius + 0x10000 + r.choice([-1, 0, 0, 1])), pos[1])
        return Call(0x50e0e0, [RAM, r.choice([OUT, OUT, nav + 8]), nav], regions, conv='t', ret='a')
    if op == 'Hit':
        if r.random() < 0.2:  # a zero discriminant: x 0, y under 2^18, a radius whose square is 0
            put(mem, 0x20, 0, r.randrange(0, 0x40000))
            return Call(0x50cba0, [OTHER, r.randrange(0, 0x100)], {RAM: mem}, ret='a')
        return Call(0x50cba0, [OTHER, r.choice([r.randrange(0, 0x40000), near(r)]) & 0xffffffff], {RAM: mem}, ret='a')
    if op == 'Ray':  # from (the other), a direction and a centre
        scale = r.choice([0x10000, 0x40000, 0x400000])
        put(mem, 0x30, near(r, 0x10000), near(r, 0x10000), near(r, scale), near(r, scale))
        return Call(0x50cc20, [OTHER, RAM + 0x30, r.choice([RAM + 0x38, OTHER]), r.choice([r.randrange(0, 0x40000), r.randrange(0x100), near(r)]) & 0xffffffff],
                    {RAM: mem}, ret='a')
    # Obstacle: the reach and the nearest hit so far
    put(glob, 0x8, r.choice([r.randrange(0, 0x100000), near(r)]))
    put(glob, 0x2ec, r.choice([0x7fffffff, r.randrange(0, 0x100000), near(r)]))
    return Call(0x50e2b0, [NAV, OTHER], {RAM: mem, GLOBALS: glob})


if __name__ == '__main__':
    B.main(case)

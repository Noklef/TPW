# The strip animation (src/game/model/stripanim: 0x4543c0, 0x4547c0, 0x454190) against the originals, dealt in turn (Apply twice as
# often): an object (+4 flags, +0xb0 -> count, +0xb4 -> shape) with grouped vertices and texture coordinates, t across the ramp (and
# wild), counts from none up.
# FitGround: strips (count, divisors 0-8, ends of random halves mostly up to the divisors) on a footprint of up to 6 by 6 cells at a cell
# 0-10, turned by the holder's right angle (now and then another), over a grid 24 cells wide of heights about -50..50 (now and then
# infinite or NaN); sometimes no map or grid, or flag 2 clear.
# Show (0x454670): shown (any nonzero) with holder.py's animator joining the grid, or hidden with misc.py's RemRect14 object leaving it,
# as the strip owner; the holder's turn in degrees of any value, the root's translation and the model's 0x18 bytes random.
# Grow (0x454550, frames 1, 2, 4, 8 and others) and Refresh (0x4547f0): Show's shown environment with strips for StripAnim_Apply
# (0-3, or none at all), flags of any frame (several, none), growing or not, either direction.
# Restart (0x4df300, src/game/world/object): Grow's environment as the ride at 0x7a4610[+0x20] (0-3) of an object of random bytes whose ride id
# (+0x24) has a record in ridelist.py's list or not (the system flag set or not; arrays of 0-16 dwords, so element 6 is there or not).
# usage: stripanim.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402
import holder  # noqa: E402
import misc  # noqa: E402

B = Bench([(0x454050, 0x45453b), (0x4547c0, 0x4547e7), (0x454670, 0x4547b6), (0x46f150, 0x475010), (0x454550, 0x454661), (0x4547f0, 0x4548eb),
           (0x4df300, 0x4df389), (0x50b350, 0x50b360), (0x55a070, 0x55a0b0), (0x55a0b0, 0x55a100)] + holder.RANGES, sines=256,
          hooks=holder.HOOKS)
OBJECT, SHAPE, CHAIN, VERTS, COORDS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x1000, RAM + 0x2000


HOLDER, MODEL, XF, STRIPS, ENDS, FSHAPE, FVERTS, MAP, GRID, HEIGHTS = (RAM + 0x10000 * i for i in range(1, 11))
GW = 24


def height(r):
    if r.random() < 0.03:
        return r.choice([float('nan'), float('inf'), float('-inf'), 3e38])
    return float(r.randrange(-50, 51)) if r.random() < 0.3 else r.uniform(-50, 50)


def fit_case(r):
    n = r.choice([1, 2, 3, 6, r.randrange(-1, 7)])
    obj = bytearray(r.bytes(0xc8))
    put(obj, 4, 'I', r.getrandbits(32) | 2 if r.random() < 0.95 else r.getrandbits(32) & ~2)
    put(obj, 8, 'I', HOLDER)
    put(obj, 0xb0, '2I', STRIPS, FSHAPE)
    x0, y0 = r.randrange(11), r.randrange(11)
    put(obj, 0xc0, '2I', x0 << 16 | y0, (x0 + r.randrange(-1, 7)) % 0x10000 << 16 | (y0 + r.randrange(-1, 7)) % 0x10000)
    holder = put(bytearray(r.bytes(0x14)), 4, 'I', MODEL)
    put(holder, 0x10, 'i', r.choice([0, 90, 180, 270] * 6 + [r.randrange(-400, 400)]))
    s4, s8 = r.choice([1, 2, 4, 8, r.randrange(9)]), r.choice([1, 2, 4, 8, r.randrange(9)])
    ends = bytearray()
    for _ in range(2 * max(n, 0)):
        hi, lo = (r.randrange(0, 2 * max(s, 1) + 1) if r.random() < 0.2 else r.randrange(0, max(s, 1) + 1) for s in (s4, s8))
        ends += struct.pack('<I', hi << 16 | lo)
    grid = put(bytearray(r.bytes(0x30)), 0x18, 'I', GW)
    put(grid, 0x28, 'I', HEIGHTS)
    heights = bytearray(4 * (GW + 1) * 25)
    for i in range(len(heights) // 4):
        put(heights, 4 * i, 'f', height(r))
    world = put(bytearray(r.bytes(0x70)), 0x6c, 'I', GRID if r.random() < 0.97 else 0)
    regions = {OBJECT: obj, HOLDER: holder, MODEL: put(bytearray(r.bytes(0x7c)), 0x78, 'I', XF), XF: r.floats(r.bytes(0x48), [0x44], 50),
               STRIPS: put(bytearray(0x10), 0, '4I', n % (1 << 32), s4, s8, ENDS), ENDS: ends or bytearray(4), FSHAPE: put(r.bytes(0x14), 0x10, 'I', FVERTS),
               FVERTS: r.bytes(0x30 * max(n, 1) + 0x10), MAP: world, GRID: grid, HEIGHTS: heights,
               0x7a0854: struct.pack('<I', MAP if r.random() < 0.97 else 0)}
    return Call(0x454190, [OBJECT], regions)


SOBJ, SHOLDER, SMODEL, SROOT, SA8, SMAT, SDEST, SPTR = (RAM + 0xc000 + 0x200 * i for i in range(8))


def show_case(r, shown=None):
    if shown is None:
        shown = r.choice([1, 1, 1, 0, 0, 0, 2, 0x100])
    if shown:
        c = holder.animator_grid(r)
    else:
        misc.OPS = ['RemRect14']
        c = misc.case(r)
    owner, a, b = c.args
    regions = dict(c.regions)
    regions[owner] = put(bytearray(regions[owner]), 8, 'I', SA8)
    inner = struct.unpack_from('<I', regions[owner], 0x24)[0]
    regions[inner] = put(bytearray(regions[inner]), 4, 'I', SDEST)
    obj = bytearray(r.bytes(0xc8))
    put(obj, 4, 'I', r.getrandbits(32) | 2 if r.random() < 0.9 else r.getrandbits(32) & ~2)
    put(obj, 8, 'I', SHOLDER)
    put(obj, 0xb4, 'I', owner)
    put(obj, 0xc0, '2I', a, b)
    angle = r.choice([0, 90, 180, 270, -90, 45, 1, r.randrange(-1000, 1000), r.randrange(-1000, 1000), r.getrandbits(32)])
    regions.update({SOBJ: obj, SHOLDER: put(put(bytearray(r.bytes(0x14)), 4, 'I', SMODEL), 0x10, 'I', angle & 0xffffffff),
                    SMODEL: put(bytearray(r.bytes(0xa0)), 0x78, 'I', SROOT), SROOT: r.bytes(0x50),
                    SA8: put(put(bytearray(r.bytes(0x30)), 0x28, 'I', SMAT), 0x2c, 'I', SPTR), SMAT: r.bytes(0x40), SDEST: r.bytes(0xa0),
                    SPTR: r.bytes(8)})
    return Call(0x454670, [SOBJ, shown], regions, conv='c', ret='v')


SCOUNT, SVERTS, SROOT2, SCOORDS = (RAM + 0xd000 + 0x400 * i for i in range(4))


def strips_case(r, op):   # Grow / Refresh
    c = show_case(r, 1)
    regions = c.regions
    obj = regions[SOBJ]
    flags = r.getrandbits(32) & ~0xfe2 | r.choice([2, 2, 2, 0]) | r.choice([0x20, 0x20, 0]) | r.choice([0x40, 0x80, 0xc0, 0])
    put(obj, 4, 'I', flags | r.choice([0, 0x100, 0x200, 0x400, 0x800, 0x900, 0x600, 0xc00, 0xf00, 0]))
    put(obj, 0xb0, 'I', r.choice([SCOUNT] * 9 + [0]))
    put(obj, 0xb8, 'I', r.choice([fb(r.uniform(0, 1.25)), fb(0.25), fb(1.0), r.f()]))
    owner = struct.unpack_from('<I', obj, 0xb4)[0]
    regions[owner] = put(bytearray(regions[owner]), 0x10, 'I', SVERTS)
    regions[SDEST] = put(bytearray(regions[SDEST]), 0x78, 'I', SROOT2)
    verts = bytearray(0x200)
    r.floats(verts, range(0, 0x200, 4), r.choice([1, 10, 100, None]))
    regions.update({SCOUNT: struct.pack('<i', r.choice([1, 2, 3, 0, -1])), SVERTS: verts,
                    SROOT2: put(bytearray(r.bytes(0x70)), 0x68, 'I', SCOORDS), SCOORDS: r.bytes(0x100)})
    if op == 'grow':
        return Call(0x454550, [SOBJ, r.choice([1, 1, 2, 2, 4, 4, 8, 8, 0, 3, 0x10])], regions, conv='c', ret='v')
    return Call(0x4547f0, [SOBJ], regions, conv='c', ret='v')


ROBJ, RNODES, RARRAYS = RAM + 0xf000, RAM + 0xf100, RAM + 0xf600


def restart_case(r):
    c = strips_case(r, 'grow')
    regions = dict(c.regions)
    count = r.choice([0, 1, 2, 3, 4])
    ids = r.sample(range(1, 9), count)
    nodes = bytearray(r.bytes(0x100 * 4))
    for i, rid in enumerate(ids):
        put(nodes, 0x100 * i, 'I', RNODES + 0x100 * (i + 1) if i + 1 < count else 0)
        put(nodes, 0x100 * i + 8, 'I', rid)
        put(nodes, 0x100 * i + 0x1c, 'I', RARRAYS + 0x40 * i)
        put(nodes, 0x100 * i + 0x8c, 'i', r.choice([0, 4, 6, 7, 8, 16, -1]))
    k = r.randrange(4)
    obj = bytearray(r.bytes(0x80))
    put(obj, 0x20, 'I', k)
    put(obj, 0x24, 'I', r.choice(ids + [0, 99]) if ids else r.choice([0, 1, 5]))
    table = bytearray(r.bytes(0x10))
    put(table, 4 * k, 'I', SOBJ)
    regions.update({ROBJ: obj, RNODES: nodes, RARRAYS: r.bytes(0x100), 0x7a4610: table, 0x8791a0: struct.pack('<I', r.choice([0, 1, 1, 1])),
                    0x8791b0: struct.pack('<I', RNODES if count else 0)})
    return Call(0x4df300, [ROBJ], regions, conv='t', ret='v')


def case(r):
    op = r.deal(['apply', 'apply', 'shrink', 'fit', 'show', 'show', 'grow', 'grow', 'refresh', 'refresh', 'restart', 'restart'])
    if op == 'restart':
        return restart_case(r)
    if op == 'fit':
        return fit_case(r)
    if op == 'show':
        return show_case(r)
    if op in ('grow', 'refresh'):
        return strips_case(r, op)
    ram = bytearray(r.bytes(0x3000))
    n = r.choice([0, 1, 2, 3, 8, r.randrange(-3, 40)])
    put(ram, 4, 'I', r.getrandbits(32) | r.choice([2, 2, 2, 0]) if r.random() < 0.9 else r.getrandbits(32) & ~2)
    put(ram, 0xb0, 'I', CHAIN + 0x80)
    put(ram, 0x280, 'i', n)
    put(ram, 0xb4, 'I', SHAPE)
    put(ram, 0x110, 'I', VERTS)
    put(ram, 0x124, 'I', CHAIN)          # holder: +4 model
    put(ram, 0x204, 'I', CHAIN + 0x20)   # model: +0x78 root
    put(ram, 0x298, 'I', CHAIN + 0x40)
    put(ram, 0x2a8, 'I', COORDS)         # root: +0x68 coordinates
    r.floats(ram, range(VERTS - RAM, VERTS - RAM + 0x30 * 40, 4), r.choice([1, 10, 100, None]))
    t = r.choice([r.f(2), r.f(), fb(r.uniform(0, 1.25)), fb(0.25), fb(1.0), fb(0.0), r.f()])
    if op == 'shrink':  # StartShrink: strips or none, flags 0x20 / 0x40 / 0x80 either way
        put(ram, 0xb0, 'I', r.choice([CHAIN + 0x80, 0]))
        return Call(0x4547c0, [OBJECT], {RAM: ram})
    return Call(0x4543c0, [OBJECT, t], {RAM: ram})


if __name__ == '__main__':
    B.main(case)

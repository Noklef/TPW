# A map cell's messages (src/game/world/cellmessage: MapCell_HandleMessage 0x532fc0) against the original, over the originals of what
# it calls, on the worlds of the benches of those callees:
# - Build (flag 0x200) and Walls (flag 0x100): cellbuild.py's world (the cell off the map's edges mostly for Walls), the mode 0x81ae2c
#   0-4. Walls: 0x81b604 set or not; the codes 4, 0x18, 0xb, 0x10 and cellbuild.py's kinds, a 5 often; the ground (Terrain_HeightAt:
#   a 128 x 128 grid of 10-unit cells, only the rows about the cell laid out, heights -60..150) under every laid out entry, whose dwords
#   +4 are rides without records (an entry of type 0xc / 0x11 with parent 0 gets the cell as its parent: a ride of 0 would look up the
#   cell's objects, not laid out here); the face queue (0x871ee0) empty, part full, near full or full.
# - Direction (0x82), Owner (0x83), Count (0x84), EndFlag (0x85): cellbuild.py's world; the cell's direction 0 mostly (0x82); the cell's
#   entry of the track types mostly (its parent's for 0xc / 0x11), with low flags or not (0x85).
# - Ends (0x32): cellkind.py's Clear world; the cell's entry (and its parent: pathmodel.py's) of the track types mostly, its id word the
#   cell's, its 0x44-byte cell ([0x8023a0] + (id - 1) * 0x44) laid out of type 1, 3 or others; its direction never a side (the ends
#   reset on cellkind.py's world would step past what is laid out: Reset does that).
# - Reset (0x32): lineends.py's world, its 0x44-byte cells moved to [0x80239c]'s (gMapCells [0x8023a0] = WORLD + 0x2d8, so that the
#   entries are lineends.py's 0x28-byte cells), the cell of flag 0x20 and a type other than 1 (MapCell_Clear leaves it alone, 0x81d7a8
#   clear); its entry's parent (0xc / 0x11) a cell next to it.
# - Link (0x80): pathlink.py's case (its mode in 0x816d64, its steps a and b). Model (0x81): cellkind.py's Clear world. Kind (others):
#   cellkind.py's SetKind cases (the kind the message; 0x15, 0x415 and others), 0x820ab8 / 0x820abc set or not. DeadEnds (0x87):
#   cellkind.py's Clear world, the cell of type 1 often, 0x818c10 / 0x818c14 about each other, the mode 0x81ae2c 0-4; 0x527ee0 and
#   0x52f580 (agent A's, unported) cdecl recorders 27 of 3 and 28 of 2 (driver.cpp's kCellMessageEnv).
# - Run (0x86): trackshape.py's run world (start 1, flash 1 for UpdateA, then UpdateB; no things on the cells: their plays and flashes are
#   trackshape.py's); a run that would not stop for either is redrawn. The cell's entry (laid out apart from the run's) of the track types
#   mostly, with a parent word whose world cell's object chain (0x7cfb90) holds the run's thing (kind 3) after a decoy, or none; the thing's
#   +0x2c 0 or not. 0x53afa0 is the thiscall recorder 24 (driver.cpp's kCellRunEnv) for these cases only: the others' ends reset runs it.
# The messages' bits past the low byte (0x400 and up) any now and then.
# usage: cellmessage.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Build,Walls,Direction,Owner,Count,EndFlag,Ends,Reset,Link,Model,Kind,DeadEnds,Run)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import cellapply  # noqa: E402
import cellbuild  # noqa: E402
import cellkind  # noqa: E402
import lineends  # noqa: E402
import pathapply  # noqa: E402
import pathlink  # noqa: E402
import pathmodel  # noqa: E402
import trackanim  # noqa: E402
import trackshape  # noqa: E402
from trackrun import STEP_B, nib, u16, u32  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

RANGES = pathapply.merged(
    cellkind.RANGES + pathlink.RANGES + lineends.RANGES + cellbuild.RANGES + [
        (0x532fc0, 0x5345f4), (0x52f840, 0x52f850), (0x527940, 0x527950), (0x4d0af0, 0x4d0b40), (0x53a510, 0x53a540),
        (0x53ad20, 0x53aefc), (0x522700, 0x5228b5), (0x532dd0, 0x532f5e),
        # (rideat.py's: Terrain_SurfaceHeight and its lookups)
        (0x452ae0, 0x452c97), (0x4527f0, 0x452a70), (0x53bf30, 0x53bfa0), (0x527e80, 0x527ed8), (0x527d60, 0x527e76), (0x4d8300, 0x4d8328),
        # (dynfaces.py's square)
        (0x53c8d0, 0x53c9c0),
        # (trackshape.py's runs)
        (0x52c6f0, 0x52d1e8), (0x4d0b10, 0x4d0b27), (0x4d8c00, 0x4d8c4e), (0x50c320, 0x50c540), (0x536300, 0x53644c), (0x4d99c0, 0x4d99e0),
        (0x4d97b0, 0x4d97e0), (0x5da3c0, 0x5da3c1), (0x469180, 0x469350), (0x469580, 0x469850), (0x45b9d0, 0x45ba30)])
HOOKS = {**pathapply.HOOKS, **cellkind.HOOKS, **cellapply.HOOKS, 0x527ee0: _recorder(27, 3, cdecl=True), 0x52f580: _recorder(28, 2, cdecl=True)}
RUN_HOOK = _recorder(24, 0)


class _Bench(Bench):
    """0x53afa0 hooked (the run's env recorder) for the Run cases only: the ends reset of the others runs its original."""

    def original(self, c):
        if getattr(c, 'run', False):
            self.emu.hooks[0x53afa0] = RUN_HOOK
        else:
            self.emu.hooks.pop(0x53afa0, None)
        return super().original(c)


B = _Bench(RANGES, sines=256, hooks=HOOKS)
SPECIAL = (0x32, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87)
TRACK = [0xb, 0xd, 0x10, 0x12, 0x19]
W = cellbuild.W
MAP, GRID, GROUND, FLAGS = RAM + 0x70000, RAM + 0x70100, RAM + 0x80000, RAM + 0xa0000
MSG, DECOY = 0x37000000, 0x37001000


def find(regions, addr, n=1):
    for k, v in regions.items():
        if k <= addr and addr + n <= k + len(v):
            return k
    return None


def peek(regions, addr, fmt):
    k = find(regions, addr, struct.calcsize('<' + fmt))
    return struct.unpack_from('<' + fmt, regions[k], addr - k)[0]


def poke(regions, addr, fmt, *values):
    k = find(regions, addr, struct.calcsize('<' + fmt))
    regions[k] = put(bytearray(regions[k]), addr - k, fmt, *values)


def lay(regions, addr, data):
    """data at addr when it overlaps nothing laid out; whether it did."""
    if any(k < addr + len(data) and addr < k + len(v) for k, v in regions.items()):
        return False
    regions[addr] = data
    return True


def setg(regions, addr, data):
    """A global: written into the region holding it, else laid out."""
    k = find(regions, addr, len(data))
    if k is not None:
        regions[k] = bytearray(regions[k])
        regions[k][addr - k:addr - k + len(data)] = data
    else:
        assert lay(regions, addr, bytearray(data)), hex(addr)


def high(r):   # (bits past the low byte the switch ignores)
    return r.choice([0, 0, 0, 0x400, 0xff0000, r.getrandbits(32) & ~0x3ff])


def mode(r, regions):
    setg(regions, 0x81ae2c, struct.pack('<I', r.choice([0, 1, 2, 3, 4, 4, r.getrandbits(32)])))


def ground(r, regions, y):
    """Terrain_HeightAt's grid: 128 x 128 cells of 10, the heights rows y - 1 .. y + 2 and the flags rows y - 1 .. y + 1 laid out."""
    grid = put(put(put(bytearray(r.bytes(0x30)), 0x10, '2f', 10.0, 10.0), 0x18, '2I', 128, 128), 0x28, '2I', GROUND, FLAGS)
    regions.update({0x7a0854: struct.pack('<I', MAP), MAP: put(bytearray(r.bytes(0x70)), 0x6c, 'I', GRID), GRID: grid,
                    0x7a4610: bytes(0x40)})
    for iy in range(max(0, y - 1), min(128, y + 2) + 1):
        heights = [float(r.randrange(-60, 151)) if r.random() < 0.4 else r.uniform(-60, 150) for _ in range(129)]
        regions[GROUND + 4 * 129 * iy] = struct.pack('<129f', *heights)
    for iy in range(max(0, y - 1), min(127, y + 1) + 1):
        regions[FLAGS + 4 * 128 * iy] = r.bytes(4 * 128)


def queue(r, regions):
    """The face queue (0x820b00, tags 0x872168, count 0x871ee0): room for every square a case adds after the count."""
    n = r.choice([0, 0, 1, 5, 0x806 - 40, 0x806 - 3, 0x806, 0x810])
    m = min(n, 0x806)
    regions.update({0x820b00 + 0xa0 * m: r.bytes(0xa0 * max(1, min(0x806, m + 100) - m)), 0x872168 + 4 * m: r.bytes(4 * 101),
                    0x871ee0: struct.pack('<I', n), 0x763c98: bytes([r.choice([0, 1])]) + bytes(3), 0x874fc4: struct.pack('<I', r.choice([0, 0, 1, 5]))})


def window_of(cell):
    ident = (cell - W - 0x294) // 0x44
    return ident, [(ident + dy * 128 + dx) & 0xffff for dy in (-1, 0, 1) for dx in (-1, 0, 1)]


def build_world(r, calm=False, inner=False):
    regions, cell = cellbuild.world(r, calm=calm, inner=inner)
    mode(r, regions)
    return regions, cell


def build(r):
    regions, cell = build_world(r)
    kind = r.choice(cellbuild.KINDS) if r.random() < 0.95 else r.randrange(0x40)
    return Call(0x532fc0, [cell, 0x200 | kind | r.choice([0, 0x100, 0x400, r.getrandbits(32) & ~0x3ff]), r.getrandbits(32), r.getrandbits(32)],
                regions, conv='t', ret='a')


def walls(r):
    regions, cell = build_world(r, calm=r.random() < 0.5, inner=True)
    ident, window = window_of(cell)
    for k in window:   # (rides without records: Terrain_SurfaceHeight reads the ground)
        e = W + 0x1102b0 + k * 0x28
        if peek(regions, e + 8, 'I') in (0xc, 0x11):
            if not peek(regions, e + 0x10, 'H'):
                poke(regions, e + 0x10, 'H', ident)
            e = W + 0x1102b0 + peek(regions, e + 0x10, 'H') * 0x28
        poke(regions, e + 4, 'I', r.randrange(1, 0x10))
    ground(r, regions, ((ident - 1) & 0xffff) >> 7)
    queue(r, regions)
    setg(regions, 0x81b604, struct.pack('<I', r.choice([0, 0, 1])))
    code = r.choice([4, 0x18, 0xb, 0x10, 4, 0x18, 0xb, 0x10] + cellbuild.KINDS)
    a = r.choice([5, 5, 0, 1, 2, 3, r.getrandbits(32)])
    return Call(0x532fc0, [cell, 0x100 | code | r.choice([0, 0, 0x400, r.getrandbits(32) & ~0x3ff]), a, r.choice([0, 1, 2, r.getrandbits(32)])],
                regions, conv='t', ret='a')


def simple(r, code):
    regions, cell = build_world(r)
    ident, window = window_of(cell)
    if code == 0x82:
        poke(regions, cell + 0xd, 'B', r.choice([0, 0, 0, 0, 1, 4, 0x10, 0x40, r.getrandbits(8)]))
    if code == 0x85:   # (the entry, or its parent, of the track types mostly, with low flags or not)
        e = W + 0x1102b0 + peek(regions, cell, 'H') * 0x28
        poke(regions, e + 8, 'I', r.choice(TRACK * 2 + [0xc, 0x11, 1, 3, 0]))
        if peek(regions, e + 8, 'I') in (0xc, 0x11):
            e = W + 0x1102b0 + peek(regions, e + 0x10, 'H') * 0x28
            poke(regions, e + 8, 'I', r.choice(TRACK * 2 + [1, 3, 0]))
        poke(regions, e + 0xe, 'B', r.choice([0, 0, 0, 1, 8, 0xf]) | r.getrandbits(8) & 0xf0)
    step = [-1, -1, 1, 1, 0, 2, r.getrandbits(32)]
    return Call(0x532fc0, [cell, code | high(r), r.choice(step) & 0xffffffff, r.choice(step) & 0xffffffff], regions, conv='t', ret='a')


def kind_world(r, op):
    cellkind.OPS = [op]
    c = cellkind.case(r)
    regions = dict(c.regions)
    setg(regions, 0x820abc, struct.pack('<I', r.choice([0, 0, 1])))
    setg(regions, 0x816d64, r.bytes(4))
    return c, regions


def ends(r):
    c, regions = kind_world(r, 'Clear')
    cell = pathmodel.CELL
    h = peek(regions, cell, 'H')
    e = pathmodel.G + 0x10ffd8 + h * 0x28
    poke(regions, e, 'H', h)
    if peek(regions, e + 8, 'I') in (0xc, 0x11):   # (pathmodel.py's parent in [0x7cf83c]'s world)
        e = pathmodel.W1 + (peek(regions, e + 0x10, 'H') + 0x6cde) * 0x28
        poke(regions, e, 'H', h)
        poke(regions, e + 8, 'I', r.choice(TRACK * 2 + [1, 3, 0]))
    else:
        poke(regions, e + 8, 'I', r.choice(TRACK * 2 + [1, 3, 0]))
    poke(regions, e + 0xd, 'B', r.choice([0, 2, 8, 0x20, 0x80, 0x11, 0x44, 0x05]))
    poke(regions, e + 0xe, 'B', r.choice([0, 1, 8, 0xf, 0xf, 3]) | r.getrandbits(8) & 0xf0)
    c44 = bytearray(r.bytes(0x44))
    put(c44, 0, 'H', h)
    put(c44, 8, 'I', r.choice([1, 3, 0, 0x15, 9, 0, 0]))
    assert lay(regions, pathmodel.G + (h - 1) * 0x44, c44)
    return Call(0x532fc0, [cell, 0x32 | high(r)] + c.args[1:], regions, conv='t', ret='a')


def reset(r):
    regions = dict(cellapply.case(r).regions)
    world, mapc = lineends.WORLD, lineends.MAPC
    regions.update({0x80239c: struct.pack('<I', world), 0x7cf83c: struct.pack('<I', world), 0x7cdba0: lineends.pairs(r)})
    x, y = r.choice([r.randrange(4, 124)] * 4 + [0, 1, 126, 127]), r.choice([r.randrange(4, 124)] * 4 + [0, 1, 126, 127])
    lineends.block(r, regions, x, y)
    regions[0x7639e0] = struct.pack('<4i', *(r.choice([lineends.spot(r), -1]) for _ in range(4)))
    for k in [k for k in regions if mapc <= k < mapc + 0x110000]:   # (the 0x44-byte cells under [0x80239c]'s world)
        assert lay(regions, world + 0x2d8 + k - mapc, regions.pop(k)), hex(k)
    regions[0x8023a0] = struct.pack('<I', world + 0x2d8)
    h = lineends.ident(x, y)
    cell = world + 0x294 + h * 0x44
    poke(regions, cell, 'H', h)
    poke(regions, cell + 8, 'I', r.choice([0x15, 0, 0x14, 2, 3, 9]))
    poke(regions, cell + 0xe, 'H', r.getrandbits(16) | 0x20)
    e = lineends.TABLE + h * 0x28
    if peek(regions, e + 8, 'i') in (0xc, 0x11):   # (its parent a cell next to it)
        poke(regions, e + 0x10, 'H', lineends.ident(min(127, max(0, x + r.randrange(-1, 2))), min(127, max(0, y + r.randrange(-1, 2)))))
    setg(regions, 0x81d7a8, bytes(4))
    return Call(0x532fc0, [cell, 0x32 | high(r), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


def link(r):
    cellkind.OPS = ['Clear']
    c = pathlink.case(r)
    regions = dict(c.regions)
    setg(regions, 0x816d64, struct.pack('<I', c.args[1]))
    return Call(0x532fc0, [pathmodel.CELL, 0x80 | high(r), c.args[2], c.args[3]], regions, conv='t', ret='a')


def model(r):
    c, regions = kind_world(r, 'Clear')
    return Call(0x532fc0, [pathmodel.CELL, 0x81 | high(r), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


def kind(r):
    c, regions = kind_world(r, 'SetKind')
    message = c.args[1]
    if message & 0x300 or message & 0xff in SPECIAL:
        message = r.choice([0x15, 1, 3, 0x415, (message & ~0x3ff) | 0x15])
    if r.random() < 0.15:
        message = r.choice([0x15, 0x415])
    return Call(0x532fc0, [pathmodel.CELL, message, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')


def dead_ends(r):
    c, regions = kind_world(r, 'Clear')
    if r.random() < 0.5:
        poke(regions, pathmodel.CELL + 8, 'I', 1)
    level = r.choice([0, 5, -1, 100])
    setg(regions, 0x818c10, struct.pack('<2i', level + r.choice([-1, 0, 1, 1, 1]), level))
    setg(regions, 0x81ede4, r.bytes(8))
    mode(r, regions)
    return Call(0x532fc0, [pathmodel.CELL, 0x87 | high(r)] + c.args[1:], regions, conv='t', ret='a')


def run(r):
    while True:
        x0, y0, d, hoarding, rows, cells, thing = trackanim.draw(r, False, 1)
        angle = u32(thing, 0x10)
        sx, sy, db = STEP_B[angle] if angle in STEP_B else (0, 0, nib(1))
        if trackshape.stops(cells, x0, y0, d, hoarding) and trackshape.stops(cells, thing[5] + sx, thing[7] + sy, db, hoarding):
            break
    for cell in cells.values():
        if r.random() < 0.3:
            cell[0xe] &= 0xf0
        struct.pack_into('<I', cell, 4, 0)
    world, table = trackanim.WORLD, trackanim.CELLS
    regions = dict(rows)
    thing = bytearray(thing) + bytearray(r.bytes(0x10))
    put(thing, 0x2c, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    thing[2] = 3
    regions.update({trackshape.THING: thing, 0x81b4cc: struct.pack('<I', r.choice([1, 0xff, 0x101]) if hoarding else r.choice([0, 0x100])),
                    0x7cf83c: struct.pack('<I', world), 0x80239c: struct.pack('<I', world), 0x8023a0: struct.pack('<I', world + 0x2d8),
                    0x7cdba0: trackshape.PAIRS, 0x818c50: r.bytes(4), LOG: log_region()})

    def entry(kinds):
        while True:
            eid = r.randrange(1, 0x4000)
            e = bytearray(r.bytes(0x28))
            put(e, 0, 'H', eid)
            put(e, 8, 'I', r.choice(kinds))
            if lay(regions, table + eid * 0x28, e):
                return eid, e
    eid, e = entry(TRACK * 3 + [0xc, 0x11, 1, 3, 0x19])
    if u32(e, 8) in (0xc, 0x11):
        pid, e = entry(TRACK * 3 + [1, 0x19])
        put(e, 0, 'H', pid)
        poke(regions, table + eid * 0x28 + 0x10, 'H', pid)
    p = r.choice([0, r.randrange(1, 0x3000), r.randrange(1, 0x3000), r.randrange(1, 0x3000)])
    put(e, 0x10, 'H', p)
    h2 = r.choice([1, 2, 0x101, 0x1ff, r.randrange(1, 0x200)])
    h1 = r.choice([k for k in (3, 0x102, 0x1fe, r.randrange(1, 0x200)) if k != h2])
    slots = bytearray(r.bytes(20 * 0x200))
    put(slots, 20 * h2, 'I', trackshape.THING)
    put(slots, 20 * h1, 'I', DECOY)
    decoy = bytearray(r.bytes(0x10))
    decoy[2] = r.choice([0, 1, 2, 4])
    put(decoy, 0xa, 'H', r.choice([h2, h2, 0]))
    regions.update({0x7cfb90: slots, DECOY: decoy})
    put(thing, 0xa, 'H', 0)
    if p:
        c44 = bytearray(r.bytes(0x44))
        put(c44, 0x24, 'H', r.choice([h1, h1, h2, 0]))
        assert lay(regions, world + 0x294 + p * 0x44, c44)
    cell = bytearray(r.bytes(0x44))
    put(cell, 0, 'H', eid)
    regions[MSG] = cell
    c = Call(0x532fc0, [MSG, 0x86 | high(r), r.choice([0, 0x10000, 0xffff0000, r.getrandbits(32)]), r.getrandbits(32)], regions, conv='t', ret='a')
    c.run = True
    return c


CASES = {'Build': build, 'Walls': walls, 'Direction': lambda r: simple(r, 0x82), 'Owner': lambda r: simple(r, 0x83),
         'Count': lambda r: simple(r, 0x84), 'EndFlag': lambda r: simple(r, 0x85), 'Ends': ends, 'Reset': reset, 'Link': link, 'Model': model,
         'Kind': kind, 'DeadEnds': dead_ends, 'Run': run}
OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]


def case(r):
    return CASES[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)

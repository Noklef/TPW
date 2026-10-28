# The track tool dragged (src/game/world/focuscell: FocusCell_TrackDrag 0x527960) against the original, its callees run:
#   Mode: no focus cell (0x818698 0): the focus object none, or one on thingenv.py's park whose definition (its id one of the park's mostly)
#     has +0x40 and +0x9c any of the switch's values (both recorded on actionrec.py's recorder); 0x81ecac (without a definition) and the flag
#     0x8187f8 picking the mode 0xe / 0x13 / 0x1a or none, the current mode (0x81ae2c) the same or not (GameMode_SetOnly: recorded, the world's
#     advisor asked, agent B's unported 0x59aa70 thiscall recorder 13 of 1 as in gamemode.py).
#   Trace: trackrun.py's TraceA world about the focus object (+0x2c 0): the cell 0x7639a4 / 0x7639a8 and (x, y) any, the drag start set.
#   Line: no drag start (the word 0x7639b0 -1): mapstep.py's path nudge world at the point, the line's cell operation recorded (slot 20, the
#     MapLine env); the focus object's +0x2c set.
#   Connect: cellbuild.py's world (MapCell_CanBuildTrack's, its kinds as the current mode, the cell's 0x28-byte entry owned track now and
#     then for 9) about the point, laid out wider: the 0x28-byte entries 6 cells about it (ids right, track mostly two cells apart, the
#     path nudge's flag 0x10 now and then), the 0x44-byte cells 3 about; the drag start (0x818694, its word 0x7639b0 about the point's +0x24)
#     the point, a cell two or four away (on a straight track to the point half the time) or another; a case whose CellRun_Connect walk
#     (cellrun.py's model, for the start and the point or a nudged one either way) would not end is redrawn. Agent B's unported cell
#     operations (0x5370e0 recorder 20 of 3, 0x53b280 7 of 3) as in cellrun.py.
# The point lands on the case's cell: the cell 0x7639a4 / 0x7639a8 is the point less Position_StepA's step (quarter turns only: others give
# a stack address as the code) less an even amount, (x, y) the point or one past it.
# usage: trackdrag.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Mode,Trace,Line,Connect)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import actionrec  # noqa: E402
import cellbuild  # noqa: E402
import cellrun  # noqa: E402
import mapstep  # noqa: E402
import thingenv  # noqa: E402
import trackrun  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


RANGES = cellbuild.RANGES + [
    (0x527960, 0x527cec), (0x4dd4e0, 0x4dd4fb), (0x412e90, 0x412f00), (0x52fa90, 0x52fb20), (0x52f840, 0x52f872), (0x52f200, 0x52f83b),
    (0x403be0, 0x403f00), (0x5194d0, 0x51950d), (0x52a3a0, 0x52a4b0),
    (0x530830, 0x530ac2), (0x4d8c00, 0x4d8c50), (0x4d8300, 0x4d8340), (0x522700, 0x5228b5), (0x53a510, 0x53a53d), (0x53ad20, 0x53aefc),
    (0x50c320, 0x50c540), (0x536300, 0x53644c), (0x4d99c0, 0x4d99e0), (0x4d96f0, 0x4d9920), (0x5da3c0, 0x5da3c1),
    (0x52b400, 0x52b99a), (0x53aac0, 0x53ad1c), (0x4d0af0, 0x4d0b40), (0x535db0, 0x535dc3), (0x539210, 0x539230), (0x523070, 0x52322f)]
B = Bench(merged(RANGES),
          hooks={0x5370e0: _recorder(20, 3), 0x53b280: _recorder(7, 3), 0x59aa70: _recorder(13, 1), 0x4474d0: _recorder(14, 2, cdecl=True),
                 0x446050: _recorder(15, 0, cdecl=True), 0x53bc60: _recorder(27, 3, cdecl=True), 0x545450: _recorder(28, 1, cdecl=True)})
OPS = [k for k in os.environ.get('OPS', 'Mode,Trace,Line,Connect').split(',') if k]
STEP_A = trackrun.STEP_A   # angle: (dx, dy, code) of Position_StepA
ANGLES = [0, 90, 180, 270]
OBJ = RAM + 0x40000        # the Mode op's focus object
THING = RAM + 0x5000       # the Line and Connect ops' focus object
R = 6                      # the Connect op's entries this many cells about the point
SIDES = [1, 4, 0x10, 0x40]
PAIRS = bytearray(0x40)
for _k, (_dx, _dy) in enumerate(cellbuild.PAIRS):
    put(PAIRS, 8 * _k, 'ii', _dx, _dy)


def no_defs():
    """gObjectDefs (0x7890a0) with no definitions: Object_GetDef answers none."""
    return put(put(bytearray(0x738), 0x528, 'I', 0), 0x730, 'HHI', 0xfffe, 0, 0)


def focus_object(r, scrap):
    """A focus object (0x40 bytes): a quarter turn (+0x10), the id 0x55 (no definition), +0x2c 0 for scrap None, else set."""
    t = bytearray(r.bytes(0x40))
    put(t, 0x10, 'I', r.choice(ANGLES))
    put(t, 0xe, 'H', 0x55)
    put(t, 0x2c, 'I', 0 if scrap is None else r.choice([1, 1, 0x100, r.getrandbits(32) | 1]))
    return t


def aim(r, t, px, py, regions):
    """The cell (0x7639a4 / 0x7639a8), the direction (the focus object's) and (x, y) so that the point is (px, py); the drag globals."""
    dx, dy, _ = STEP_A[struct.unpack_from('<I', t, 0x10)[0]]
    a = px - dx - 2 * r.choice([0, 0, 1, -1, 2, -3, 20])
    b = py - dy - 2 * r.choice([0, 0, 1, -1, 2, -3, 20])
    steps = bytearray(r.bytes(0x50))
    put(steps, 4, 'ii', a, b)
    regions[0x7639a0] = steps
    return [(px + r.choice([0, 1])) & 0xffffffff, (py + r.choice([0, 1])) & 0xffffffff, r.getrandbits(32)]


def drag_globals(r, regions, thing, focus=None):
    """The focus block 0x818684..0x8186d8 (cellbuild.py's 0x818688 block kept when there is one), the cell word set, the floats."""
    block = bytearray(r.bytes(0x54))
    if focus is not None:
        block[4:] = focus
    put(block, 0x14, 'H', r.choice([1, 0x81, r.getrandbits(16) | 1]))   # 0x818698
    put(block, 0x28, 'I', thing)                                          # 0x8186ac
    regions[0x818684] = block
    regions[0x818c50] = r.bytes(4)
    regions[0x820af8] = r.bytes(8)
    regions.setdefault(0x81ede4, r.bytes(8))
    regions.setdefault(0x816d48, r.bytes(0x1c))
    regions.setdefault(LOG, log_region())
    return block


def quiet_mode(r, regions, mode=None):
    """The current mode, and 0x81ecac / 0x8187f8 so that no mode is set (0x81ecac 0, 3, 5, or the current mode's)."""
    mode = r.choice([0xe, 0x13, 0x1a, 0, 0xb, r.getrandbits(32)]) if mode is None else mode
    regions[0x81ae2c] = struct.pack('<I', mode)
    pick = {0xe: 1, 0x13: 2, 0x1a: 4}.get(mode, 0)
    regions[0x81ecac] = struct.pack('<I', r.choice([0, 3, 5, pick, pick]))
    regions.setdefault(0x8187f8, struct.pack('<I', r.choice([0, 0, 1])))
    regions[0x81b0cc] = r.bytes(4)


def mode_case(r):
    regions, handles, addrs = thingenv.park(r, count=r.choice([1, 2, 3]), defs=True)
    items = bytearray(regions[thingenv.ITEMS])
    n = struct.unpack_from('<i', regions[thingenv.DEFS], 0x528)[0]
    ids = [struct.unpack_from('<H', items, thingenv.ITEM * i)[0] for i in range(n)]
    for i in range(thingenv.NITEMS):
        put(items, i * thingenv.ITEM + 0x40, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        put(items, i * thingenv.ITEM + 0x9c, 'I', r.choice([1, 2, 4, 0, 3, 1, 2, 4, r.getrandbits(32)]))
    regions[thingenv.ITEMS] = bytes(items)
    obj = bytearray(r.bytes(0x40))
    put(obj, 0xe, 'H', r.choice(ids) if r.random() < 0.85 else 0xfffd)
    regions[OBJ] = obj
    thing = r.choice([OBJ, OBJ, OBJ, 0])
    block = drag_globals(r, regions, thing)
    put(block, 0x14, 'H', 0)
    regions[0x81ae2c] = struct.pack('<I', r.choice([0xe, 0x13, 0x1a, 0, 3, 0x15, r.getrandbits(32)]))
    regions[0x81ecac] = struct.pack('<I', r.choice([1, 2, 4, 0, 3, r.getrandbits(32)]))
    regions[0x8187f8] = struct.pack('<I', r.choice([0, 0, 0, 1, 0x100]))
    regions[0x81b0cc] = r.bytes(4)
    regions[0x81af64] = r.bytes(4)
    world = 0x31000000
    regions[0x7cf83c] = struct.pack('<I', world)
    regions[world + 0x1da71c] = struct.pack('<H', r.choice(handles + [0, 11]))
    rec = actionrec.writer(r)
    regions[0x785a08] = rec.pop(actionrec.THIS)
    regions.update(rec)
    return Call(0x527960, [r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='c', ret='v')


def trace_case(r):
    while True:
        x0, y0, d, hoarding, rows, cells, t = trackrun.draw(r, False)
        if trackrun.stops(cells, x0, y0, d, hoarding, False, False):
            break
    regions = dict(rows)
    thing = bytearray(r.bytes(0x40))
    thing[:0x20] = t
    put(thing, 0xe, 'H', 0x55)
    put(thing, 0x2c, 'I', 0)
    regions[trackrun.THING] = thing
    regions.update({0x81b4cc: struct.pack('<I', r.choice([1, 0xff, 0x101]) if hoarding else r.choice([0, 0x100])),
                    0x7cf83c: struct.pack('<I', trackrun.WORLD), 0x80239c: struct.pack('<I', trackrun.WORLD), 0x7cdba0: trackrun.PAIRS,
                    0x81b740: r.bytes(0x800), 0x820a8c: r.bytes(4), 0x81ede4: r.bytes(8), 0x7890a0: no_defs()})
    drag_globals(r, regions, trackrun.THING)
    quiet_mode(r, regions)
    args = aim(r, thing, r.randrange(-4, 0x84), r.randrange(-4, 0x84), regions)
    put(regions[0x7639a0], 0x10, 'H', r.choice([0, 1, 0x7fff, 0x8000, r.getrandbits(16)]) if r.random() < 0.95 else 0xfffe)
    return Call(0x527960, args, regions, conv='c', ret='v')


def line_case(r):
    c = mapstep.nudge(r)
    regions = dict(c.regions)
    px, py = struct.unpack('<ii', bytes(regions.pop(c.args[0])))
    thing = focus_object(r, True)
    regions[THING] = thing
    regions[0x7890a0] = no_defs()
    regions[0x820ac8] = r.bytes(0x10)
    drag_globals(r, regions, THING)
    quiet_mode(r, regions)
    args = aim(r, thing, px, py, regions)
    put(regions[0x7639a0], 0x10, 'H', 0xffff)
    return Call(0x527960, args, regions, conv='c', ret='v')


def part(mem, a, n):
    """The bytes a..a+n of a Sparse layout (laid out), or None."""
    for s, b in mem.parts.items():
        if s <= a and a + n <= s + len(b):
            return bytes(b[a - s:a - s + n])
    return None


def entry(k):
    return cellbuild.W + 0x1102b0 + (k & 0xffff) * 0x28


def connect_case(r):
    W = cellbuild.W
    while True:
        regions, c44 = cellbuild.world(r, calm=r.random() < 0.8, inner=True)
        e = (c44 - W - 0x294) // 0x44
        ex, ey = cellrun.xy_of(e)
        if R <= ex < 0x80 - R and R <= ey < 0x80 - R:
            break
    mem = cellbuild.Sparse(r)
    for a in [a for a in regions if W <= a < W + 0x1000000]:
        mem.parts[a] = bytearray(regions.pop(a))
    window3 = [cellrun.ident(ex + i, ey + j) for j in (-1, 0, 1) for i in (-1, 0, 1)]
    owner = struct.unpack_from('<H', regions[0x818688], 0x10)[0]
    word = r.getrandbits(16)
    words = [word, (word + 1) & 0xffff, (word - 1) & 0xffff, r.getrandbits(16)]
    track = [0xb, 0x10, 0x19, 0xd, 0x12]
    for j in range(-R, R + 1):
        for i in range(-R, R + 1):
            k = cellrun.ident(ex + i, ey + j)
            a = entry(k)
            if k not in window3:
                mem.ensure(a, 0x28)
                mem.put8(a + 2, r.choice([2, 2, 0, 1]))
                t = r.choice(track + [0, 1, 3, 4, 0x15, 7])
                mem.put(a + 8, 'I', t)
                mem.put8(a + 0xc, cellrun.side_bits(r))
                mem.put8(a + 0x22, cellrun.side_bits(r))
                # (track: its owner the player's or not; others name a laid out 0x44-byte cell, the anchor of a footprint)
                mem.put(a + 0x10, 'H', r.choice([owner, owner, r.choice(window3), 0]) if t in track else r.choice(window3))
            mem.put(a, 'H', k)
            mem.put8(a + 0xd, r.choice(SIDES * 3 + [0, 0x11]))
            mem.put(a + 0xe, 'H', r.choice([0, 0, 0x10, 0x10, 0x30, 0x40, 0x100]))
            if r.random() < 0.7:
                mem.put(a + 0x24, 'H', r.choice(words))
    for j in range(-3, 4):
        for i in range(-3, 4):
            k = cellrun.ident(ex + i, ey + j)
            if k not in window3:
                cellbuild.lay_cell(r, mem, k, window3, r.choice(window3), True)
    cellbuild.lay_cell(r, mem, 0, window3, r.choice(window3), True)   # (the anchor 0)
    mem.put(c44, 'H', e)
    near = [cellrun.ident(ex + 2 * i, ey + 2 * j) for i, j in ((0, -1), (1, 0), (0, 1), (-1, 0))]
    for k in [e] + near:   # (the point's and its nudges' entries: not stand-ins, so the walks start where the model starts them)
        t = struct.unpack('<I', part(mem, entry(k) + 8, 4))[0]
        if t in (0xc, 0x11):
            mem.put(entry(k) + 8, 'I', r.choice(track))
    mode = r.choice([0xe, 0x13, 0xe, 0x13, 0xb, 0x10, 0x19, 0xf, 0x39, 0])
    if mode in (0xe, 0x13) and r.random() < 0.7:   # (owned track: 9 unless it spreads past two side neighbours)
        mem.put(entry(e) + 8, 'I', r.choice([0xb, 0x10]))
        mem.put(entry(e) + 0x10, 'H', owner)
    hoarding = regions[0x81b4cc][0] != 0
    base = dict(mem.parts)
    for _ in range(20):
        mem.parts = {a: bytearray(b) for a, b in base.items()}
        if r.random() < 0.5:   # (a straight track from the start to the point)
            d = r.choice(SIDES)
            mx, my = cellrun.MOVES[d]
            n = r.choice([1, 1, 2])
            sx, sy = ex - n * mx, ey - n * my
            for s in range(n + 1):
                k = cellrun.ident(sx + s * mx, sy + s * my)
                mem.put(entry(k) + 8, 'I', r.choice([0xb, 0x10, 0x19]))
                nb = d | cellrun.REVERSE[d]
                mem.put8(entry(k) + 0xc, nb)
                mem.put8(entry(k) + 0x22, nb)
            start = cellrun.ident(sx, sy)
            mem.put8(entry(start) + 0xd, d)
        else:
            start = r.choice([e, e] + near + [cellrun.ident(ex + r.randrange(-4, 5), ey + r.randrange(-4, 5))])
        cells = {}
        for j in range(-R, R + 1):
            for i in range(-R, R + 1):
                k = cellrun.ident(ex + i, ey + j)
                cells[k] = part(mem, entry(k), 0x28)
        if struct.unpack_from('<I', cells[start], 8)[0] in (0xc, 0x11):
            continue
        pairs = [(start, start)] + [p for k in [e] + near for p in ((k, start), (start, k))]
        if all(cellrun.connect_model(cells, f, t, hoarding, 0) for f, t in pairs):
            break
    else:
        return connect_case(r)
    regions.update(mem.parts)
    regions[0x7cdba0] = PAIRS
    thing = focus_object(r, True)
    regions[THING] = thing
    block = drag_globals(r, regions, THING, focus=regions.pop(0x818688))
    here = struct.unpack('<H', part(mem, entry(e) + 0x24, 2))[0]
    put(block, 0x10, 'I', start | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0))   # 0x818694
    args = aim(r, thing, ex, ey, regions)
    sw = r.choice([here, (here + 1) & 0xffff, (here - 1) & 0xffff, r.getrandbits(16)])
    put(regions[0x7639a0], 0x10, 'H', sw if sw != 0xffff else 0)
    quiet_mode(r, regions, mode)
    return Call(0x527960, args, regions, conv='c', ret='v')


CASES = {'Mode': mode_case, 'Trace': trace_case, 'Line': line_case, 'Connect': connect_case}


def case(r):
    return CASES[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)

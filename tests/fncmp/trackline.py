# A track's line walked and the cells off it removed (src/game/world/trackline: TrackLine_Prune 0x5311c0) against the original.
# trackremove.py's scene for TrackCell_Remove (its world's cells replaced) in the game's layout: [0x7cf83c] = [0x80239c] = the world,
# [0x8023a0] = the world + 0x2d8, so that the map cells (MapHandle_Cell44, Map_StepCell) and the 0x28-byte cells are each one table;
# pathmodel.py's model tables and pathapply.py's queue pieces for MapCell_ApplyPathModel (no terrain: [0x7a0854] null). A small map
# ([0x81b1b0] rows, [0x81d740] columns: 3-8 mostly, now and then 0-2) laid out with two more rows and columns each side: those of flag
# 0x20 and not of kind 0x19 (so that the walk and the side scans stop inside), the two rows above now and then without the flag; the
# rest of kinds 0x19, 0xc / 0x11 (owners inside, of other kinds), the line kinds and others, flag 0x20 now and then; their map cells
# of flag 0x40 now and then, of kind 0x15 or others, their owners inside. The line: from (0x7639f0, 0x7639f4) a walk two cells at a
# time (no cell twice) of kind 0x19 cells facing on (their bits the way in and out: straight or a corner), the cell two on from its end
# not of kind 0x19, or its last cell of one neighbour; the cell two east of the start (the walk's first facing) not of kind 0x19 either
# unless the line goes on there. Now and then: the line on the margin too (the spot two on from its end off the map:
# Map_StepCell28InDirection null), a straight cell but the start a T, a start going east of one neighbour.
# usage: trackline.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAND_SEED, Bench, Call, blob, put, set_return  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import cellapply  # noqa: E402
import cellkind  # noqa: E402
import cellmodel  # noqa: E402
import lineends as le  # noqa: E402
import pathmodel  # noqa: E402
import trackremove as tr  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

WORLD, TABLE, MAPC, ident = le.WORLD, le.TABLE, le.MAPC, le.ident
CELLS = WORLD + 0x294   # (the map cells: + id * 0x44)
RANGES = cellapply.merged(tr.RANGES + [(0x5311c0, 0x531a50), (0x53ad60, 0x53ad70), (0x53ad90, 0x53adb0), (0x53ae00, 0x53ae50)])
HOOKS = tr.HOOKS
B = Bench(RANGES, sines=256, hooks=HOOKS)
STEP = {1: (0, -2), 4: (2, 0), 0x10: (0, 2), 0x40: (-2, 0)}
REVERSE = {1: 0x10, 4: 0x40, 0x10: 1, 0x40: 4}


def line(r, x, y, w, h, lo=2):
    """The line's cells from (x, y) facing 4 (in columns and rows lo to w / h + 1): {(x, y): bits}, and the spot two on from its end
    (or None: its last cell of one bit)."""
    cells, facing = {}, 4
    for _ in range(r.randrange(1, 9)):
        here = (x, y)
        options = [d for d in (1, 4, 0x10, 0x40) if d != REVERSE[facing]]
        inside = [d for d in options if lo <= x + STEP[d][0] <= w + 1 and lo <= y + STEP[d][1] <= h + 1 and
                  (x + STEP[d][0], y + STEP[d][1]) not in cells and (x + STEP[d][0], y + STEP[d][1]) != here]
        if not inside or r.random() < 0.1:
            break
        out = r.choice(inside + [facing] * 2) if facing in inside else r.choice(inside)
        cells[here] = REVERSE[facing] | out
        facing = out
        x, y = x + STEP[out][0], y + STEP[out][1]
    if (x, y) in cells:
        return cells, None
    ends = [d for d in (1, 4, 0x10, 0x40) if d != REVERSE[facing] and (x + STEP[d][0], y + STEP[d][1]) not in cells]
    if not ends or r.random() < 0.15:
        cells[(x, y)] = REVERSE[facing]
        return cells, None
    out = r.choice(ends)
    cells[(x, y)] = REVERSE[facing] | out
    return cells, (x + STEP[out][0], y + STEP[out][1])


def put_kind(regions, x, y, kind):
    """The kind (+8) of the laid-out map's 0x28-byte cell (x, y)."""
    put(regions[TABLE + ident(0, y) * 0x28], x * 0x28 + 8, 'i', kind)


def put_owner(regions, x, y, owner):
    """The owner (+0x10) of the laid-out map's 0x28-byte cell (x, y)."""
    put(regions[TABLE + ident(0, y) * 0x28], x * 0x28 + 0x10, 'H', owner)


def case(r):
    regions = {k: v for k, v in tr.case(r).regions.items() if not (TABLE <= k < TABLE + 0xa0000 or MAPC <= k < MAPC + 0x110000)}
    pm = pathmodel.case(r).regions
    for k in (0x763138, 0x820ac0, RAND_SEED):
        regions[k] = pm[k]
    # (pathmodel.py's table 0x7630b0 is the tail of cellmodel.py's 27 rows at 0x762ff0; its counts 0x700c20 / 0x700c24 one region)
    tail = bytearray(regions[0x762ff0])
    tail[0xc0:0xc0 + len(pm[0x7630b0])] = pm[0x7630b0]
    regions.update({0x762ff0: tail, 0x700c20: pm[0x700c20][:4], 0x700c24: pm[0x700c20][4:]})
    log = regions[LOG]
    blob(log, 30, 2, 0xf)
    if r.random() < 0.5:
        set_return(log, 30, r.choice([0, 1, r.getrandbits(32)]))
    w, h = r.choice([r.randrange(3, 9)] * 6 + [0, 1, 2]), r.choice([r.randrange(3, 9)] * 6 + [0, 1, 2])
    wl, hl = max(w, 4), max(h, 4)   # (the laid-out map: the line needs room)
    lo = 0 if r.random() < 0.08 else 2   # (now and then a line on the margin: the spot two on from its end off the map)
    x0, y0 = r.randrange(lo, wl + 2), r.randrange(lo, hl + 2)
    cells, beyond = line(r, x0, y0, wl, hl, lo)
    for spot, bits in cells.items():   # (now and then a straight cell but the start a T: not a corner, so the walk keeps on)
        if bits in (0x11, 0x44) and spot != (x0, y0) and r.random() < 0.1:
            cells[spot] = bits | r.choice([4, 0x40] if bits == 0x11 else [1, 0x10])
    if r.random() < 0.1:
        cells[(x0, y0)] = None   # (a start not of kind 0x19)
    elif cells[(x0, y0)] == 0x44 and r.random() < 0.3:
        cells[(x0, y0)] = 4   # (a start of one neighbour, east: the walk goes on from it)
    # the cell east two of the start not of kind 0x19 unless the line goes on there (the walk faces east from the start)
    east = (x0 + 2, y0) if cells[(x0, y0)] is None or (x0 + 2, y0) not in cells else None
    inner = [ident(x, y) for x in range(2, wl + 2) for y in range(2, hl + 2)]
    rows = {}
    for y in range(hl + 4):
        row = bytearray()
        for x in range(wl + 4):
            c = le.cell(r, ident(x, y))
            put(c, 8, 'i', r.choice([0x19] * 4 + [0xc, 0x11] * 2 + tr.LINES + le.KINDS))
            put(c, 0x10, 'H', r.choice(inner))
            put(c, 0xe, 'H', r.choice([0x20, 0x20, 0, 0x40, r.getrandbits(16)]))
            if x < 2 or y < 2 or x >= wl + 2 or y >= hl + 2:   # (the margin)
                put(c, 8, 'i', r.choice([0, 0xb, 0xd, 3]))
                c[0xe] |= 0x20
                if y < 2 <= x and r.random() < 0.2:   # (the rows above without it: the north scan stops once its row is negative)
                    c[0xe] &= ~0x20
            if (x, y) in cells:
                bits = cells[(x, y)]
                if bits is None:
                    put(c, 8, 'i', r.choice([0, 0xd, 0xc]))
                else:
                    put(c, 8, 'i', 0x19)
                    c[0xc] = c[0x22] = bits
            if (x, y) in (beyond, east):
                put(c, 8, 'i', r.choice([0, 0xb, 0xd, 3, 0xc]))
            row += c
            regions[WORLD + 0x1b02d8 + cellkind.block(ident(x, y)) * 4] = r.bytes(4)
        rows[y] = row
    # the owners of the 0xc / 0x11 cells not of those kinds themselves (TrackCell_Remove goes on to the owner's: a chain of them can
    # loop until the stack runs out, which the original and the port report differently)
    kind = {(x, y): struct.unpack_from('<i', rows[y], x * 0x28 + 8)[0] for y in rows for x in range(wl + 4)}
    owners = [ident(x, y) for (x, y), k in kind.items() if k not in (0xc, 0x11) and ident(x, y) in inner]
    for (x, y), k in kind.items():
        if k in (0xc, 0x11) and owners:
            put(rows[y], x * 0x28 + 0x10, 'H', r.choice(owners))
    for y, row in rows.items():
        regions[TABLE + ident(0, y) * 0x28] = row
    for y in range(hl + 5):
        mapc = bytearray(r.bytes(0x44 * (wl + 5)))
        for x in range(wl + 5):
            at = x * 0x44
            put(mapc, at, 'H', ident(x, y))
            mapc[at + 2] = r.choice([2, 0, 1])
            put(mapc, at + 4, 'I', r.choice([0, 0, 0, 5]))
            put(mapc, at + 8, 'i', r.choice([0x15, 0x15, 0, 1, 3, 5, 9, -1, r.getrandbits(32) - (1 << 31)]))
            mapc[at + 0xc], mapc[at + 0x22] = cellmodel.bits(r), cellmodel.bits(r)
            put(mapc, at + 0xe, 'H', r.choice([0x40, 0x40, 0, 0x20, r.getrandbits(16)]))
            put(mapc, at + 0x10, 'H', r.choice(inner))   # (an owner on the map: MapCell_Clear reads its cell)
            put(mapc, at + 0x24, 'H', r.choice([0, 3, 4, 5, 6, 7]))
        regions[CELLS + ident(0, y) * 0x44] = mapc
    regions.update({0x8023a0: struct.pack('<I', WORLD + 0x2d8), 0x7639f0: struct.pack('<2i', x0, y0), 0x818c50: r.bytes(1),
                    0x81b1b0: struct.pack('<i', h), 0x81d740: struct.pack('<i', w), 0x7a0854: bytes(4), 0x763388: r.bytes(0x200),
                    0x816d68: struct.pack('<2I', r.choice([1, 2]), r.choice([1, 2])), 0x820aa8: r.bytes(12)})
    return Call(0x5311c0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

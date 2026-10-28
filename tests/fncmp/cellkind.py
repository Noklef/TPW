# A map cell's kind set and a cell cleared (src/game/world/cellkind: MapCell_SetKind 0x5346d0, MapCell_Clear 0x5367a0, which call each
# other) against the originals. pathapply.py's scene (pathmodel.py's cell, model tables and neighbours, scene.py's terrain, the queue
# pieces' recorders 30 / 31), widened to the 5 x 5 cells about the cell in [0x80239c]'s map (pathmodel.py's side cells kept; the kinds 1,
# 9, 3, 0, 0xa, 5 or any; owners among two ids or 0) and the 3 x 3 entries ([0x8023a0] + 0x10ffd8 + id * 0x28, none of type 0xc / 0x11
# but the cell's own), the diagonal direction pairs too (0x7cdba0, mostly the real ones). object.py's Scrap object (its clock, imports
# and definitions; the clock in [0x80239c]'s world) is the thing of handles 0, 3 and 4 in the thing table 0x7cfb90; handle 1 the money
# record (money.py's record), 2 the park thing (+0x1fc90 / +0x1f5a0), 5-7 decoys of other types (+2) chained 5 -> 6 -> 7 -> 3, 4 or 0;
# the owners' cells in [0x7cf83c]'s map of types 4, 9, 0xa, 0 or others, their first thing (+0x24) any of 0, 3-7. 0x4de1f0 (agent C's,
# unported) a thiscall recorder (26, driver.cpp's kCellKindEnv). The flags 0x81d7a8, 0x8186d4, 0x785a2c and 0x820ab8 (Kinds_Compatible)
# set or not, the prices 0x81b6dc / 0x81ea14 about the money's balance, the counter 0x7cdb98 and the cell's block value.
# SetKind: kinds 0, 1, 3, 4, 9, 0x15, the cell's own or any on cells of those types (kind 1 on a type 3 cell and kind 3 on a type 1 one
# often). Clear: the cell's type 1, 0xa, 3, 9, 4, 0x15 or others, its flag 0x20 set or not, its count +0x20 about 0, a and b 0 or not.
# Reset (0x53af00 TrackCell_Reset): SetTrack's world and ClearBlock's track cells about the cell, 0x81b4cc set or not.
# usage: cellkind.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetKind,Clear,SetTrack,ClearBlock,Reset)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import cellmodel  # noqa: E402
import money  # noqa: E402
import object as objects  # noqa: E402
import pathapply  # noqa: E402
import pathmodel  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

W1, W2, G = pathmodel.W1, pathmodel.W2, pathmodel.G
objects.OBJ, objects.ITEMS, objects.WORLD, objects.OPS = RAM + 0x50000, RAM + 0x50400, W2, ['Scrap']
MONEY, DECOYS, PARK, TRACK = RAM + 0x54000, RAM + 0x55000, RAM + 0x60000, RAM + 0x56000
CALLS = {'SetKind': 0x5346d0, 'Clear': 0x5367a0, 'SetTrack': 0x538fc0, 'ClearBlock': 0x53bb20, 'Reset': 0x53af00}
RANGES = (pathapply.merged(pathapply.RANGES + objects.RANGES + money.RANGES +
                           [(0x5346d0, 0x5348c5), (0x5367a0, 0x536c0d), (0x522730, 0x522800), (0x522810, 0x522860), (0x4d8c00, 0x4d8cb0),
                            (0x4d97e0, 0x4d9a70), (0x539210, 0x539230), (0x52fb20, 0x52fb60), (0x5195d0, 0x519620), (0x535600, 0x535680),
                            (0x6ad810, 0x6ad830), (0x538fc0, 0x539202), (0x53bb20, 0x53bc55), (0x52fb60, 0x52fba0), (0x4d0af0, 0x4d0b40),
                            (0x4d96f0, 0x4d97e0), (0x50c320, 0x50c3f0), (0x53af00, 0x53af96)]))
HOOKS = {**pathapply.HOOKS, 0x4de1f0: _recorder(26, 0)}
B = Bench(RANGES, sines=256, hooks=HOOKS)
DIAGONALS = {0x10: (-1, 1), 0x18: (1, 1), 0x20: (-1, -1), 0x38: (1, -1)}   # (0x20, 8, 0x80, 2's pairs)
KINDS = [1, 1, 1, 9, 9, 3, 0, 0xa, 5]


def ident(x, y):
    return ((y << 7) + x + 1) & 0xffff


def block(h):   # (Map_SetBlockValue's dword for the id h)
    i = h - 1
    return (i >> 11) * 33 + ((i >> 4) & 7)


def things(r, regions, world=W1):   # (world: the [0x7cf83c] whose +0x1da720 handles name the money and the park thing)
    slots = bytearray(r.bytes(20 * 8))
    for h, p in enumerate([objects.OBJ, MONEY, PARK, objects.OBJ, objects.OBJ, DECOYS, DECOYS + 0x10, DECOYS + 0x20]):
        put(slots, 20 * h, 'I', p)
    decoys = bytearray(r.bytes(0x30))
    for i, nxt in enumerate([6, 7, r.choice([3, 4, 0])]):
        decoys[i * 0x10 + 2] = r.choice([0, 1, 2, 4, 0x83])
        put(decoys, i * 0x10 + 0xa, 'H', nxt)
    regions[objects.OBJ][2] = 3
    regions.update({0x7cfb90: slots, DECOYS: decoys, MONEY: money.record(r), PARK + 0x1fc90: r.bytes(4), PARK + 0x1f5a0: r.bytes(4),
                    world + 0x1da720: struct.pack('<HHHH', 2, r.getrandbits(16), r.getrandbits(16), 1)})


def patch(r, regions, x, y, owners, reach=2, kinds=None):
    """The cells within reach of (x, y) (inside the map; kinds: {(dx, dy): kind} given) and the entries within reach - 1; pathmodel.py's
    side cells and the cell's entry kept."""
    for dy in range(-reach, reach + 1):
        yy = y + dy
        if not 0 <= yy < 128:
            continue
        x0, x1 = max(0, x - reach), min(127, x + reach)
        row = bytearray()
        for xx in range(x0, x1 + 1):
            cell = regions.pop(W2 + 0x294 + ident(xx, yy) * 0x44, None)
            if cell is None:
                cell = bytearray(r.bytes(0x44))
                cell[2] = r.choice([2, 2, 0, 1])
                cell[0xc], cell[0x22] = cellmodel.bits(r), cellmodel.bits(r)
                put(cell, 4, 'I', r.choice([0, 0, 0, 5, r.getrandbits(32)]))
            cell = bytearray(cell)
            put(cell, 0, 'H', ident(xx, yy))   # (its id: Map_StepCell from it steps from there)
            put(cell, 8, 'I', r.choice(KINDS + [r.getrandbits(32)]))
            if kinds and (xx - x, dy) in kinds:
                put(cell, 8, 'I', kinds[(xx - x, dy)])
            put(cell, 0x10, 'H', r.choice(owners))
            row += cell
        regions[W2 + 0x294 + ident(x0, yy) * 0x44] = row
        if abs(dy) < reach:
            x0, x1 = max(0, x - reach + 1), min(127, x + reach - 1)
            entries = bytearray()
            for xx in range(x0, x1 + 1):
                entry = regions.pop(G + 0x10ffd8 + ident(xx, yy) * 0x28, None)
                if entry is None:
                    entry = bytearray(r.bytes(0x28))
                    put(entry, 8, 'I', r.choice([1, 3, 0xb, 0, 0x19]))
                    entry[0xe] = r.choice([0, 0, 0, 1, 8, 0xf]) | r.getrandbits(8) & 0xf0
                entries += entry
            regions[G + 0x10ffd8 + ident(x0, yy) * 0x28] = entries


def track_cell(r, h, kind=None):
    """A 0x28-byte track cell of id h: its kind (+8) of the track kinds mostly, flag 0x40 (+0xe) or not, its count +0x20 about 0."""
    c = bytearray(r.bytes(0x28))
    put(c, 0, 'H', h)
    c[2] = r.choice([2, 2, 0, 1])
    put(c, 8, 'I', r.choice([0, 0xb, 0x10, 0x19, 7, 1, 3, r.randrange(0x20)]) if kind is None else kind)
    put(c, 0xe, 'H', r.choice([0, 0x40, r.getrandbits(16)]))
    put(c, 0x20, 'h', r.choice([-1, 0, 1, r.getrandbits(16) - 0x8000]))
    return c


def track_case(r, op, regions, h, owners, clear):
    """SetTrack: a track cell of id h whose map cell (MapHandle_Cell44: [0x8023a0] + (h - 1) * 0x44) is 1 or 0 when clear; ClearBlock:
    the 3 x 3 track cells about it in [0x80239c]'s table (+ 0x1102b0 + id * 0x28: Map_StepCell28's (id + 0x6cde) * 0x28)."""
    x, y = (h - 1) & 0x7f, (h - 1) >> 7
    regions.update({0x81aebc: struct.pack('<I', r.choice([0, 1, 50, 100, 0x80000000, r.getrandbits(32)])), 0x820ad0: struct.pack('<I', r.choice([0, 1, 1])),
                    0x81b43c: struct.pack('<I', r.choice([0, 1, 50, 100, 0x80000000, r.getrandbits(32)]))})
    if op in ('ClearBlock', 'Reset'):
        for dy in (-1, 0, 1):
            if 0 <= y + dy < 128:
                x0, x1 = max(0, x - 1), min(127, x + 1)
                regions[W2 + 0x1102b0 + ident(x0, y + dy) * 0x28] = b''.join(track_cell(r, ident(xx, y + dy)) for xx in range(x0, x1 + 1))
        regions[TRACK] = track_cell(r, h)
        if op == 'Reset':
            regions[0x81b4cc] = bytes([r.choice([0, 0, 1])])
        return Call(CALLS[op], [TRACK], regions, conv='t', ret='v')
    here = bytearray(r.bytes(0x44))
    put(here, 0, 'H', h)
    here[2] = r.choice([2, 2, 0, 1])
    here[0xc], here[0x22] = cellmodel.bits(r), cellmodel.bits(r)
    put(here, 8, 'I', r.choice([1, 1, 0]) if clear else r.choice(KINDS))
    put(here, 0xe, 'H', r.choice([0, 0x20, r.getrandbits(16)]))
    put(here, 0x10, 'H', r.choice(owners))
    put(here, 0x20, 'h', r.choice([-1, 0, 1]))
    regions[G + (h - 1) * 0x44] = here
    mine = r.choice([0, 0xb, 0x10, 0x10, 0x19, 7, 1, 3, r.randrange(0x20)])
    kind = r.choice([0, 0xb, 0xb, 0x10, 0x10, 0x19, 7, mine, r.randrange(0x20), r.getrandbits(32)])
    if clear:
        kind = r.choice([0xb, 0x10])
    elif r.random() < 0.15:   # (kind 7 onto a cell it may replace: set, no debit)
        mine, kind = r.choice([0, 0x19]), 7
    regions[TRACK] = track_cell(r, h, mine)
    return Call(CALLS[op], [TRACK, kind], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    regions = dict(objects.case(r).regions)
    regions.update(pathapply.case(r, regions[LOG]).regions)
    things(r, regions)
    cell = bytearray(regions[pathmodel.CELL]) + r.bytes(0x1c)
    h = struct.unpack_from('<H', cell)[0]
    x, y = (h - 1) & 0x7f, (h - 1) >> 7
    owners = [r.randrange(1, 0x3000), r.randrange(1, 0x3000), 0]
    for o in owners:
        oc = bytearray(r.bytes(0x44))
        put(oc, 8, 'I', r.choice([4, 9, 0xa, 0, 0, 1, 3, r.getrandbits(32)]))
        put(oc, 0x24, 'H', r.choice([0, 3, 4, 5, 6, 7]))
        regions[W1 + 0x294 + o * 0x44] = oc
    pairs = bytearray(regions[0x7cdba0]) + bytes(8)
    for at, d in DIAGONALS.items():
        struct.pack_into('<ii', pairs, at, *(d if r.random() < 0.95 else (r.randrange(-1, 2), r.randrange(-1, 2))))
    regions[0x7cdba0] = pairs
    if op in ('SetTrack', 'Reset'):   # (the cells east, south-east and south of the map cell 1 or 0 in half the cases)
        clear = r.random() < 0.5
        patch(r, regions, x, y, owners, 3, {d: r.choice([1, 1, 0]) for d in ((1, 0), (1, 1), (0, 1))} if clear else None)
    else:
        patch(r, regions, x, y, owners)
    price =[0, 1, 50, 99, 100, 101, 0x80000000, r.getrandbits(32)]
    regions.update({0x81d7a8: struct.pack('<I', r.choice([0, 0, 0, 1])), 0x8186d4: bytes([r.choice([0, 0, 1, 0x80])]),
                    0x785a2c: struct.pack('<I', r.choice([0, 0, 1])), 0x820ab8: struct.pack('<I', r.choice([0, 0, 1])),
                    0x81b6dc: struct.pack('<I', r.choice(price)), 0x81ea14: struct.pack('<I', r.choice(price)), 0x7cdb98: r.bytes(4)})
    if h:
        regions[W1 + 0x2d8 + 0x1b0000 + block(h) * 4] = r.bytes(4)
    put(cell, 0x10, 'H', r.choice(owners))
    put(cell, 0x20, 'h', r.choice([-1, 0, 1, 2, r.getrandbits(16) - 0x8000]))
    if op in ('SetTrack', 'ClearBlock', 'Reset'):
        return track_case(r, op, regions, h, owners, op in ('SetTrack', 'Reset') and clear)
    if op == 'SetKind':
        mine = r.choice([0, 1, 1, 3, 3, 4, 9, 0xa, 0x15, 2, r.randrange(0x20)])
        kind = r.choice([0, 1, 1, 3, 3, 4, 9, 0x15, mine, mine, r.randrange(0x20), r.getrandbits(32)])
        if r.random() < 0.3:
            mine, kind = r.choice([(3, 1), (1, 3)])
        put(cell, 8, 'I', mine)
        put(cell, 0xe, 'H', r.choice([0, 0x20, r.getrandbits(16)]))
        args = [kind]
    else:
        put(cell, 8, 'I', r.choice([1, 1, 1, 0xa, 3, 3, 9, 9, 4, 0x15, 2, 0x16, 0, r.getrandbits(32)]))
        put(cell, 0xe, 'H', r.choice([0, 0x20, 0x20, r.getrandbits(16)]))
        args = [r.choice([0, 0, 1, r.getrandbits(32)]), r.choice([0, 0, 1, r.getrandbits(32)])]
    regions[pathmodel.CELL] = cell
    return Call(CALLS[op], [pathmodel.CELL] + args, regions, conv='t', ret='a' if op == 'SetKind' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

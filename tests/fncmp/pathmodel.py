# A path cell's model picked from its neighbours (src/game/world/cellmodel: MapCell_PickPathModel 0x535dd0) against the original: a cell of
# type 3 mostly (its negative, the early-out ones and others), neighbour bits of the shapes the code tells apart (cellmodel.py's), facings
# among those it compares; the two model tables (0x7630b0 / 0x763138, their row counts at 0x700c20 / 0x700c24 now and then 0 or negative)
# laid with rows of those shapes (one of the first four now and then the cell's own bits) whose models are of group 1 (index 2, 0xa or another) or 2 mostly, now and then any or -1; the toggle
# 0x820ac0 and the CRT rand()'s seed (fncmp.RAND_SEED) any. For a group 2 model the sides: the cell's id mostly inside the map (now and then
# at an edge), the direction pairs (0x7cdba0) the real ones mostly, each side's next cell ([0x80239c] + 0x294 + id * 0x44) of type 1 mostly
# with the side's reverse among its bits or not, and the cell's own entry ([0x8023a0] + 0x10ffd8 + id * 0x28) of type 0xc / 0x11 now and
# then (its parent [0x7cf83c] + (its +0x10 + 0x6cde) * 0x28) and low flags (+0xe) or not.
# usage: pathmodel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, RAND_SEED, Bench, Call, put  # noqa: E402
import cellmodel  # noqa: E402

RANGES = ([(0x535dd0, 0x5360fe), (0x522700, 0x5228b5), (0x4d8c00, 0x4d8c4e), (0x4d96f0, 0x4d97b0), (0x4d97b0, 0x4d97e0), (0x4d8300, 0x4d8328), (0x4d97e0, 0x4d9920), (0x4d99c0, 0x4d99e0),
           (0x4d0af0, 0x4d0b27), (0x53ad20, 0x53aefc), (0x5da3c0, 0x5da3c1)])
B = Bench(RANGES)
CELL, OUT = RAM, RAM + 0x1000
W1, W2, G = 0x31000000, 0x33000000, 0x32000000   # ([0x7cf83c], [0x80239c], [0x8023a0])
SIDES = {1: (0, (0, -1)), 0x10: (8, (0, 1)), 4: (0x28, (1, 0)), 0x40: (0x30, (-1, 0))}
REVERSE = {1: 0x10, 0x10: 1, 4: 0x40, 0x40: 4}
TYPES = [3] * 16 + [-3] * 4 + [2, 4, 7, 9, 0xa, 0x15, 0x1e, -2, -0x1e, 0, 1, 1, 5, 0xb, 0x10, 0x19, -0x80000000]


def ident(x, y):
    return ((y << 7) + x + 1) & 0xffff


def model(r):
    return r.choice([1 << 16 | 2, 1 << 16 | 0xa, 1 << 16 | 1, 1 << 16 | 9, 1 << 16 | r.randrange(0x20), 2 << 16 | r.randrange(0x20),
                     2 << 16 | r.randrange(0x20), 2 << 16 | r.randrange(0x20), r.getrandbits(32), 0xffffffff])


def case(r, xy=None):
    """xy: the cell's (x, y) given (pathapply.py's), else drawn."""
    x, y = xy or (r.choice([r.randrange(1, 127)] * 5 + [0, 127]) for _ in range(2))
    h = ident(x, y)
    cell = bytearray(r.bytes(0x28))
    put(cell, 0, 'H', h)
    cell[2] = r.choice([2, 2, 0, 1])
    put(cell, 8, 'i', r.choice(TYPES + [r.getrandbits(32) - (1 << 31)]))
    cell[0xc], cell[0x22] = cellmodel.bits(r), cellmodel.bits(r)
    cell[0xd] = r.choice([1, 4, 0x10, 0x40, 0x14, 0x41, 0x50, 0x05, r.getrandbits(8)])
    if r.random() < 0.4:   # (a corner facing one of the ways the code compares)
        cell[0xd], cell[0xc] = r.choice(cellmodel.FACINGS)
        cell[0x22] = cell[0xc]
    elif r.random() < 0.3:   # (a straight one)
        cell[0xc] = cell[0x22] = r.choice([0x11, 0x44])
    regions = {CELL: cell, OUT: r.bytes(4), 0x81b4cc: struct.pack('<I', r.choice([0, 0, 1, 0x100]))}
    for table, most in ((0x7630b0, 11), (0x763138, 16)):
        data = bytearray(r.bytes(most * 12))
        for i in range(most):
            put(data, i * 12, 'Ii', model(r), r.choice([0, 90, 180, 270, -90, r.getrandbits(16)]))
            data[i * 12 + 8] = r.choice(cellmodel.SHAPES * 2 + [r.getrandbits(8), 0x1f, 0x7c, 0xf1])
        if r.random() < 0.7:   # (a row with the cell's bits: an exact match)
            data[r.randrange(4) * 12 + 8] = r.choice([cell[0xc], cell[0x22]])
        regions[table] = data
    regions[0x700c20] = struct.pack('<ii', *(r.choice([r.randrange(4, most + 1)] * 10 + [r.randrange(1, 4), most, 0, -1]) for most in (11, 16)))
    regions[0x820ac0] = bytes([r.choice([0, 1, 1, 2, 0xff])])
    regions[RAND_SEED] = r.bytes(4)
    pairs = bytearray(0x38)
    for side, (at, d) in SIDES.items():
        struct.pack_into('<ii', pairs, at, *(d if r.random() < 0.95 else (r.randrange(-1, 2), r.randrange(-1, 2))))
    regions[0x7cdba0] = pairs
    regions.update({0x7cf83c: struct.pack('<I', W1), 0x80239c: struct.pack('<I', W2), 0x8023a0: struct.pack('<I', G)})
    for side, (at, _) in SIDES.items():   # (the next cell the pair gives, when inside the map)
        dx, dy = struct.unpack_from('<ii', pairs, at)
        if not (0 <= x + dx < 128 and 0 <= y + dy < 128):
            continue
        nid = ((dy << 7) + h + dx) & 0xffff
        nxt = bytearray(r.bytes(0x44))
        put(nxt, 8, 'I', r.choice([1, 1, 1, 0, 3, r.getrandbits(32)]))
        nxt[2] = r.choice([2, 2, 0, 1])
        for k in (0xc, 0x22):
            nxt[k] = r.getrandbits(8) | REVERSE[side] if r.random() < 0.6 else r.getrandbits(8) & ~REVERSE[side]
        regions[W2 + 0x294 + nid * 0x44] = nxt
    entry = bytearray(r.bytes(0x28))
    parent = r.random() < 0.3
    put(entry, 8, 'I', r.choice([0xc, 0x11]) if parent else r.choice([1, 3, 0xb, 0, r.getrandbits(32)]))
    entry[0xe] = r.choice([0, 0, 0, 1, 8, 0xf]) | r.getrandbits(8) & 0xf0
    regions[G + 0x10ffd8 + h * 0x28] = entry
    if parent:
        p = struct.unpack_from('<H', entry, 0x10)[0]
        pentry = bytearray(r.bytes(0x28))
        pentry[0xe] = r.choice([0, 0, 1, 4, 0xf]) | r.getrandbits(8) & 0xf0
        regions[W1 + (p + 0x6cde) * 0x28] = pentry
    return Call(0x535dd0, [CELL, OUT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

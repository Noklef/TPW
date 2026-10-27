# A map cell's model picked from its neighbours (src/game/world/cellmodel: 0x53a540) against the original: a cell of the picking types mostly
# (3, 0xb, 0x10, 0x19, their negatives, the early-out ones and others), neighbour bits of the shapes the code tells apart (straight, corners,
# junctions, single sides, now and then diagonals or random), facings among those it compares, flags with the low four bits now and then, the
# three model tables (0x762f30, 0x762ff0, 0x763138, their row counts at 0x700c18 / 0x700c1c / 0x700c24 now and then 0 or negative) laid with
# rows of those shapes (their model now and then -1, which sends an exact match on to the second search), the two kept cells (0x7639f0..) on the cell now and then, and the variant counter (0x820aec) and last variant (0x820af0)
# with the world's random state.
# usage: cellmodel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x53a540, 0x53aa88), (0x522700, 0x5228b5), (0x516330, 0x516370), (0x4d8c00, 0x4d8c1f), (0x4d0b10, 0x4d0b30)])
CELL, OUT, WORLD = RAM, RAM + 0x1000, RAM + 0x100000
TABLES = ((0x762f30, 0x700c18, 16), (0x762ff0, 0x700c1c, 27), (0x763138, 0x700c24, 16))
SHAPES = [0x11, 0x44, 0x14, 0x41, 0x50, 0x05, 0x15, 0x45, 0x51, 0x54, 0x55, 1, 4, 0x10, 0x40, 0]
FACINGS = [(0x40, 0x50), (0x10, 0x14), (1, 0x41), (4, 5), (0x40, 0x14), (1, 0x50), (4, 0x41), (0x10, 5)]
TYPES = [3, 0xb, 0x10, 0x19] * 4 + [0x19] * 3 + [4, 5, 7, 9, 0xa, 0xc, 0x11, 0, 1, 2, 6, 0xd, 0x12, 0x1a, -3, -0xb, -0x10, -0x19, -4, -0x80000000]


def bits(r):
    b = r.choice(SHAPES * 3 + [r.getrandbits(8)])
    return b | (r.choice([2, 8, 0x20, 0x80, 0xaa]) if r.random() < 0.15 else 0)


def case(r):
    cell = bytearray(r.bytes(0x28))
    x, y = r.randrange(128), r.randrange(128)
    put(cell, 0, 'H', ((y << 7) + x + 1) & 0xffff if r.random() < 0.95 else r.choice([0, r.getrandbits(16)]))
    cell[2] = r.choice([2, 2, 0, 1])
    put(cell, 8, 'i', r.choice(TYPES + [r.getrandbits(32) - (1 << 31)]))
    cell[0xc], cell[0x22] = bits(r), bits(r)
    cell[0xd] = r.choice([1, 4, 0x10, 0x40, 0x14, 0x41, 0x50, 0x05, 0x14, 0x41, 0x50, r.getrandbits(8)])
    if r.random() < 0.4:   # (a corner facing one of the ways the code compares)
        cell[0xd], cell[0xc] = r.choice(FACINGS)
        cell[0x22] = cell[0xc]
    elif r.random() < 0.25:   # (a straight 0x19 cell without the low flags: the variant)
        put(cell, 8, 'i', 0x19)
        cell[0xc] = cell[0x22] = r.choice([0x11, 0x44])
        put(cell, 0xe, 'H', r.choice([0, 0x10, 0x100]))
    put(cell, 0xe, 'H', r.choice([0, 0, 0, 1, 2, 4, 8, 0xf, 3, 0x10, 0x20, r.getrandbits(16)]))
    regions = {CELL: cell, OUT: r.bytes(4)}
    for table, count, most in TABLES:
        rows = r.randrange(1, most + 1)
        data = bytearray(r.bytes(most * 12))
        for i in range(most):
            put(data, i * 12, 'I', r.choice([(table & 0xf) << 16 | r.randrange(0x20)] * 3 + [r.getrandbits(32), 0xffffffff]))
            put(data, i * 12 + 4, 'i', r.choice([0, 90, 180, 270, 0, 90, 180, 270, -90, r.getrandbits(16)]))
            data[i * 12 + 8] = r.choice(SHAPES * 2 + [r.getrandbits(8), 0x1f, 0x7c, 0xf1])
        regions[table] = data
        regions[count] = struct.pack('<i', r.choice([rows, rows, rows, most, 0, -1]))
    kept = [r.randrange(128) for _ in range(4)]
    if r.random() < 0.5:
        k = r.choice([0, 2])
        kept[k], kept[k + 1] = x, y
    regions[0x7639f0] = struct.pack('<4i', *kept)
    regions[0x81b4cc] = struct.pack('<I', r.choice([0, 0, 1, 0x100]))
    counter = r.choice([r.randrange(0x30), r.randrange(0x30), 0x11, 0x12, 0x17, 0x18, 0x1d, 0x1e, -1, 0x7fffffff])
    variant = 0 if counter < 0x12 else 0x14 if counter < 0x18 else 0x17 if counter < 0x1e else OUT
    regions[0x820aec] = struct.pack('<iI', counter, variant if r.random() < 0.4 else r.choice([0, 0x14, 0x17, 0, 0x14, 0x17, OUT, r.getrandbits(32)]))
    regions[0x7cf83c] = struct.pack('<I', WORLD)
    regions[WORLD + 0x1da708] = r.bytes(4)
    return Call(0x53a540, [CELL, OUT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

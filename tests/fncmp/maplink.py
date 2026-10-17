# A line's cell linked to its neighbours (src/game/world/mapline: 0x539220) against the original: a cell anywhere on the map (mostly away from the
# edges) with the cells around it laid out (rows y - 2 to y + 2, x - 2 to x + 2: the neighbours two away and the cells between), its type (the
# argument, not the cell's: 0xb to 0x19 mostly, the others doing nothing) and the line's step (dx, dy: mostly 0 or +-2), the first-cell flag
# 0x820ac8 with the last steps (0x763b00 / 0x763b04, now and then 0x80), the neighbours' types (the track ones mostly, 0x19 often), neighbour bits
# with a few to all of the eight set, and owners (+0x10) mostly 0 or the focus cell id 0x818698. The direction pairs (0x7cdba0, set at run time)
# are the real ones mostly, now and then another within one cell (so two steps stay among the laid cells); the cell keeps its own id.
# usage: maplink.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x539220, 0x539641), (0x50c320, 0x50c3e0), (0x4d97b0, 0x4d97e0), (0x4d99c0, 0x4d99e0), (0x4d8300, 0x4d8328), (0x522700, 0x5228b5)])
CELLS = RAM + 0x10000
WORLD = (CELLS - 0x6cde * 0x28) & 0xffffffff
STEPS = {0: (0, -1), 8: (0, 1), 0x10: (-1, 1), 0x18: (1, 1), 0x20: (-1, -1), 0x28: (1, 0), 0x30: (-1, 0), 0x38: (1, -1)}
TYPES = [0xb, 0xd, 0x10, 0x12, 0x19]


def ident(x, y):
    return ((y << 7) + x + 1) & 0xffff


def step(r):
    return r.choice([0, 2, -2, 0, 2, -2, 1, -1, 0x80, r.getrandbits(32) - (1 << 31)])


def case(r):
    x0 = r.choice([r.randrange(0, 128), r.randrange(2, 126), r.randrange(2, 126), r.choice([0, 1, 126, 127])])
    y0 = r.choice([r.randrange(0, 128), r.randrange(2, 126), r.randrange(2, 126), r.choice([0, 1, 126, 127])])
    focus = r.getrandbits(16)
    regions = {}
    for j in range(-2, 3):
        lo = ident(x0 - 2, y0 + j)
        row = bytearray(r.bytes(5 * 0x28))
        for i in range(5):
            at = i * 0x28
            put(row, at, 'H', (lo + i) & 0xffff if r.random() < 0.95 else r.getrandbits(16))
            put(row, at + 8, 'I', r.choice(TYPES * 3 + [0x19] * 4 + [0xc, 0x11, 0, 0x1a, 0x10b]))
            row[at + 2] = r.choice([2, 2, 0, 1])
            for k in (0xc, 0x22):
                row[at + k] = r.choice([r.getrandbits(8) & r.getrandbits(8), r.getrandbits(8) & r.getrandbits(8) & r.getrandbits(8), 1 << r.randrange(8), 0, 0,
                                        r.getrandbits(8), 0xff, 0x55])
            put(row, at + 0x10, 'H', r.choice([0, 0, focus, focus, r.getrandbits(16)]))
        if j == 0:   # (the cell's own id: the steps start from it)
            put(row, 2 * 0x28, 'H', ident(x0, y0))
        regions[CELLS + lo * 0x28] = row
    pairs = bytearray(0x40)
    for at, d in STEPS.items():
        struct.pack_into('<ii', pairs, at, *(d if r.random() < 0.95 else (r.randrange(-1, 2), r.randrange(-1, 2))))   # (two steps stay laid out)
    regions[0x7cdba0] = pairs
    regions[0x80239c] = struct.pack('<I', WORLD)
    regions[0x81b4cc] = struct.pack('<I', r.choice([0, 0, 1, 0x100]))
    regions[0x818698] = struct.pack('<I', focus | r.getrandbits(16) << 16)
    regions[0x820ac8] = struct.pack('<I', r.choice([0, 0, 1, 0x100]))
    last = [step(r), step(r)]
    if r.random() < 0.25:   # (one of them 0x80, the other a step)
        last = r.choice([[0x80, r.choice([2, -2])], [r.choice([2, -2]), 0x80]])
    regions[0x763b00] = struct.pack('<ii', *last)
    kind = r.choice(TYPES * 3 + [0x19] * 3 + [0xc, 0xe, 0x11, 0x18, 0x1a, 0, 0xa, r.getrandbits(32)])
    return Call(0x539220, [CELLS + ident(x0, y0) * 0x28, kind, step(r) & 0xffffffff, step(r) & 0xffffffff], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

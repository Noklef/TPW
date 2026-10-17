# The hoardings' coarse grid (src/game/world/hoardinggrid: 0x532960) against the original: areas
# from empty (and negative) to 12 x 12 and a row past the map's 128 columns, the world's cells of
# type 0x19, parented (0xc / 0x11, parents of any id in the table) or neither, the 0x44-byte cells
# they name with flag 0x40 or not.
# usage: hoardinggrid.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x532960, 0x532a50), (0x4d8300, 0x4d8328), (0x4d0b10, 0x4d0b27), (0x535db0, 0x535dc3), (0x53ad20, 0x53aefc)])
CELLS44, TABLE = RAM, RAM + 0x1000  # 16 cells of 0x44 bytes; the world's 0x28-byte cells for ids 0 to 0x680


def case(r):
    if r.random() < 0.1:
        rows, cols = r.choice([1, 2]), r.choice([127, 128, 130])
    else:
        rows, cols = r.choice([0, 1, 2, 3, 5, 12, -1, r.randrange(13)]), r.choice([0, 1, 2, 3, 6, 12, -5, r.randrange(13)])
    last = max(rows - 1, 0) * 128 + max(cols, 0) + 1
    table = bytearray(r.bytes(0x28 * (last + 1)))
    kind = r.choice([0.1, 0.3, 0.6])
    for i in range(last + 1):
        k = 0x28 * i
        put(table, k, 'H', r.randrange(1, 17))
        if r.random() < kind:
            put(table, k + 8, 'I', r.choice([0x19, 0xc, 0x11]))
        put(table, k + 0x10, 'H', r.randrange(last + 1))
    cells = bytearray(r.bytes(16 * 0x44))
    for i in range(16):
        put(cells, 0x44 * i + 0xe, 'H', r.choice([0, 0x40, 0xffff, r.getrandbits(16)]))
    regions = {CELLS44: cells, TABLE: table, 0x81eeb0: r.bytes(0x1100),
               0x81b1b0: put(bytearray(4), 0, 'i', rows), 0x81d740: put(bytearray(4), 0, 'i', cols),
               0x7cf83c: put(bytearray(4), 0, 'I', TABLE - 0x1102b0), 0x8023a0: put(bytearray(4), 0, 'I', CELLS44)}
    return Call(0x532960, [], regions)


if __name__ == '__main__':
    B.main(case)

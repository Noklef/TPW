# The analyser cell type test (src/game/world/analysercell: 0x4fa990) against the original: records
# of any column and row (the cells laid out for the first rows), cells of types about 0, 1, 3, 9, 0xa.
# usage: analysercell.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4fa990, 0x4fa9fa), (0x536300, 0x53644c), (0x4de040, 0x4de100), (0x522770, 0x522790)])
CELLS = RAM + 0x1000


def case(r):
    if r.random() < 0.5:
        obj = bytearray(r.bytes(0x40))
        put(obj, 0x36, 'H', r.choice([1, 2, 3, 0x81, r.randrange(1, 0x300)]))
        cells = bytearray(r.bytes(0x44 * 0x300))
        deltas = bytearray(r.bytes(0x40))
        regions = {RAM: obj, CELLS: cells, 0x8023a0: put(bytearray(r.bytes(8)), 0, 'I', CELLS), 0x7cdba0: deltas, RAM + 0x800: bytearray(16)}
        return Call(0x4de040, [RAM, RAM + 0x800], regions, conv='t')
    rec = bytearray(r.bytes(0x10))
    col, row = r.choice([0, 1, 127, 128, 255, r.randrange(256)]), r.randrange(4)
    rec[5], rec[7] = col, row
    cells = bytearray(r.bytes(0x44 * (4 * 128 + 256)))
    cell = (row << 7) + col
    put(cells, cell * 0x44 + 8, 'I', r.choice([0, 1, 2, 3, 4, 5, 8, 9, 0xa, 0xb, 0x109, r.getrandbits(32)]))
    return Call(0x4fa990, [RAM], {RAM: rec, CELLS: cells, 0x8023a0: put(bytearray(r.bytes(8)), 0, 'I', CELLS)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

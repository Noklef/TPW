# The special cell count of src/game/world/mapcell (Map_CountSpecialCells 0x4cd7f0) against the original: a map of 128 x 128 cells with random types (the first word of
# each cell, taken as a handle by one of the tests, kept small so that the 0x28-byte entries it reaches stay in the region).
# usage: mapcount.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4cd7f0, 0x4cd880), (0x536340, 0x536484), (0x53ad20, 0x53aefc), (0x4d0a70, 0x4d0b27), (0x522700, 0x5228ac)])
TYPES = [0, 1, 2, 3, 4, 5, 7, 9, 0xa, 0xb, 0xc, 0xd, 0x10, 0x11, 0x12, 0x15, 0x19, 0x1e, 0x115, 0x111]


def case(r):
    mem = bytearray(r.bytes(0x10ffd8 + 0x28 * 0x400))
    for h in range(0x400):
        put(mem, 0x10ffd8 + 0x28 * h + 8, 'I', r.choice(TYPES))
        put(mem, 0x10ffd8 + 0x28 * h + 0x10, 'H', r.choice([0, 0, r.randrange(0x400)]))
    for c in range(0x4000):  # (the last cells' fields overlap the first entries: set after them)
        put(mem, 0x44 * c, 'H', r.randrange(0x400))
        put(mem, 0x44 * c + 8, 'I', r.choice(TYPES) if r.random() < 0.95 else r.getrandbits(32))
    world = RAM - 0x2d8
    return Call(0x4cd7f0, [], {0x80239c: put(put(bytearray(8), 0, 'I', world), 4, 'I', RAM), 0x7cf83c: put(bytearray(4), 0, 'I', RAM + 0x10ffd8 - 0x1102b0), RAM: mem}, ret='a')


if __name__ == '__main__':
    B.main(case)

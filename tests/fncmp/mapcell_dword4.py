# The map cell lookup by position of src/game/world/mapcell (0x53bf30 MapCell_Dword4At) against the original: the world [0x7cf83c] placed so
# that the 0x28-byte cells of ids 0-0x8f are one region of random bytes, their types often 0xc / 0x11, their parents (+0x10) 0 or an id there;
# positions on the first rows and ones whose id wraps (x -1, 0xffff, y 0x200).
# usage: mapcell_dword4.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x53bf30, 0x53bfa0), (0x5da3c0, 0x5da3c1)])
CELLS = RAM + 0x1000
WORLD = (CELLS - 0x6cde * 0x28) & 0xffffffff
COUNT = 0x90


def case(r):
    cells = bytearray(r.bytes(COUNT * 0x28))
    for k in range(COUNT):
        put(cells, 0x28 * k + 8, 'I', r.choice([0xc, 0x11, 0xc, 0x11, 0, 1, 0x19, 0x10c, r.getrandbits(32)]))
        put(cells, 0x28 * k + 0x10, 'H', r.choice([0, 0, r.randrange(COUNT), r.randrange(COUNT)]))
    x, y = r.choice([(r.randrange(0x80), 0), (r.randrange(0x0f), 1), (-1, 0), (0xffff, 0), (r.randrange(8), 0x200), (-1, 1)])
    return Call(0x53bf30, [x & 0xffffffff, y & 0xffffffff], {CELLS: cells, 0x7cf83c: struct.pack('<I', WORLD)}, ret='a')


if __name__ == '__main__':
    B.main(case)

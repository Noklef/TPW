# The path step of src/game/core/small66 (0x52a050) against the original: points over and around the first rows of the map (the edges either side of the bounds), the four
# directions and others, cells of types about 0 to 0xb (the few cells a case touches are laid out one by one, so the case stays small).
# usage: small66.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4d8300, 0x4d8340), (0x4d8c00, 0x4d8c30), (0x522700, 0x522740), (0x522770, 0x522790), (0x5227e0, 0x522800), (0x52a050, 0x52a1b0), (0x535600, 0x535680),
           (0x536300, 0x53644c), (0x522850, 0x522860)])
POINT, NX, NY, WORLD = RAM, RAM + 0x100, RAM + 0x104, RAM + 0x10000


def case(r):
    x = r.choice([0, 1, 2, 3, 126, 127, 128, -1, 5, 0x80000000 - 0x80000000])
    y = r.choice([0, 0, 1, 2, -1, 3])
    direction = r.choice([1, 4, 0x10, 0x40, 1, 4, 0x10, 0x40, 0, 2, 8, 0x80, 0xff, 0x11])
    point = put(put(put(bytearray(r.bytes(0x20)), 0, 'ii', x, y), 0x10, 'B', direction), 0x11, 'B', r.getrandbits(8))
    regions = {POINT: point, NX: r.bytes(4), NY: r.bytes(4), 0x7cf83c: struct.pack('<I', WORLD)}
    steps = {(0, 0), (-1, 0), (1, 0), (0, -1), (0, 1)}
    for dx, dy in steps:
        for px, py in ((x + dx, y + dy),):
            ident = (((py & 0xffff) << 7) + px + 1) & 0xffff
            cell = bytearray(r.bytes(0x44))
            put(cell, 8, 'I', r.choice([0, 1, 2, 3, 4, 5, 8, 9, 0xa, 0xb, 0x15]))
            regions[WORLD + 0x294 + ident * 0x44] = cell
    return Call(0x52a050, [POINT, NX, NY], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

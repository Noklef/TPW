# The object's block value test of src/game/world/object (0x4fa770) against the original: object cells over the map, a cell of type 3 or 9 (or not) from which the facing
# neighbours (type 3 cells facing up or right, so that the chain ends) lead to the back, nine offsets (small or wild) into a grid of block values about the limit at +0x198.
# usage: objectblocks.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

RANGES = [(0x4fa770, 0x4fa86f), (0x536320, 0x536336), (0x4de670, 0x4de7e0), (0x4d97e0, 0x4d9920), (0x4d99c0, 0x4d99e0), (0x536300, 0x53644c), (0x522850, 0x522860),
          (0x4d8ca0, 0x4d8cca), (0x5da3c0, 0x5da3c1),
          (0x4d96f0, 0x4d97e0), (0x4d8300, 0x4d8328)]
B = Bench(RANGES)
WORLD = MAP = 0x31000000  # [0x7cf83c] and [0x80239c] are the same world (cells, block grid)
STEPS = {0x7cdba0: (0, -1), 0x7cdba8: (0, 1), 0x7cdbc8: (1, 0), 0x7cdbd0: (-1, 0)}


def case(r):
    obj = bytearray(r.bytes(0x200))
    x, y = (r.choice([0, 1, 5, 64, 120, 126, 127, r.randrange(128), r.randrange(128), 128, 129, 150]) for _ in range(2))
    obj[5], obj[7] = x, y
    regions = {RAM: obj, 0x7cf83c: struct.pack('<I', WORLD), 0x8023a0: struct.pack('<I', WORLD + 0x294 + 0x44), 0x80239c: struct.pack('<I', MAP)}
    pairs = bytearray(r.bytes(0x38))
    for at, d in STEPS.items():
        put(pairs, at - 0x7cdba0, 'ii', *d)
    regions[0x7cdba0] = pairs
    # the cells about the object's: rows y - 7 to y + 1, columns x - 1 to x + 7; type 3 cells facing up (0x10) or right (0x40) among them
    for row in range(y - 7, y + 2):
        first = row * 128 + x - 1 + 1
        if first < 0:
            continue
        cells = bytearray(r.bytes(0x44 * 9))
        for k in range(9):
            put(cells, 0x44 * k, 'H', (first + k) & 0xffff)
            kind = r.choice([3, 3, 3, 9, 0, 1, 4])
            put(cells, 0x44 * k + 8, 'I', kind)
            # a type 3 cell faces up or right (or nothing the walk takes): facing down or left as well could make a loop, which never ends
            cells[0x44 * k + 0xd] = r.choice([0x10, 0x40, 0x10, 0x40, 0, 0x80, 2, 0x50] if kind == 3 else [0x10, 0x40, 1, 4, 0])
        regions[WORLD + 0x294 + first * 0x44] = cells
    limit = r.choice([0, 1, 5, 100, 1000, 0x7fffffff, 0xffffffff, r.getrandbits(8), r.getrandbits(32)])
    put(obj, 0x198, 'I', limit)
    offsets = bytearray(0x48)
    for k in range(9):
        put(offsets, 8 * k, 'ii', *((r.randrange(-2, 3), r.randrange(-2, 3)) if r.random() < 0.9 else (r.getrandbits(32) - (1 << 31), r.randrange(-300, 300))))
    regions[0x7ced88] = offsets
    near = [limit, limit, (limit - 1) & 0xffffffff, (limit + 1) & 0xffffffff, 0, 1, 0xffffffff]
    regions[MAP + 0x2d8 + 0x1b0000] = b''.join(struct.pack('<I', r.choice(near + [r.getrandbits(32)])) for _ in range(33 * 32))
    return Call(0x4fa770, [RAM], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

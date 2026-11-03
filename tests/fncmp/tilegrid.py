# The tiles of a rectangle of a grid (src/game/render/tilegrid: 0x577ae0) against the original, Batch_DrawQuad a recorder (k 1). Its spec is compared by its dwords with its points 0x40 on: not dword 1 (a stack address), 5
# and 11 to 15 (not the spec's). Dword 0 is the polygon flags, stack garbage in the game: the bench's stack starts at 0, as the port's flags do. The recorder also writes into the spec in half the cases, as the real call
# does: the colour at +8 and a point's z, which carry over to the next cell. Up to 32 columns and 64 rows: over 32 rows the tables run on into each other as the original's do. Texture_FormatBits runs as it is, its answer unused.
# usage: tilegrid.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fb, fill, log_region, put  # noqa: E402

B = Bench([(0x577ae0, 0x578058), (0x590c40, 0x590ce8)], hooks={0x565f90: _recorder(1, 2)})
OBJECT, QUADS, HOLDER, EMPTY, TEX, TABLE = (RAM + 0x100 * k for k in range(6))
CELLS = RAM + 0x1000
SPEC = 0xffffffff & ~(1 << 1 | 1 << 5 | 1 << 11 | 1 << 12 | 1 << 13 | 1 << 14 | 1 << 15)
NANS = [0x7fc12345, 0x7f812345]


def place(r, lo, hi, nan=0.0):
    return r.choice(NANS) if r.random() < nan else fb(r.uniform(lo, hi))


def corner(r, cell):
    """A place in a cell: on its edge now and then, or anywhere in it. Under 0 on the edge: a place there truncates to the cell above."""
    return float(cell * 64) if cell < 0 or r.random() < 0.25 else (cell + r.random()) * 64


def case(r):
    log = log_region()
    columns = r.deal([1, 1, 2, 3, 4, 6, 8, 32])
    rows = r.deal([1, 1, 2, 3, 4, 5, 6, 33, 64]) if columns < 6 else r.deal([1, 2])
    x0, y0 = r.randrange(-8, 40), r.randrange(-8, 40)
    x1, y1 = x0 + columns - 1, y0 + rows - 1
    empty = r.deal([0] * 8 + [1, 2])
    if empty == 1:
        x0 = x1 + 1
    elif empty == 2:
        y0 = y1 + 1
    width = columns + r.randrange(0, 3)
    first = r.randrange(0, 3)

    obj = bytearray(r.bytes(0x60))
    put(obj, 0x10, 'i', width)
    put(obj, 0x1c, 'I', r.deal([CELLS] * 14 + [0]))
    put(obj, 0x20, 'ffff', corner(r, x0), corner(r, y0), corner(r, x1), corner(r, y1))
    nan = r.choice([0, 0, 0, 0.1])
    put(obj, 0x30, 'IIII', *(place(r, -500, 500, nan) for _ in range(4)))
    put(obj, 0x40, 'I', r.deal([0xff000000, 0x80000000]) | r.getrandbits(24))
    put(obj, 0x44, 'I', place(r, -2, 2, nan))
    put(obj, 0x48, 'i', first - y0 * width - x0)
    put(obj, 0x5c, 'I', r.deal([0, 1, 0x80000000]))

    count = (rows + 1) * width + first + 2
    cells = bytearray(count * 0x60)
    sticky = r.deal([0, 0, 0.1, 1])
    for k in range(count):
        at = k * 0x60
        cells[at:at + 0x30] = r.bytes(0x30)
        flags = r.getrandbits(32) & ~0x101000
        if r.random() < 0.5:
            flags |= 0x1000
        if r.random() < sticky:
            flags |= 0x100000
        put(cells, at + 8, 'I', flags)
        put(cells, at + 0x18, 'I', r.deal([0, EMPTY, HOLDER, HOLDER]))
        put(cells, at + 0x20, 'IIII', place(r, 0, 2, nan), place(r, 0, 2, nan), place(r, -1, 1, nan), place(r, -1, 1, nan))

    table = [1] * 64
    if r.random() < 0.15:
        table[r.randrange(0, 8)] = 0
    answers(log, 1, TABLE)
    blob(log, 1, 0, SPEC)
    if r.random() < 0.5:
        fill(log, 1, 0, struct.pack('<I', r.getrandbits(32)), offset=8)
        fill(log, 1, 0, struct.pack('<I', fb(r.uniform(-9, 9))), which=1, offset=0x48)
    regions = {LOG: log, OBJECT: obj, QUADS: r.bytes(0x40), EMPTY: bytes(4), HOLDER: struct.pack('<I', TEX),
               TEX: bytes(8) + struct.pack('<ii', r.deal([1, 8, 64, 256, 0]), r.deal([1, 16, 64, 128, 0])), TABLE: struct.pack('<64I', *table), CELLS: cells,
               0x879ab0: struct.pack('<I', QUADS), 0x8bd524: struct.pack('<I', r.deal([4, 4, 1, 0, 2])), 0x8bcbc8: struct.pack('<I', r.deal([0, 2, 0xfffffffd, 0xffffffff]))}
    return Call(0x577ae0, [OBJECT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

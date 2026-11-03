# The water surface drawn into a batch (src/game/terrain/water: 0x570d90) against the original, with the landscape's getters, the format bits, the transform (0x570d20 to 0x579330 in the original,
# the driver's stand-in calls the port of 0x579330) and the clip codes running as they are. Batch_Begin (k 1), Batch_Close (k 2) and ClipPoly_Vertices (k 3, stdcall, its source hashed without each
# vertex's unset +0x14 and what is past +0x20, its count filled in) are recorders. The world: a kept batch moved, or 0-5 rows of cells over columns each row's points cover (every point read was
# written: the rest of the frame's arrays is stack garbage); rows from -1, so that row 0 is the next row; a code table empty, sparse or dense; the vertex and index room run out now and then.
# usage: terrainwater.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fb, fill, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x570d90, 0x572057), (0x67b110, 0x67b13f), (0x585ef0, 0x585ef7), (0x585f00, 0x585f07), (0x590c40, 0x590ca8), (0x570d20, 0x570d52), (0x570d60, 0x570d8a), (0x561bc0, 0x561c3c), (0x579250, 0x579a50)],
          hooks={0x565240: _recorder(1, 5), 0x5658c0: _recorder(2, 1), 0x562a70: _recorder(3, 5, com=True)})
SELF, HEIGHTS, CELLS, VERTS, INDICES = RAM, RAM + 0x3000, RAM + 0x5000, RAM + 0x7000, RAM + 0xb000
GRID, TEX, BATCH, REND, XF, VT, MATRIX = RAM + 0xd000, RAM + 0xd100, RAM + 0xd200, RAM + 0xd300, RAM + 0xd400, RAM + 0xd440, RAM + 0xd480
WIDTH, VERT_ROOM, INDEX_ROOM = 128, 0x4000, 0x1000


def wild(r, x, chance=0.03):
    return r.f() if r.random() < chance else fb(x)


def keep(r, regions):
    """a batch kept from before, top settings bits clear"""
    verts = bytearray(VERT_ROOM)
    r.floats(verts, range(0, 0x100, 4), 100.0)
    batch = bytearray(r.bytes(0x40))
    put(batch, 0x20, 'I', r.choice([0, 1, 2, 5]))
    put(batch, 0x28, 'I', VERTS)
    regions.update({BATCH: batch, VERTS: verts, 0x7014b4: struct.pack('<I', r.choice([0x20, 0x24])),
                    SELF + 0x2958: struct.pack('<I3I', BATCH, wild(r, r.uniform(-50, 50)), wild(r, r.uniform(-50, 50)), wild(r, r.uniform(-50, 50)))})


def case(r):
    log = log_region()
    settings = r.choice([0, 0x40000000, 0x80000000, 0xc0000000]) | r.getrandbits(30)
    stride = r.choice([0x20, 0x20, 0x20, 0x24, 0x28])
    colour = r.getrandbits(32) | 0x01000000
    obj = bytearray(r.bytes(0x240))
    put(obj, 0, 'I', GRID)
    put(obj, 8, 'I', WIDTH)

    grid = bytearray(r.bytes(0x40))
    put(grid, 8, 'ffff', r.uniform(-100, 100), r.uniform(-100, 100), r.choice([1.0, -1.0, r.uniform(-8, 8)]), r.choice([1.0, -1.0, r.uniform(-8, 8)]))
    put(grid, 0x18, 'I', WIDTH)
    put(grid, 0x2c, 'I', CELLS + WIDTH * 4)

    texture = bytearray(r.bytes(0x40))
    put(texture, 8, 'I', r.getrandbits(32) & ~0x31000 | r.choice([0, 0x1000]) | r.choice([0, 0x10000, 0x20000]))
    put(texture, 0x20, 'IIII', wild(r, r.uniform(-2, 2)), wild(r, r.uniform(-2, 2)), wild(r, r.uniform(-2, 2)), wild(r, r.uniform(-2, 2)))

    # The rows, each with its columns; the first row is filled before the loop.
    start = r.choice([0, 0, 1, 2, -1])
    count = r.choice([0, 1, 1, 2, 3, 5])
    left = r.choice([0, 0, 3, 10, 118])
    ranges = {}
    for row in range(start, start + count + 1):
        if r.random() < 0.1:
            ranges[row] = None
            none_first = r.random() < 0.5
            put(obj, row + 0x28, 'B', 0x7f if none_first else left)
            put(obj, row + 0xa8, 'B', r.randrange(0x7f) if none_first else 0xff)
            continue
        first, last = left + r.randrange(2), left + r.randrange(2, 9)
        if start == -1 and row <= 0:
            first = 0  # row -1's cells start at +0x12f, +0x12c's high byte: column 0, so both rows' points from there were written
        ranges[row] = (first, last)
        put(obj, row + 0x28, 'B', first)
        put(obj, row + 0xa8, 'B', last)
    put(obj, 0x128, 'I', start & 0xffffffff)
    put(obj, 0x12c, 'I', (start + count) & 0xffffffff)
    for row in range(start, start + count):
        a, b = ranges[row], ranges[row + 1]
        low, high = 0, 0
        if a and b and r.random() < 0.9:
            low, high = max(a[0], b[0]), min(a[1], b[1])
            if r.random() < 0.3:
                low += r.randrange(2)
            if r.random() < 0.3:
                high -= r.randrange(2)
        if row >= 0:
            put(obj, row + 0x130, 'B', low)
        put(obj, row + 0x1b0, 'B', high)

    heights = bytearray(WIDTH * 4 * 10)
    r.floats(heights, range(0, len(heights), 4), 30.0)
    cells = bytearray(WIDTH * 4 * 10)
    for k in range(0, len(cells), 4):
        put(cells, k, 'I', r.getrandbits(16) | r.getrandbits(16) << 16)

    matrix = bytearray(0x40)
    r.floats(matrix, range(0, 0x40, 4), 2.0)
    density = r.choice([0, 0, 0.1, 0.5, 1])
    table = bytes(r.getrandbits(8) if r.random() < density else 0 for _ in range(0x80))
    records = [r.choice([0, 3, 3, 4, 5, 6]) for _ in range(8)]
    set_return(log, 1, BATCH)
    set_return(log, 2, r.choice([0, 1, 1, 7]))
    mask = 0
    for dword in (0, 1, 2, 3, 4, 6, 7):
        for vertex in range(3):
            mask |= 1 << (dword + vertex * stride // 4)
    blob(log, 3, 2, mask)
    strings(log, 3, 0, out=1 << 3)
    fill(log, 3, 3, struct.pack('<8I', *records), records=8)

    batch = bytearray(r.bytes(0x40))
    put(batch, 0x28, 'II', VERTS, INDICES)
    vert_room = r.choice([VERT_ROOM, VERT_ROOM, VERT_ROOM, (stride << 4) + stride * r.randrange(13)])
    index_room = r.choice([INDEX_ROOM, INDEX_ROOM, INDEX_ROOM, 0x30 + 2 * r.randrange(40)])
    rend = bytearray(r.bytes(0x40))
    put(rend, 0x28, 'II', VERTS + vert_room, INDICES + index_room)
    regions = {LOG: log, SELF: obj, SELF + 0x12970: struct.pack('<I', HEIGHTS + WIDTH * 4 if r.random() > 0.02 else 0),
               SELF + 0x2958: struct.pack('<I3I', r.getrandbits(32) if settings >> 30 else 0, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)),
               HEIGHTS: heights, CELLS: cells, VERTS: r.bytes(VERT_ROOM), INDICES: r.bytes(INDEX_ROOM), GRID: grid, TEX: texture, BATCH: batch, REND: rend,
               XF: struct.pack('<I', VT), VT: struct.pack('<II', 0x12345678, 0x579330), MATRIX: matrix,
               0x8c37c8: struct.pack('<3I', wild(r, r.uniform(-200, 200)), r.getrandbits(32), wild(r, r.uniform(-200, 200))),
               0x8c37f8: struct.pack('<I', TEX if r.random() > 0.02 else 0),
               0x8bd508: struct.pack('<I', settings), 0x8bcbc8: struct.pack('<I', r.getrandbits(32)), 0x7014b4: struct.pack('<I', stride),
               0x8bd4ec: struct.pack('<f', r.uniform(-5, 5)), 0x8bd4bc: struct.pack('<I', colour), 0x8bd524: struct.pack('<I', r.choice([0, 1, 2])),
               0x879ab0: struct.pack('<I', REND), 0x879a20: r.bytes(4), 0x87b088: struct.pack('<I', MATRIX), 0x87b1d8: struct.pack('<I', XF),
               0x879970: table, 0x8c63b0: bytes(0x24)}
    if r.random() < 0.05:
        set_return(log, 1, 0)
    if settings >> 30 == 0 and r.random() < 0.5:
        keep(r, regions)
    return Call(0x570d90, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

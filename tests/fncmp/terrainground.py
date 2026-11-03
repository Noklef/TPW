# The landscape's ground drawn into batches (src/game/terrain/ground: 0x56f670) against the original, with the transform (0x570d20 to 0x579330 in the original, the driver's stand-in calls the port of 0x579330),
# the clip codes, Bytes4_Permute and the records' sort running as they are. Batch_Begin (k 1), Batch_Close (k 2), ClipPoly_Vertices (k 3, stdcall, its source hashed without each vertex's unset +0x14 and what is
# past +0x20, its count filled in), ListNodes_Drain (k 4), Meshes_SetView (k 5), the lighting (k 6), Bands_CollectTextures (k 7, ids and textures filled in), Batch_Fog (k 8), Batch_TransformVertices (k 9), the
# fixed-point pack (k 10) and DrawBatch_Open (k 11) are recorders. Rows' columns are within what both rows wrote and the first row has its points: the rest of the frame's arrays is stack garbage.
# usage: terrainground.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fb, fill, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x56f670, 0x570d11), (0x67b110, 0x67b13f), (0x570d20, 0x570d52), (0x570d60, 0x570d8a), (0x561bc0, 0x561c3c), (0x579250, 0x579a50), (0x56f4f0, 0x56f570), (0x56f570, 0x56f668)],
          hooks={0x565240: _recorder(1, 5), 0x5658c0: _recorder(2, 1), 0x562a70: _recorder(3, 5, com=True), 0x576230: _recorder(4, 0), 0x5743a0: _recorder(5, 2), 0x56ef10: _recorder(6, 0),
                 0x56f380: _recorder(7, 3), 0x565ee0: _recorder(8, 0), 0x565f50: _recorder(9, 1), 0x563130: _recorder(10, 3, cdecl=True), 0x565310: _recorder(11, 5)})
SELF = RAM
HEIGHTS, CELLS, GRID, XF, VT, MATRIX, HOLDER, TABLE = (RAM + 0x4000 + 0x100 * k for k in range(8))
TEXTURES = [RAM + 0x4800 + 0x80 * k for k in range(3)]
BATCHES = [RAM + 0x5000 + 0x80 * k for k in range(4)]
INDICES = [RAM + 0x5400 + 0x480 * k for k in range(4)]
VERTS = RAM + 0x8000
WIDTH, VERT_ROOM = 8, 0x2000
RENDER = 0x12340000


def wild(r, x, chance=0.03):
    return r.f() if r.random() < chance else fb(x)


def texture(r, ids):
    t = bytearray(r.bytes(0x48))
    put(t, 8, 'I', r.getrandbits(32) & ~0x1000 | r.deal([0x1000, 0x1000, 0x1000, 0]))
    put(t, 0x1c, 'I', r.choice(ids))
    put(t, 0x20, 'IIII', *[wild(r, r.uniform(-2, 2)) for _ in range(4)])
    return t


def rows(r, obj):
    """The rows and their columns: a row without points now and then, but for the first; its cells within what both last wrote."""
    start, count = r.deal([0, 0, 1, 2]), r.deal([0, 1, 1, 2, 2, 3])
    if r.random() < 0.03:
        put(obj, 0x128, 'II', start + 1, start)
        return
    put(obj, 0x128, 'II', start, start + count)
    was = None
    for row in range(start, start + count + 1):
        if was and r.random() < 0.15:
            put(obj, row + 0x28, 'B', 0x7f)
            put(obj, row + 0xa8, 'B', r.randrange(0x100))
            now = was
        else:
            now = r.randrange(2), r.randrange(2, WIDTH)
            put(obj, row + 0x28, 'B', now[0])
            put(obj, row + 0xa8, 'B', now[1])
        if was:
            low, high = max(was[0], now[0]), min(was[1], now[1])
            low += r.choice([0, 0, 1])
            high -= r.choice([0, 0, 1])
            put(obj, row - 1 + 0x130, 'B', low)
            put(obj, row - 1 + 0x1b0, 'B', high)
        was = now


def case(r):
    log = log_region()
    stride = r.choice([0x20, 0x20, 0x20, 0x24, 0x28])
    obj = bytearray(r.bytes(0x240))
    put(obj, 0, 'I', GRID)
    put(obj, 8, 'I', WIDTH)
    rows(r, obj)

    grid = bytearray(r.bytes(0x40))
    put(grid, 8, 'ffff', r.uniform(-100, 100), r.uniform(-100, 100), r.choice([1.0, -1.0, r.uniform(-8, 8)]), r.choice([1.0, -1.0, r.uniform(-8, 8)]))
    put(grid, 0x18, 'I', WIDTH)
    put(grid, 0x28, 'I', HEIGHTS)
    put(grid, 0x2c, 'I', CELLS)
    heights = bytearray(WIDTH * 4 * 7)
    r.floats(heights, range(0, len(heights), 4), 30.0)
    cells = bytearray(WIDTH * 4 * 7)
    for k in range(0, len(cells), 4):
        put(cells, k, 'I', r.getrandbits(32) & ~1 | (r.random() < 0.15))

    matrix = bytearray(0x40)
    r.floats(matrix, range(0, 0x40, 4), 2.0)
    density = r.deal([0, 0, 0, 0.1, 0.1, 0.5, 1])
    table = bytes(r.getrandbits(8) if r.random() < density else 0 for _ in range(0x80))

    # The textures: three with an id from few (so runs of one), none for some cells.
    ids = [r.getrandbits(31) | 1 for _ in range(2)] + [0]
    set_return(log, 1, 0 if r.random() < 0.03 else BATCHES[0])
    strings(log, 7, 0, out=7)
    fill(log, 7, 0, struct.pack('<4I', *[r.getrandbits(32) for _ in range(4)]))
    fill(log, 7, 2, struct.pack('<28I', *[r.choice(TEXTURES + TEXTURES + TEXTURES + [0]) for _ in range(28)]))
    strings(log, 5, 0, out=1 << 1)

    # The clipper: its source hashed but for each vertex's unset +0x14 and what is past +0x20, its counts in turn.
    mask = 0
    for dword in (0, 1, 2, 3, 4, 6, 7):
        for vertex in range(3):
            mask |= 1 << (dword + vertex * stride // 4)
    blob(log, 3, 2, mask)
    strings(log, 3, 0, out=1 << 3)
    fill(log, 3, 3, struct.pack('<8I', *[r.choice([0, 0, 3, 3, 4, 4, 5, 0xffffffff]) for _ in range(8)]), records=8)

    set_return(log, 2, r.deal([1, 1, 1, 7, 0]))
    answers(log, 11, TABLE)
    regions = {LOG: log, SELF: obj, SELF + 0x2968: r.bytes(0xc00), HEIGHTS: heights, CELLS: cells, GRID: grid, XF: struct.pack('<I', VT), VT: struct.pack('<II', 0x12345678, 0x579330), MATRIX: matrix,
               HOLDER: r.bytes(0x30), TABLE: struct.pack('<40I', *[0 if r.random() < 0.03 else BATCHES[1 + k % 3] for k in range(40)]), VERTS: r.bytes(VERT_ROOM),
               0x8bd508: struct.pack('<I', r.getrandbits(30) | r.deal([0x40000000, 0x80000000, 0xc0000000, 0x40000000, 0x80000000, 0xc0000000, 0x40000000, 0x80000000, 0xc0000000, 0])),
               0x8bcbc8: struct.pack('<I', r.getrandbits(32) & ~0x2800 | r.deal([0, 0x800]) | r.deal([0, 0x2000])), 0x7014b4: struct.pack('<I', stride), 0x8bd524: struct.pack('<I', r.deal([0, 1, 2, 4])),
               0x879ab0: struct.pack('<I', RENDER), 0x879a20: r.bytes(4), 0x87b088: struct.pack('<I', MATRIX), 0x87b1d8: struct.pack('<I', XF), 0x879970: table, 0x8c63b0: bytes(0x24),
               0x87a6d0: struct.pack('<I', HOLDER), 0x87a248: r.bytes(4), 0x879dec: r.bytes(4)}
    for at in TEXTURES:
        regions[at] = texture(r, ids)
    for at, indices in zip(BATCHES, INDICES):
        batch = bytearray(r.bytes(0x40))
        put(batch, 0x28, 'II', VERTS, indices)
        regions[at] = batch
        regions[indices] = r.bytes(0x480)
    return Call(0x56f670, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

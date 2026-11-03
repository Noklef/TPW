# A panel of tiles (src/game/render/tiledpanel: 0x58a010) against the original, Batch_DrawQuad a recorder (k 1). The points lie before the spec in the frame, and one hash is too short for all: a case hashes the spec's dwords
# (not 1, a stack address, nor 5, unset) or its points' dwords through the spec's pointer: the first three points or the last three.
# usage: tiledpanel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, blob_bytes, fb, log_region, put  # noqa: E402

B = Bench([(0x58a010, 0x58a3bf), (0x590c40, 0x590ce8)], hooks={0x565f90: _recorder(1, 2)})
OBJECT, QUADS, PANEL, TABLE, GLOBALS = (RAM + 0x100 * k for k in range(5))
CELLS = RAM + 0x1000
TILE_HOLDER = RAM + 0x4000
SPEC = 0x7dd
NANS = [0x7fc12345, 0x7f812345]


def place(r, lo, hi, nan=0.0):
    return r.choice(NANS) if r.random() < nan else fb(r.uniform(lo, hi))


def case(r):
    log = log_region()
    nan = r.choice([0, 0, 0, 0.05])
    tile = r.deal([32, 64, 128, 256, 1])
    columns, rows = r.deal([1, 1, 2, 3, 4, 0]), r.deal([1, 1, 2, 3, 4, 0])
    width = max(columns * tile - r.deal([0, 0, 1, tile // 2]), 0)
    height = max(rows * tile - r.deal([0, 0, 1, tile // 2]), 0)
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x10, 'I', r.deal([0, 0x1000000, 0x2000000, 0x4000000, 0x3000000]) | r.deal([0, 0x10000000, 0x20000000, 0x40000000, 0x60000000]) | r.getrandbits(24))
    put(obj, 0x14, 'III', place(r, -2, 2, nan), place(r, -2, 2, nan), place(r, 0, 1, nan))
    put(obj, 0x28, 'II', PANEL, place(r, 0, 3, nan))
    put(obj, 0x30, 'hh', r.randrange(-300, 300), r.randrange(-300, 300))

    count = max(columns, 0) * max(rows, 0) + 1
    cells = bytearray(count * 0x60)
    first_null = r.deal([0] * 9 + [1])
    if first_null:
        count = 1
    for k in range(count):
        at = k * 0x60
        cells[at:at + 0x30] = r.bytes(0x30)
        put(cells, at + 8, 'I', r.getrandbits(32) & ~0x101000 | r.deal([0, 0x1000]) | r.deal([0, 0, 0, 0x100000]))
        put(cells, at + 0x20, 'IIII', place(r, 0, 2, nan), place(r, 0, 2, nan), place(r, -1, 1, nan), place(r, -1, 1, nan))
    panel = bytearray(r.bytes(0x20))
    put(panel, 0, 'I', 0 if first_null else CELLS)
    put(panel, 0xc, 'HHHH', r.deal([1, 16, 256, 640, 0]), r.deal([1, 16, 256, 480, 0]), width, height)

    table = [1] * 64
    if r.random() < 0.15:
        table[r.randrange(0, 6)] = 0
    answers(log, 1, TABLE)
    # A null first tile faults at the next one: one tile only (or none) then.
    if first_null:
        put(panel, 0x10, 'HH', min(width, tile), min(height, tile))
    how = r.deal(['spec', 'first', 'last'])
    if how == 'spec':
        blob(log, 1, 0, SPEC)
    elif how == 'first':
        blob_bytes(log, 1, 0, 0, inner=(4, (1 << 47) - 1))
    else:
        blob_bytes(log, 1, 0, 0, inner=(4, (1 << 47) - 1, 16))
    globs = bytearray(0x18f4)
    put(globs, 0x18f0, 'i', tile)
    regions = {LOG: log, OBJECT: obj, QUADS: r.bytes(0x40), PANEL: panel, TABLE: struct.pack('<64I', *table), CELLS: cells, TILE_HOLDER: globs,
               0x8c4b44: struct.pack('<I', TILE_HOLDER), 0x879ab0: struct.pack('<I', QUADS), 0x8bcbcc: struct.pack('<I', place(r, 0, 2, nan)), 0x8bd524: struct.pack('<I', r.deal([4, 1, 0, 2])),
               0x8bcbd0: struct.pack('<ii', r.deal([640, 800, 1024, 1]), r.deal([480, 600, 768, 1])), 0xfb34a4: r.bytes(4)}
    return Call(0x58a010, [OBJECT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

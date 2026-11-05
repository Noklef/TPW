# Bench: TerrainGround_PrepareTextures 0x56f090: the visible cells' textures kept and touched, or inside the override box numbered into records for the
# tile cache. TexSet_Texture and Texture_SetKeep run as they are. The records are on the stack and those without a texture keep garbage: blob() masks them.
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, blob_array, log_region  # noqa: E402

TOUCH, UPDATE = 1, 2
B = Bench([(0x56f090, 0x56f2eb), (0x67b110, 0x67b13f), (0x56dd60, 0x56dd67), (0x56dd70, 0x56dd77), (0x575f90, 0x575f9d), (0x590f90, 0x590f9a)],
          hooks={0x58c9f0: _recorder(TOUCH, 2), 0x572440: _recorder(UPDATE, 3)})
SELF, MAP, SETS, BLOCK, CELLS = RAM, RAM + 0x3000, RAM + 0x3040, RAM + 0x3100, RAM + 0x8000
SET, TABLE, TEX = RAM + 0x3400, RAM + 0x3500, RAM + 0x3600
MAX_RECORDS = 0x9c4


def view(r, big):
    """The band's rows: first, last, each row's start and end columns; and the map's stride."""
    if big:
        return 0, 64, [0] * 0x80, [64] * 0x80
    first, last = r.randrange(4), r.randrange(7)
    return first, last, [r.choice([0, 1, 2, -1 if y else 0]) for y in range(0x80)], [r.randrange(9) for _ in range(0x80)]


def view_bytes(v):
    return struct.pack('<2I', v[0], v[1]) + struct.pack('<128b', *v[2]) + struct.pack('<128b', *v[3])


def case(r):
    big = r.deal([False] * 39 + [True])
    log = log_region()
    v = view(r, big)
    stride = 64 if big else 8
    rows = 64 if big else 8
    copy = big or r.getrandbits(1)
    flags = r.deal([0, 0x40000000, 0x80000000, 0xc0000000, 0x40000000, 0x80000000]) | (0 if copy else 0x100) | r.getrandbits(32) & 0x3ffffeff
    if big:
        flags |= 0x40000000
    option = (4 if big else r.deal([0, 4, 4])) | r.getrandbits(32) & ~4
    cache = RAM + 0x3800 if big or r.deal([0, 1, 1, 1]) else 0
    box = [0, 0, 127, 127] if big else [r.randrange(-1, 4), r.randrange(-1, 4), r.randrange(2, 9), r.randrange(2, 9)]
    cells = []
    for i in range(stride * rows):
        kept = 0x80 if big or r.getrandbits(1) else 0
        low = kept | r.getrandbits(8) & 0x7e | (0 if big or r.getrandbits(3) else 1)
        ident = 0 if big else r.randrange(2) if kept else r.randrange(4)
        cells.append(low | r.getrandbits(8) << 8 | ident << 16)

    records = []
    for y in range(v[0], v[1]):
        for x in range(v[2][y], v[3][y]):
            cell = cells[y * stride + x]
            if cell & 1 or not (option & 4 and cache):
                continue
            if box[0] <= x <= box[2] and box[1] <= y <= box[3] and len(records) < MAX_RECORDS:
                records.append(cell & 0x80)
    if big:
        blob_array(log, UPDATE, 1, 2, 0x14)
    else:
        mask = 0
        for i, kept in enumerate(records[:6]):
            mask |= (0x1f if kept else 0x11) << (5 * i)
        blob(log, UPDATE, 1, mask)

    shown = view_bytes(v)
    self_bytes = struct.pack('<2I', MAP, SETS) + r.bytes(0x18) + r.bytes(0x108) + (r.bytes(len(shown)) if copy else shown)
    self_bytes += struct.pack('<5I', cache, *[b & 0xffffffff for b in box]) + r.bytes(4) + r.bytes(0x4 * (MAX_RECORDS + 1) if big else 0x100)
    block = (shown if copy else r.bytes(len(shown))) + r.bytes(len(shown))
    regions = {LOG: log, SELF: self_bytes, SELF + 0x1296c: r.bytes(4), 0x8bd508: struct.pack('<I', flags), 0x879d54: struct.pack('<I', BLOCK), BLOCK + 0xc4: block,
               0x8bd554: struct.pack('<I', r.randrange(8)), 0x7014c8: r.bytes(0x20), 0x8bd550: struct.pack('<I', option), 0x8c4b44: r.bytes(4),
               MAP: r.bytes(0x18) + struct.pack('<I', stride) + r.bytes(0x10) + struct.pack('<I', CELLS), CELLS: struct.pack('<%dI' % len(cells), *cells),
               SETS: b''.join(r.bytes(4) + struct.pack('<I', s) for s in (SET, SET + 0x10, 0, SET + 0x20)),
               SET: b''.join(r.bytes(8) + struct.pack('<I', TABLE + 4 * k) + r.bytes(4) for k in range(3)),
               TABLE: struct.pack('<3I', TEX, TEX + 0x20, 0),
               TEX: b''.join(r.bytes(0x10) + struct.pack('<2h', r.randrange(-300, 300), r.randrange(-300, 300)) + r.bytes(12) for _ in range(2))}
    return Call(0x56f090, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

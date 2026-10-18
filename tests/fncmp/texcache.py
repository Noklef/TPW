# The terrain texture tile cache's constructor and destructor (src/game/terrain/texcache: 0x5720d0, 0x572350; its jump 0x572100,
# core/thunks) against the originals: any
# bytes before; for the destructor 0-3 tiles in use (or a negative count) whose texture arrays (as texturemem.py) are set or not, the
# landscape at +0 set (as land_levels.py's destructor) or not.
# The procedural texture object's unload (src/game/terrain/procedural: 0x56e4e0): its cache as the destructor's or none, a tracked
# block at +0x12970 or none (the tracked blocks' table as in spritesfree.py). Destroy (0x56de90): the same object at [0x879d60], or none.
# AddRect (0x56e030): a header of width / height 0-6 (or past the grid), a grid of random floats, a rectangle around and past it, any delta.
# usage: texcache.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Dtor,Jump,Unload,Destroy,AddRect)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

CALLS = {'Ctor': 0x5720d0, 'Dtor': 0x572350, 'Jump': 0x572100, 'Unload': 0x56e4e0, 'Destroy': 0x56de90, 'AddRect': 0x56e030}
B = Bench([(0x5720d0, 0x572105), (0x572350, 0x5723ef), (0x55f1e0, 0x55f730), (0x5c4f60, 0x5c4f90), (0x5d3860, 0x5d38a0), (0x590a70, 0x590a75),
           (0x590b90, 0x590c39), (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1),
           (0x56e4e0, 0x56e549), (0x56de90, 0x56df67), (0x56e030, 0x56e095)])
CACHE, LND, TILES, ARRAYS = RAM + 0x10000, RAM + 0x4000, RAM + 0x4400, RAM + 0x5000   # (texturemem.py's regions below RAM + 0x4000)
TEX, COUNT, TABLE = RAM + 0x20100, 0x8bb214, 0x87b1f8


def dtor(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    cache = bytearray(r.bytes(0x10018))
    n = r.choice([0, 1, 2, 3])
    count = r.choice([n, n, n, 0, 0xffffffff]) if n else r.choice([0, 0, 0xffffffff])
    put(cache, 0x10010, 'I', count)
    put(cache, 0x10008, 'I', TILES)
    tiles = bytearray(r.bytes(0x40))
    for k in range(n):
        if r.random() < 0.8:
            m = r.choice([0, 1, 2, 3])
            base = ARRAYS + 0x400 * k
            tex = texturemem.textures(r, regions, base + 4, m + 1)
            regions[base] = struct.pack('<I', m) + tex
            put(tiles, 0x10 * k + 0xc, 'I', base + 4)
        else:
            put(tiles, 0x10 * k + 0xc, 'I', 0)
    regions[TILES] = tiles
    if r.random() < 0.5:   # a landscape (land_levels.py's destructor case)
        lnd = bytearray(r.bytes(0x300))
        for off in [0x14] + [0x24 + 0xd8 * k + off for k in range(3) for off in range(8, 0xd8, 0x10)]:
            put(lnd, off + 0xc, 'I', r.choice([0, 0, HEAP + 0x2000 + off]))
        regions[LND] = lnd
        regions[0x8798c8] = struct.pack('<I', r.choice([0, HEAP + 0x1f00]))
        put(cache, 0, 'I', LND)
    else:
        put(cache, 0, 'I', 0)
    regions[CACHE] = cache
    regions[0x8bd550] = r.bytes(4)
    return Call(0x572350, [CACHE], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'AddRect':
        tex = bytearray(r.bytes(0x10))
        hdr = bytearray(r.bytes(0x20))
        w, h = r.choice([0, 1, 2, 3, 6]), r.choice([0, 1, 2, 4, 6])
        put(hdr, 0x18, 'I', r.choice([w, w, w, 0xffffffff]))
        put(hdr, 0x1c, 'I', r.choice([h, h, h, 0x7fffffff]))
        grid = bytearray(r.bytes(4 * 0x84))   # (from 0x10 bytes before the grid: a column -1 written there shows)
        for k in range(0x84):
            if r.random() < 0.8:
                put(grid, 4 * k, 'I', r.f())
        put(tex, 0, 'I', RAM + 0x100)
        regions = {RAM: tex, RAM + 0x100: hdr, RAM + 0x1f0: grid, RAM + 0x12970: struct.pack('<I', RAM + 0x200)}
        if r.random() < 0.05:
            put(tex, 0, 'I', 0x80000000 | r.getrandbits(31))   # (a bad header: a fault in both for rows in range)
        x0, y0 = r.choice([-2, -1, 0, 0, 1, 2, 3]), r.choice([-2, -1, 0, 0, 1, 2])
        x1, y1 = x0 + r.choice([-1, 0, 1, 2, 3, 5, 8]), y0 + r.choice([-1, 0, 1, 2, 4, 7])
        rect = [v & 0xffffffff for v in (x0, y0, x1, y1)]
        return Call(0x56e030, [RAM] + rect + [r.f()], regions, conv='t')
    if op in ('Unload', 'Destroy'):
        c = dtor(r)
        blocks = [HEAP + 0x20, HEAP + 0x40, HEAP + 0x3c]
        table = bytearray(8 * 8)
        for k in range(8):
            b = r.choice(blocks + [r.getrandbits(32) | 0x80000000])
            put(table, 8 * k, '2I', b, r.choice([b, b, b + 4]) & 0xffffffff)
        tex = bytearray(r.bytes(0x234))
        put(tex, 0x230, 'I', r.choice([CACHE, CACHE, CACHE, 0]))
        c.regions.update({TEX: tex, TEX + 0x12970: struct.pack('<I', r.choice([0] + blocks + [r.getrandbits(32) | 0x80000000])),
                          TABLE: table, COUNT: struct.pack('<I', r.choice([0, 1, 2, 3, 5]))})
        c.address, c.args = 0x56e4e0, [TEX]
        if op == 'Destroy':
            c.regions[0x879d60] = struct.pack('<I', r.choice([TEX, TEX, TEX, 0]))
            return Call(0x56de90, [], c.regions, ret='a')
        return c
    if op != 'Ctor':
        c = dtor(r)
        c.address = CALLS[op]
        return c
    return Call(0x5720d0, [RAM], {RAM: r.bytes(0x10018)}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

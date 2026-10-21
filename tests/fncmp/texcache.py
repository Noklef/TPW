# The terrain texture tile cache's constructor and destructor (src/game/terrain/texcache: 0x5720d0, 0x572350; its jump 0x572100,
# core/thunks) against the originals: any
# bytes before; for the destructor 0-3 tiles in use (or a negative count) whose texture arrays (as texturemem.py) are set or not, the
# landscape at +0 set (as land_levels.py's destructor) or not.
# The procedural texture object's unload (src/game/terrain/procedural: 0x56e4e0): its cache as the destructor's or none, a tracked
# block at +0x12970 or none (the tracked blocks' table as in spritesfree.py). Destroy (0x56de90): the same object at [0x879d60], or none.
# AddRect (0x56e030): a header of width / height 0-6 (or past the grid), a grid of random floats, a rectangle around and past it, any delta.
# Stamp (0x56e0a0): widths and heights about the fixed rectangles' edges (0x1c-0x50 by 0-0x11), the grid as AddRect's.
# SetView (0x56e6c0) / Setup (0x56e3f0): a header of floats (ordinary or specials; the span +0x20..+0x24 zero or a NaN now and then) and
# sizes 0-24 or the stamps' (wrapping ones now and then), the world view [0x879d54] random, the renderer [0x87a6d0] as trackedglobal.py's
# frame buffer case; Setup's land holds the header, the render flag 0x10000 set or not, the old block tracked or none, the new one's word
# count (+4) the grid's or not, and its source at +0x28.
# usage: texcache.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Dtor,Jump,Unload,Destroy,AddRect,Stamp,SetView,Setup,Init)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, fb, global_heap, heap, log_region, put  # noqa: E402

CALLS = {'Ctor': 0x5720d0, 'Dtor': 0x572350, 'Jump': 0x572100, 'Unload': 0x56e4e0, 'Destroy': 0x56de90, 'AddRect': 0x56e030,
         'Stamp': 0x56e0a0, 'SetView': 0x56e6c0, 'Setup': 0x56e3f0, 'Init': 0x572110}
RANGES = [(0x5720d0, 0x572105), (0x572350, 0x5723ef), (0x55f1e0, 0x55f730), (0x5c4f60, 0x5c4f90), (0x5d3860, 0x5d38a0), (0x590a70, 0x590a75),
           (0x590b90, 0x590c39), (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1),
           (0x56e4e0, 0x56e549), (0x56de90, 0x56df67), (0x56e030, 0x56e095), (0x56e0a0, 0x56e3e2), (0x56e3f0, 0x56e4d4), (0x56e6c0, 0x56e77b),
           (0x56b830, 0x56b841), (0x56b850, 0x56b93e), (0x576310, 0x5763e1), (0x56af20, 0x56af9c),
          (0x572110, 0x57234c), (0x55f040, 0x55f1e0), (0x590a00, 0x590a70), (0x590a80, 0x590b88), (0x67d830, 0x67d8f0),
          (0x67acf0, 0x67ada0)]
B = Bench(RANGES)
HDR, VIEW, RENDERER, GRID, LANDOBJ, SOURCE = RAM + 0x100, RAM + 0x400, RAM + 0x800, RAM + 0x1000, RAM + 0x3000, RAM + 0x3400
CACHE, LND, TILES, ARRAYS = RAM + 0x10000, RAM + 0x4000, RAM + 0x4400, RAM + 0x5000   # (texturemem.py's regions below RAM + 0x4000)
TEX, COUNT, TABLE = RAM + 0x20100, 0x8bb214, 0x87b1f8


def init(r):
    """0x572110: 0-4 tiles of a size 0 (the cache's +4), 0x10-0x40 or odd, the video and texture memory about the limits (any format),
    the renderer kind 4 now and then; the tracked blocks' table as trackedglobal.py's (a few in use); the heap failing one of the
    allocations now and then (the landscape's, the tiles', a texture block's or its pixels': the error pushed and the destructor run)."""
    count = r.choice([0, 1, 2, 3, 4, 4])
    size = r.choice([0, 0, 0x40, 0x40, 0x20, 0x10, 8, 1, 0x30])
    cache = put(bytearray(r.bytes(0x10018)), 4, 'I', r.choice([0x40, 0x40, 0x20, 0x10, 0x30, 2]))
    regions = {CACHE: cache, HEAP: heap(0x30000, fail=r.choice([0] * 10 + [1, 2, 3, 4, 5, 6])), **global_heap(), LOG: log_region()}
    table = bytearray(0x18 + 8 * 12)
    used = r.choice([0, 0, 1, 3])
    for k in range(used):
        put(table, 0x18 + 8 * k, 'II', HEAP + 0x2fff0, 4)
    regions[0x87b1e0] = bytes(table)
    regions[COUNT] = struct.pack('<I', used) + r.bytes(4)
    errors = put(bytearray(r.bytes(0x204)), 0x200, 'i', r.choice([r.randrange(0x40), 0, 0x3f, 0x40]))
    regions[0x879ad0] = errors
    regions[0x8bd508] = struct.pack('<I', r.getrandbits(32) & ~0x1000000)
    regions[0x8bd520] = struct.pack('<iI', r.choice([0x2000000, 0x2ffffff, 0x3000000, 0x3800000, 0x3ffffff, 0x4000000, 0x8000000, -1]),
                                    r.choice([0, 1, 2, 3, 4, 4, 5]))   # (+4: the renderer kind)
    regions[0x8bd550] = r.bytes(4) + struct.pack('<I', r.randrange(4))   # (+4: the format)
    regions[0x8c1624] = struct.pack('<i', r.choice([0, 0x100000, 0x300000, 0x5fffff, 0x600000, 0x900000, 0xa00000, 0x1000000, 0x2000000, -1]))
    regions[0x8798c8] = bytes(4)
    return Call(0x572110, [CACHE, count, size], regions, conv='t', ret='a')


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


def fl(r):
    return r.choice([fb(r.uniform(-500.0, 500.0)), fb(r.uniform(0.01, 8.0)), fb(float(r.randrange(-64, 65))), r.f()])


def grid_floats(r, n):
    grid = bytearray(r.bytes(4 * n))
    for k in range(n):
        if r.random() < 0.8:
            put(grid, 4 * k, 'I', r.choice([fl(r), r.f()]))
    return grid


def view_case(r, op):
    """SetView / Setup: the header, the world view and the renderer (trackedglobal.py's frame buffer case)."""
    regions = {HEAP: heap(0x8000, r.choice([0] * 9 + [1, 2])), **global_heap()}
    hdr = bytearray(r.bytes(0x30))
    for off in (8, 0xc, 0x10, 0x14, 0x20, 0x24):
        put(hdr, off, 'I', fl(r))
    roll = r.random()
    if roll < 0.1:
        put(hdr, 0x24, 'I', struct.unpack_from('<I', hdr, 0x20)[0])
    elif roll < 0.2:
        put(hdr, r.choice([0x20, 0x24]), 'I', r.choice([0x7fc00000, 0xffc00000, 0x7fa00000, 0xff812345]))
    w, h = r.randrange(25), r.randrange(25)
    if r.random() < 0.4:
        w, h = r.choice([0x1d, 0x2c, 0x30, 0x34, 0x45, 0x50]), r.choice([0xb, 0xc, 0x10, 0x11])
    if r.random() < 0.05:
        w, h = r.choice([(0x40000001, 3), (0xffffffff, 2), (3, 0x80000000)])
    put(hdr, 0x18, '2I', w, h)
    regions[HDR] = hdr
    regions[VIEW] = r.bytes(0x2d8)
    regions[0x879d54] = struct.pack('<I', VIEW)
    errors = bytearray(r.bytes(0x204))
    put(errors, 0x200, 'i', r.choice([r.randrange(0x40), 0, 0x3f, 0x40]))
    regions[0x879ad0] = errors
    regions[0x8bd508] = struct.pack('<I', r.getrandbits(32) & ~0x1000000)
    regions[0x87afe8] = r.bytes(4)
    ren = bytearray(r.bytes(0x80))
    put(ren, 0x10, 'I', r.choice([0, HEAP + 0x7000]))
    put(ren, 0x70, 'i', r.choice([-1, 0, 1, 2, 3, 4, 5]))
    for k in range(4):
        if r.random() < 0.3:
            put(ren, 0x60 + 4 * k, 'I', RENDERER + 0x48)
    regions[RENDERER] = ren
    regions[0x87a6d0] = struct.pack('<I', RENDERER)
    table = bytearray(r.bytes(0x18 + 8 * 8))
    for k in range(8):
        b = r.choice([HEAP + 0x7000, HEAP + 0x7100, r.getrandbits(32) | 0x80000000])
        put(table, 0x18 + 8 * k, '2I', b, r.choice([b, b, b + 4]) & 0xffffffff)
    regions[TABLE - 0x18] = table
    regions[COUNT] = struct.pack('<I', r.choice([0, 1, 2, 3, 5, 8]))
    tex = bytearray(r.bytes(0x20))
    put(tex, 0, 'I', HDR)
    regions[RAM] = tex
    if op == 'SetView':
        return Call(0x56e6c0, [RAM], regions, conv='t', ret='a')
    regions[RAM + 0x12968] = put(bytearray(r.bytes(0xc)), 8, 'I', r.choice([0, HEAP + 0x7000, HEAP + 0x7100]))
    regions[0x8bcbc8] = struct.pack('<I', r.getrandbits(32) ^ r.choice([0, 0x10000]))
    cells = (w + 1) * (h + 1) if w < 0x100 and h < 0x100 else 4
    put(hdr, 4, 'H', r.choice([cells, cells, cells, 0, 1, r.randrange(0x400)]))
    put(hdr, 0x28, 'I', SOURCE)
    regions[SOURCE] = grid_floats(r, 0x800)
    land = bytearray(r.bytes(0x80))
    put(land, 0x6c, 'I', HDR)
    regions[LANDOBJ] = land
    return Call(0x56e3f0, [RAM, LANDOBJ], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Init':
        return init(r)
    if op in ('SetView', 'Setup'):
        return view_case(r, op)
    if op == 'Stamp':
        w = r.choice([0x10, 0x1c, 0x1d, 0x1e, 0x2c, 0x2d, 0x30, 0x34, 0x41, 0x43, 0x44, 0x45, 0x50])
        h = r.choice([0, 4, 5, 8, 0xa, 0xb, 0xc, 0x10, 0x11])
        hdr = bytearray(r.bytes(0x20))
        put(hdr, 0x18, '2I', w, h)
        tex = bytearray(r.bytes(0x10))
        put(tex, 0, 'I', HDR)
        return Call(0x56e0a0, [RAM], {RAM: tex, HDR: hdr, GRID - 0x10: grid_floats(r, (w + 1) * (h + 1) + 8),
                                      RAM + 0x12970: struct.pack('<I', GRID)}, conv='t')
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

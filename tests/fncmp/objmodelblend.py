# An engine object's model textures blended with the terrain under it (src/game/model/objmodel: 0x57ace0 ObjModel_BlendTerrainTiles)
# against the original, its callees run: terraintexcache_tile.py's cache (+0 terraintile.py's Lnd of 2-3 x 2-3 tiles, +4 its tile size, 4 or 8
# here) at [[0x879d60]+0x230], beside the map at [[0x879d60]] (+0x18 width, +0x2c cells) of 2-5 x 2-4 cells; a rectangle in it (now and then
# empty, or from -1) with 0-3 cells flagged 0x80 and 0x200 (only on the Lnd's tiles; others with one of the two), their high words picking
# one of 1-3 entries of the model's set table (+8 -> +0x50), now and then the same one twice (the second gets the first's new set); each
# set's first texture plain (no surface), now and then flipped (0x40000), 1-16 x 1-16 pixels, owned by a list that holds it or not, under
# the texture cache [0x8c4b44] with its flag 0x20000000 or not; the heap failing one of the first ten allocations now and then (the
# pairs', the sets', a texture's, its pixels', a set's list), and now and then answering 0 to the first two of a rectangle with no
# flagged cell (the heap's next block 0: the pairs and sets both none). The pixel format [0x8bd554] 0-3.
# usage: objmodelblend.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import terraintile  # noqa: E402
terraintile.SIZES[:] = [4, 4, 4, 8]   # (before terraintexcache_tile sets its own)
import terraintexcache_tile as tile  # noqa: E402
import texcache  # noqa: E402
import texset  # noqa: E402
import wctimage  # noqa: E402
from fncmp import HEAP, RAM, Bench, Call, _cdecl_log, fb, heap, put  # noqa: E402

B = Bench(wctimage.merged([(0x55f780, 0x560ff0), (0x55e780, 0x55eaa0), (0x55eaa0, 0x55f060), (0x5e7840, 0x5e78b0), (0x572630, 0x572702),
                           (0x590a80, 0x590b88), (0x579b40, 0x579c13), (0x56af20, 0x56af9c), (0x590d40, 0x590f7e), (0x562f70, 0x563099),
                           (0x56a770, 0x56ae60), (0x67a400, 0x690000), (0x57ace0, 0x57b06c), (0x57b070, 0x57b130), (0x575c00, 0x575c5b),
                           (0x575bf0, 0x575bf5)] + texset.RANGES + texcache.RANGES), hooks={0x564650: _cdecl_log})
ENGINE, MAP, CELLS = RAM + 0x19000, RAM + 0x19300, RAM + 0x19400
OBJ, MODEL, TABLE, SETS, LISTS, TEXTURES, OWNERS = (RAM + o for o in (0x1a000, 0x1a200, 0x1a300, 0x1a400, 0x1a500, 0x1a600, 0x1a800))
PIXELS, TEXCACHE = RAM + 0x1b000, RAM + 0x1c000


def case(r):
    c = tile.case(r)
    regions = c.regions
    del regions[tile.TEXTURE]
    tx, ty = struct.unpack_from('<2I', regions[terraintile.LND], 0xc)
    w, h = r.choice([tx, tx + 1, 4, 5]), r.choice([ty, ty + 1, 3, 4])
    x0, y0 = r.randrange(w) if r.random() < 0.95 else -1, r.randrange(h) if r.random() < 0.95 else -1
    x1, y1 = r.randrange(max(x0, 0), w), r.randrange(max(y0, 0), h)
    if r.random() < 0.05:
        x1, y1 = (x0 - 1, y1) if r.random() < 0.5 else (x1, y0 - 1)
    kinds = r.choice([1, 1, 2, 3])
    both_none = r.random() < 0.04
    inside = [(x, y) for y in range(max(y0, 0), y1 + 1) for x in range(max(x0, 0), x1 + 1) if x < tx and y < ty]
    flagged = set() if both_none else set(r.sample(inside, min(len(inside), r.choice([0, 1, 1, 2, 2, 3]))))
    cells = bytearray(r.bytes(4 * w * h))
    for y in range(h):
        for x in range(w):
            v = r.randrange(kinds) << 16 | r.getrandbits(16)
            if (x, y) in flagged:
                v |= 0x280
            elif v & 0x280 == 0x280 or r.random() < 0.3:
                v = v & ~r.choice([0x80, 0x200, 0x280]) | r.choice([0x80, 0x200, 0])
                if v & 0x280 == 0x280:
                    v &= ~0x80
            put(cells, 4 * (w * y + x), 'I', v)
    regions[CELLS] = cells
    regions[MAP] = put(put(bytearray(r.bytes(0x40)), 0x18, 'I', w), 0x2c, 'I', CELLS)
    regions[ENGINE] = put(put(bytearray(r.bytes(0x240)), 0, 'I', MAP), 0x230, 'I', tile.CACHE)
    regions[0x879d60] = struct.pack('<I', ENGINE)

    obj = put(bytearray(r.bytes(0x140)), 8, 'I', MODEL)
    regions[OBJ] = obj
    regions[MODEL] = put(bytearray(r.bytes(0x60)), 0x50, 'I', TABLE)
    table, sets, lists, textures, owners = (bytearray(r.bytes(n)) for n in (0x20, 0x40, 0x40, 0x120, 0x40))
    side = struct.unpack_from('<I', regions[tile.CACHE], 4)[0]
    for k in range(kinds):
        put(table, 8 * k + 4, 'I', SETS + 0x10 * k)
        put(sets, 0x10 * k + 8, 'IHH', LISTS + 0x10 * k, r.choice([1, 2, 0x8001]), r.choice([1, 2, 3]))
        put(lists, 0x10 * k, 'I', TEXTURES + 0x60 * k)
        flags = r.getrandbits(32) & ~0x10000000
        flags = flags | 0x40000 if r.random() < 0.3 else flags & ~0x40000
        put(textures, 0x60 * k + 8, 'I', flags)
        put(textures, 0x60 * k + 0x10, 'hh', min(16, r.choice([side, side * 2, 1, 3])), min(16, r.choice([side, side * 2, 1, 2])))
        put(textures, 0x60 * k + 0x18, 'I', OWNERS + 0x10 * k)
        put(textures, 0x60 * k + 0x30, 'I', fb(r.uniform(0, 1.5)))
        put(textures, 0x60 * k + 0x40, 'I', PIXELS + 0x400 * k)
        put(textures, 0x60 * k + 4, 'I', 0)
        put(owners, 0x10 * k + 4, 'I', TEXTURES + 0x60 * k if r.random() < 0.8 else 0)
    for at, data in ((TABLE, table), (SETS, sets), (LISTS, lists), (TEXTURES, textures), (OWNERS, owners)):
        regions[at] = data
    regions[PIXELS] = r.bytes(0x400 * kinds)
    regions[TEXCACHE] = put(bytearray(r.bytes(0x1900)), 0x18cc, 'I',
                            r.getrandbits(32) | 0x20000000 if r.random() < 0.3 else r.getrandbits(32) & ~0x20000000)
    regions[0x8c4b44] = struct.pack('<I', TEXCACHE)
    regions[HEAP] = heap(0x10000, fail=r.choice([0] * 12 + list(range(1, 11))))
    if both_none:   # (the next block 0: the first allocations answer 0, and with no cell flagged they ask for 0 bytes)
        put(regions[HEAP], 0, 'I', 0)
    return Call(0x57ace0, [OBJ, x0 & 0xffffffff, y0 & 0xffffffff, x1 & 0xffffffff, y1 & 0xffffffff], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

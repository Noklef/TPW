# A terrain texture cache tile's blend (src/game/terrain/texcache: 0x572630) against the original, with its callees run: terraintile.py's Lnd,
# weight maps, ramps and overlay, the cache's +0 that Lnd and +4 its tile size; a texture for Texture_Alloc (its pixels a tracked global
# block from the heap, failing now and then; the tracked count 0-5), the formats 0-3 at 0x8bd554 (a 2-byte one through the scratch at
# 0x879a68 and TexFormat_ConvertSquare, else drawn in place and each pixel's bytes reversed; 4 and on are 0x100-byte pixels, past the
# heap); the engine's error list at 0x879ad0 for a failed allocation. The tile sizes dealt small mostly, 32 at most (a 32 tile costs four 16s; the
# blend and its 64 tiles are terraintile.py's), unless SIZES is given.
# usage: terraintexcache_tile.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import terraintile  # noqa: E402
if not terraintile.SIZES:
    terraintile.SIZES[:] = [4] * 4 + [8] * 12 + [16] * 4 + [24, 32]
from fncmp import HEAP, RAM, Bench, Call, _cdecl_log, global_heap, heap, put  # noqa: E402

B = Bench([(0x55f780, 0x560ff0), (0x55e780, 0x55eaa0), (0x55eaa0, 0x55f060), (0x5e7840, 0x5e78b0),   # (terraintile.py's, its 0x67acb0 below)
           (0x572630, 0x572702), (0x590a80, 0x590b88), (0x579b40, 0x579c13), (0x56af20, 0x56af9c), (0x590d40, 0x590f7e),
           (0x562f70, 0x563099), (0x56a770, 0x56ae60), (0x67a400, 0x690000)], hooks={0x564650: _cdecl_log})
CACHE, TEXTURE, SCRATCH2 = RAM + 0x18000, RAM + 0x18100, RAM + 0x40000


def case(r):
    c = terraintile.case(r)
    regions = c.regions
    lnd, out, x, y, overlay = c.args
    side = regions[lnd][0]
    regions[HEAP] = heap(0x10000, fail=r.choice([0] * 12 + [1]))
    regions.update(global_heap())
    del regions[out]
    regions[CACHE] = put(bytearray(r.bytes(0x10)), 0, '2I', lnd, side)
    texture = bytearray(r.bytes(0x60))
    put(texture, 8, 'I', r.getrandbits(32) & ~0x10000000)
    put(texture, 0x30, 'f', r.choice([0.0, 0.5, 1.0, 1.5, r.random() * 1.5]))
    regions[TEXTURE] = texture
    regions[0x8bd554] = struct.pack('<I', r.choice([0, 1, 2, 3, 3, 3]))   # (pixel sizes 2, 2, 2, 4 at 0x7024e0 and 0x701548; 4 on: 0x100)
    regions[0x879a68] = struct.pack('<I', SCRATCH2)
    regions[SCRATCH2] = r.bytes(side * side * 4)
    regions[0x8bb214] = struct.pack('<I', r.randrange(0, 6))
    regions[0x87b1e0] = r.bytes(0x18 + 8 * 8)
    errors = bytearray(r.bytes(0x204))
    put(errors, 0x200, 'i', r.choice([r.randrange(0x40), 0, 0x3f, 0x40]))
    regions[0x879ad0] = errors
    regions[0x8bd508] = struct.pack('<I', r.getrandbits(32) & ~0x1000000)
    return Call(0x572630, [CACHE, TEXTURE, x, y, overlay], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

# The landscape renderer's accessors (src/game/terrain/landscape: 0x5856e0, 0x585710, 0x585ef0,
# 0x585f00) against the originals: points 0 to 3 (and a little either side), any values. Its texture
# arrays' release (0x585e20: the three at +0x1fd0..+0x1fd8, as texturemem.py, counts 0-3, set or not) and one
# array's removal (0x5851d0: 0-4 entries of 0x20 bytes from +0x1fdc, the count at +0x21dc theirs, one less,
# 0 or negative; the array asked for one of theirs, the one past the last, 0 or another).
# usage: landscape.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetPoint,SetDetail,List,Point,FreeTextures,RemoveTexture)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

B = Bench([(0x5856e0, 0x585705), (0x585710, 0x58571d), (0x585ef0, 0x585ef7), (0x585f00, 0x585f07), (0x585e20, 0x585eed), (0x5851d0, 0x5852af),
           (0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1)])
CALLS = {'SetPoint': 0x5856e0, 'SetDetail': 0x585710, 'List': 0x585ef0, 'Point': 0x585f00, 'FreeTextures': 0x585e20, 'RemoveTexture': 0x5851d0}
LAND, ARRAYS = RAM + 0x10000, RAM + 0x5000   # (texturemem.py's regions below RAM + 0x4000)


def array(r, regions, k):
    """a texture array (count 0-3; one texture at least) at ARRAYS + 0x400 k; returns its address."""
    m = r.choice([0, 1, 2, 3])
    base = ARRAYS + 0x400 * k
    regions[base] = struct.pack('<I', m) + texturemem.textures(r, regions, base + 4, max(m, 1))
    return base + 4


def textures(r, op):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    land = bytearray(r.bytes(0x230))   # (+0x1fb0..+0x21e0: with count 0 the entry before the first is swapped with)
    if op == 'FreeTextures':
        for k in range(3):
            put(land, 0x20 + 4 * k, 'I', array(r, regions, k) if r.random() < 0.75 else 0)
        regions[LAND + 0x1fb0] = land
        return Call(0x585e20, [LAND], regions, conv='t', ret='a')
    n = r.choice([0, 1, 2, 3, 4])
    arrays = []
    for k in range(n + 1):   # (and the one past the last, which the search starts at)
        p = array(r, regions, k) if r.random() < 0.7 else r.choice([0, 0, r.getrandbits(32)])
        arrays.append(p)
        put(land, 0x2c + 0x20 * k, 'I', p)
    put(land, 0x22c, 'I', r.choice([n, n, n, max(n - 1, 0), 0, 0xffffffff]))
    regions[LAND + 0x1fb0] = land
    arg = r.choice(arrays) if r.random() < 0.85 else r.choice([0, ARRAYS + 0x2004])
    return Call(0x5851d0, [LAND, arg], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op in ('FreeTextures', 'RemoveTexture'):
        return textures(r, op)
    regions = {RAM + 0x1f00: r.bytes(0x100), RAM + 0x2ec0: r.bytes(0x10)}
    if op == 'SetPoint':
        args = [RAM, r.choice([0, 1, 2, 3, -1, 4, 5])] + [r.getrandbits(32) for _ in range(3)]
        return Call(0x5856e0, args, regions, conv='t')
    if op == 'SetDetail':
        return Call(0x585710, [RAM, r.choice([0, 1, 2, 3, 4, r.getrandbits(32)])], regions, conv='t')
    return Call(CALLS[op], [RAM], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

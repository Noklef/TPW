# The landscape renderer's accessors (src/game/terrain/landscape: 0x5856e0, 0x585710, 0x585ef0,
# 0x585f00) against the originals: points 0 to 3 (and a little either side), any values. Its texture
# arrays' release (0x585e20: the three at +0x1fd0..+0x1fd8, as texturemem.py, counts 0-3, set or not) and one
# array's removal (0x5851d0: 0-4 entries of 0x20 bytes from +0x1fdc, the count at +0x21dc theirs, one less,
# 0 or negative; the array asked for one of theirs, the one past the last, 0 or another), and a list's removal from the landscape at
# 0x8c1828 (0x418d70: 0-4 of those, the count at 0x785724 theirs, one less, 0 or -1). Its constructor (0x584ef0): the whole object
# (0x2f20 bytes) of any bytes, [0x8bd524] 4 or not. Its colour sampling (0x585ce0): a texture (or none) of sizes 0-48 (a few
# negative), pitches around the width, formats 2, 3 and others, its pixels as they are or locked (a recorder vtable).
# usage: landscape.py [CASES [SEED]] [--wild|--both] [--show N]
#        (env OPS=SetPoint,SetDetail,List,Point,FreeTextures,RemoveTexture,RemoveList,Init)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, blob, global_heap, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x418d70, 0x418d9c), (0x5856e0, 0x585705), (0x585710, 0x58571d), (0x585ef0, 0x585ef7), (0x585f00, 0x585f07), (0x585e20, 0x585eed), (0x5851d0, 0x5852af),
           (0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1),
           (0x584ef0, 0x5850b4), (0x585ce0, 0x585e1e), (0x590d40, 0x590d96)])
CALLS = {'Init': 0x584ef0, 'SetPoint': 0x5856e0, 'SetDetail': 0x585710, 'List': 0x585ef0, 'Point': 0x585f00, 'FreeTextures': 0x585e20, 'RemoveTexture': 0x5851d0, 'RemoveList': 0x418d70, 'SampleColours': 0x585ce0}
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
    if op == 'RemoveList':   # (0x418d70: a list of 0-4 of the arrays (or others) removed in turn from the landscape at 0x8c1828)
        regions[0x8c1828 + 0x1fb0] = land
        m = r.choice([0, 1, 2, 3, 4])
        pick = [r.choice(arrays) if r.random() < 0.85 else r.choice([0, ARRAYS + 0x2004]) for _ in range(m)]
        regions[RAM + 0x100] = struct.pack('<%dI' % (m + 1), *pick, 0)
        regions[0x785724] = struct.pack('<i', r.choice([m, m, m, max(m - 1, 0), 0, -1]))
        return Call(0x418d70, [RAM + 0x100], regions, conv='t', ret='a')
    regions[LAND + 0x1fb0] = land
    arg = r.choice(arrays) if r.random() < 0.85 else r.choice([0, ARRAYS + 0x2004])
    return Call(0x5851d0, [LAND, arg], regions, conv='t', ret='a')


def sample(r):
    tex, pixels, obj = RAM + 0x4000, RAM + 0x8000, RAM + 0x4100
    land = bytearray(r.bytes(0x1078))   # (+0xf60..+0x1fd8)
    log = log_region()
    regions = {LAND + 0xf60: land, LOG: log, 0x8bd554: struct.pack('<I', r.choice([2, 3, 2, 3, 0, 1, 5]))}
    if r.random() < 0.05:
        put(land, 0x1074, 'I', 0)
        return Call(0x585ce0, [LAND], regions, conv='t')
    w = r.choice([16, 32, 17, 31, 48, 0, 1, 15, r.randrange(49), r.randrange(-40, 49)])
    h = r.choice([16, 32, 17, 31, 48, 0, 1, 15, r.randrange(49), r.randrange(-40, 49)])
    pitch = max(abs(w) + r.choice([0, 0, 0, -1, 1, 3]), 0)
    texture = bytearray(r.bytes(0x60))
    flags = r.getrandbits(32) & ~0x10000000 | (0x10000000 if r.random() < 0.2 else 0)
    put(texture, 8, 'I', flags)
    put(texture, 0x10, 'h', w)
    put(texture, 0x12, 'h', h)
    put(texture, 0x34, 'h', pitch)
    put(texture, 0x40, 'I', pixels)
    regions[tex] = texture
    regions[pixels] = r.bytes(max(pitch * abs(h), 1) * 4 + 0x40)
    if flags & 0x10000000:
        put(texture, 0x40, 'I', obj)
        regions[obj] = struct.pack('<I', obj + 0x10) + r.bytes(0xc) + vtable({2: 1}, 8)
        blob(log, 2, 0, 0xf)
        regions[0x879a68] = struct.pack('<I', pixels)
    put(land, 0x1074, 'I', tex)
    return Call(0x585ce0, [LAND], regions, conv='t')


def case(r):
    op = r.deal(OPS)
    if op == 'SampleColours':
        return sample(r)
    if op in ('FreeTextures', 'RemoveTexture', 'RemoveList'):
        return textures(r, op)
    if op == 'Init':
        regions = {RAM: r.bytes(0x2f20), 0x8bd524: struct.pack('<I', r.choice([4, 4, 0, 1, 2, 3, 5, r.getrandbits(32)]))}
        return Call(0x584ef0, [RAM], regions, conv='t', ret='a')
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

# The engine textures' release (src/game/gfx/texture: 0x590b90, and its jump 0x590a70) against the originals: flags 0x800000 (the textures
# after it, up to four), 0x1000 (the cache item unlinked, as texcacheunlink.py) and 0x10000000 (the object at +0x40 deleted, a recorder) or
# not, +0x40 / +0x3c set or 0 and found in the tracked table (as they are, 4 below) or not, the float at +0x30 around 0..1.5 or any bits.
# usage: texturemem.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Release,Jump)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, fb, global_heap, heap, log_region, put, vtable  # noqa: E402

CALLS = {'Release': 0x590b90, 'Jump': 0x590a70}
B = Bench([(0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e)])
TEX, CACHE, OWNER, OBJ, VT = RAM + 0x100, RAM + 0x1000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200
COUNT, PAIRS = 0x8bb214, 0x87b1f8


def lod(r):
    return r.choice([fb(r.uniform(0, 1.5)), fb(r.choice([0.0, 1.5, 1.0, -1.0, 3.0])), r.f()])


def texture(r, buf, off, last):
    flags = r.getrandbits(32) & ~0x10801000
    for bit, p in ((0x800000, 0.3), (0x1000, 0.5), (0x10000000, 0.3)):
        if r.random() < p and not (last and bit == 0x800000):
            flags |= bit
    put(buf, off + 8, 'I', flags)
    put(buf, off + 0x18, 'I', OWNER)
    put(buf, off + 0x30, 'I', lod(r))
    put(buf, off + 0x3c, 'I', r.choice([0, 0, HEAP + 0x1000, HEAP + 0x1100, r.getrandbits(32)]))
    put(buf, off + 0x40, 'I', OBJ if flags & 0x10000000 and r.random() < 0.8 else r.choice([0, HEAP + 0x1200, HEAP + 0x1300, r.getrandbits(32)]))


def textures(r, regions, base=TEX, n=4):
    """n textures at base, the tracked table holding some of their blocks, the cache at 0x8c4b44 with some of them in its list; returns them."""
    tex = bytearray(r.bytes(0x60 * n))
    for k in range(n):
        texture(r, tex, 0x60 * k, k == n - 1)
    blocks = [struct.unpack_from('<I', tex, 0x60 * k + o)[0] for k in range(n) for o in (0x3c, 0x40)]
    table = bytearray(8 * 8)
    for k in range(8):
        b = r.choice(blocks + [r.getrandbits(32)])
        put(table, 8 * k, 'II', r.getrandbits(32), r.choice([b, b, (b + 4) & 0xffffffff]))  # (looked up as it is, then 4 above)
    regions[PAIRS] = table
    regions[COUNT] = struct.pack('<I', r.choice([0, 1, 2, 3, 4, 6, 8]))
    cache = put(bytearray(r.bytes(0x1900)), 0x18cc, 'I', r.getrandbits(32) | 0x20000000 if r.random() < 0.85 else r.getrandbits(32) & ~0x20000000)
    regions[CACHE] = cache
    regions[0x8c4b44] = struct.pack('<I', CACHE)
    nodes = [base + 0x60 * k for k in range(n) if r.random() < 0.6]
    r.shuffle(nodes)
    for node, link in zip(nodes, nodes[1:] + [0]):
        put(tex, node - base + 4, 'I', link)
    regions[OWNER] = put(bytearray(r.bytes(0x10)), 4, 'I', nodes[0] if nodes else 0)
    regions[OBJ] = struct.pack('<I', VT) + r.bytes(12)
    regions[VT] = vtable({0: 1})
    return tex


def case(r):
    name = r.choice(OPS)
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    regions[TEX] = textures(r, regions)
    return Call(CALLS[name], [TEX], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# A draw batch's clipped triangle list (src/game/render/drawbuffer: 0x565960) against the original: 1-24 vertices of stride 0x20 near the
# view volume (clippoly.py's), 0-8 triangles of them (the count now and then not a multiple of 3, 0 or negative), the clip code table at
# 0x879970 sparse bits or the index's own, any mask; the object's vertex and index ends at, under and past what the checks need (the
# vertex size +0x1c 0x18-0x30 or any), the batch's counts random; the clip edges' globals of any bytes. Poly: a polygon clipped into the
# batch (0x565bb0), see poly().
# usage: drawbatchclip.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Tris,Poly)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import clippoly  # noqa: E402
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x565960, 0x565e38), (0x561bc0, 0x562f80)])
OBJ, BATCH, COUNT, IDX, OUT, VERTS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x1000, RAM + 0x2000
SRC, CODES = RAM + 0x4000, RAM + 0x280


def globs(r):
    """the clip edges' globals and 0x879a20 any bytes, the clip code table at 0x879970 sparse bits or the index's own"""
    glob = bytearray(r.bytes(0xc4))
    sparse = r.random() < 0.7
    for i in range(0x80):
        glob[0x10 + i] = sum(1 << b for b in range(8) if r.random() < 0.15) if sparse else i & 0x3f
    return {0x879960: glob, 0xfb34a0: r.bytes(8)}


def poly(r):
    """0x565bb0: a convex polygon of 3-8 vertices (1-2 now and then; with codes given 0 and 9-10 too) of stride 0x18-0x28, the codes given
    ({and, or}: and mostly 0) or computed (then never more than 8 vertices: the original's local for them has 8 bytes before its return
    address); the vertices' start, the batch's counts and the object's room as for the other op; flag 2 or not (the winding)."""
    stride = r.choice([0x18, 0x20, 0x28])
    given = r.random() < 0.4
    n = r.choice([3, 4, 5]) if r.random() < 0.8 else r.choice([1, 2, 6, 7, 8] + ([0, 9, 10] if given else []))
    src = bytearray()
    for p in clippoly.polygon(r, n):
        src += clippoly.vertex(r, p, stride)
    src += r.bytes(0x800 - len(src))
    base, used = r.choice([0, 3, 0x20, 0x40]), r.choice([0, 3, 0x10])
    batch = bytearray(r.bytes(0x38))
    put(batch, 0x20, 'IIII', base, used, VERTS, OUT)
    size = r.choice([0x20, 0x20, 0x18, 0x28, 0x30, 0])
    need_v, need_i = size * (base + 10) + VERTS, OUT + 2 * used + 0x30
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x1c, 'II', size, BATCH)
    put(obj, 0x28, 'II', r.choice([need_v, need_v, need_v + 0x100, need_v - 1]), r.choice([need_i, need_i, need_i + 0x40, need_i - 2]))
    regions = {OBJ: obj, BATCH: batch, SRC: src, VERTS: r.bytes(0x1000), OUT: r.bytes(0x100)}
    regions.update(globs(r))
    codes = 0
    if given:
        codes = CODES
        regions[CODES] = bytes([0 if r.random() < 0.75 else r.choice([r.getrandbits(8), 0x40, 0x80, 1 << r.randrange(6)]),r.choice([0, r.randrange(64), r.getrandbits(8), 1 << r.randrange(6)])])
    flags = r.getrandbits(32) & ~2 | r.choice([0, 2])
    return Call(0x565bb0, [OBJ, SRC, n, stride, codes, flags], regions, conv='t', ret='a')


def case(r):
    if r.deal(OPS) == 'Poly':
        return poly(r)
    nv = r.choice([1, 2, 3, 4, 6, 9, 12, 24])
    verts = bytearray()
    while len(verts) < 0x20 * nv:
        for p in clippoly.polygon(r, r.choice([3, 4, 5])):
            verts += clippoly.vertex(r, p, 0x20)
    verts = verts[:0x20 * nv] + r.bytes(0x2000 - 0x20 * nv)
    m = r.choice([0, 1, 1, 2, 3, 4, 8])
    idx = bytearray(0x60)
    for i in range(0x30):   # (all of them: a count past 3 m reads on, and an index past 0x3ff would read the original's stack)
        put(idx, 2 * i, 'H', r.randrange(nv))
    count = 3 * m if r.random() < 0.85 else r.choice([3 * m + 1, 3 * m - 1, 0, -3, -0x80000000])
    used = r.choice([0, 0, 3, 0x10])
    batch = bytearray(r.bytes(0x38))
    put(batch, 0x20, 'IIII', r.choice([0, 0, 2, 0x40]), used, VERTS, OUT)
    size = r.choice([0x20, 0x20, 0x18, 0x30, 0, r.getrandbits(32)])
    need_v = (struct.unpack_from('<I', batch, 0x20)[0] + nv) * size + VERTS & 0xffffffff
    vend = r.choice([need_v, need_v - 1, VERTS + 0x20 * nv + 0x140 + 0x20 * r.randrange(0, 24), VERTS + 0x1400])
    need_i = OUT + 2 * (used + count) & 0xffffffff
    iend = r.choice([need_i, need_i - 2, OUT + 2 * used + 0x18 + 2 * r.randrange(0, 40), OUT + 0x200])
    if r.random() < 0.85:
        vend, iend = max(vend, need_v), max(iend, need_i)
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x1c, 'II', size, BATCH)
    put(obj, 0x28, 'II', min(vend, VERTS + 0x1400), min(iend, OUT + 0x200))
    regions = globs(r)
    mask = r.choice([0, 0, 0xc0, r.getrandbits(32), 1 << r.randrange(6), 0x3f])
    regions.update({OBJ: obj, BATCH: batch, COUNT: struct.pack('<I', nv), IDX: idx, OUT: r.bytes(0x300), VERTS: verts})
    return Call(0x565960, [OBJ, COUNT, IDX, count & 0xffffffff, mask], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Tris,Poly').split(',') if k]

if __name__ == '__main__':
    B.main(case)

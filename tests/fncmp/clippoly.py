# The clipper of a polygon of vertex indices (src/game/render/clippoly: 0x562c90, 0x562ed0) against the originals: triangles of any vertices
# and convex flat polygons of 4-5 (now and then 0-2 indices, repeated indices, NaNs), mostly near the view volume, any flags (0, the six bits,
# negative), strides 0x18-0x28, the vertex count past the vertices or further, the clip edges' globals of any bytes.
# usage: clippoly.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x561c40, 0x562a6d), (0x562c90, 0x562f80)])
OPS = [0x562c90, 0x562ed0]
VERTS, POLY, COUNT, IDX = RAM, RAM + 0x1000, RAM + 0x1010, RAM + 0x1100


def coords(r):
    w = r.uniform(0.2, 3) if r.random() < 0.9 else r.uniform(-1, 1)
    return [r.uniform(-1.6, 1.6) * w, r.uniform(-1.6, 1.6) * w, r.uniform(-0.4, 1.4) * w, w]


def polygon(r, n):
    if n <= 3:
        return [coords(r) for _ in range(n)]
    c, u, v = coords(r), coords(r), coords(r)   # flat and convex: c + cos u + sin v
    a0 = r.uniform(0, 6.3)
    return [[c[k] + 0.7 * (math.cos(a0 + 6.2832 * i / n) * u[k] + math.sin(a0 + 6.2832 * i / n) * v[k]) for k in range(4)] for i in range(n)]


def vertex(r, p, stride):
    v = r.bytes(stride)
    r.floats(v, range(0x18, stride, 4), 10)
    for k in range(4):
        put(v, 4 * k, 'I', fb(p[k]))
    if r.random() < 0.03:
        put(v, 4 * r.randrange(4), 'I', r.f(special=1))
    if r.random() < 0.3:
        v[0x14:0x18] = v[0x10:0x14]
    return v


def case(r):
    op = r.deal(OPS)
    stride = r.choice([0x18, 0x20, 0x28])
    k = r.random()
    n = 3 if op == 0x562ed0 or k < 0.6 else (r.choice([4, 5]) if k < 0.95 else r.randrange(3))
    pts = polygon(r, max(n, 3))
    nv = len(pts) + r.randrange(3)
    verts = bytearray()
    for i in range(nv):
        verts += vertex(r, pts[i] if i < len(pts) else coords(r), stride)
    verts += r.bytes(0x400 - len(verts))
    order = list(range(len(pts)))
    if r.random() < 0.5:
        r.shuffle(order)
    idx = [order[i] if r.random() < 0.95 else r.randrange(nv) for i in range(n)]
    count = nv + (r.randrange(4) if r.random() < 0.2 else 0)
    flags = r.choice([r.randrange(64), r.randrange(64), 63, r.randrange(1 << 32), r.randrange(1 << 31, 1 << 32), 0, 1 << r.randrange(6)])
    buf = r.bytes(0x40)
    regions = {VERTS: verts, COUNT: struct.pack('<I', count), 0x879960: r.bytes(0xc4), 0xfb34a0: r.bytes(8)}
    if op == 0x562c90:
        start = 4 + 2 * r.randrange(4)
        for i, x in enumerate(idx):
            put(buf, start + 2 * i, 'H', x)
        regions[POLY] = struct.pack('<iI', n, IDX + start)
        regions[IDX] = buf
        return Call(op, [r.getrandbits(32), flags, VERTS, POLY, COUNT, stride], regions, conv='t', ret='a')
    start = 2 * r.randrange(4)
    for i, x in enumerate(idx):
        put(buf, start + 2 * i, 'H', x)
    regions[IDX] = buf
    return Call(op, [flags, IDX + start + 6, VERTS, COUNT, stride], regions, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)

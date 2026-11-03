# A ray against a model node's faces (src/game/terrain/raypick: RayPick_Node 0x45d090) against the original: WorldVerts_TransformNode (k 1) a recorder, the block of world vertices [0x7aecd4]
# set up by the case; Matrix_PreMul, Matrix_TransformVector and the CRT's acos as they are. A chain of nodes (children by +0xc, siblings by +8, flag 0x20 stopping them), flags 0x10 and 0x8210
# (no faces), groups of faces; some faces round the point where the ray comes down, so that it hits (the matrices then the identity, or a translation), others not. The matrix by value.
# usage: raypicknode.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, log_region, put  # noqa: E402

B = Bench([(0x45d090, 0x45d560), (0x4702d0, 0x4704d0), (0x46fca0, 0x46fd40), (0x67a400, 0x690000)], hooks={0x466c80: _recorder(1, 2, cdecl=True)})
ORIGIN, DIR, VERTS, NODES = RAM, RAM + 0x20, RAM + 0x1000, RAM + 0x2000
NODE = 0x200    # a node, then its normals, groups, faces and index in its 0x200-byte room
VERTS_EACH = 3
IDENTITY = (1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0)


def matrix(r, plain):
    """A row-major matrix: the identity with or without a translation (a ray still hits), or any."""
    if plain:
        m = list(IDENTITY)
        if r.random() < 0.5:
            m[12:15] = [r.uniform(-50, 50) for _ in range(3)]
        return m
    return [r.uniform(-1.5, 1.5) for _ in range(12)] + [r.uniform(-50, 50) for _ in range(3)] + [1.0]


def build(r, plain, origin, direction):
    """The nodes, the vertex block; the block's vertices in threes, a face's own, each round a point on the ray or anywhere."""
    regions = {}
    verts = []
    count = r.deal([1, 1, 2, 3, 4])
    for k in range(count):
        at = NODES + NODE * k
        node = bytearray(r.bytes(NODE))
        flags = r.getrandbits(32) & ~0x8230
        flags |= r.choice([0, 0, 0, 0, 0, 0x10, 0x200, 0x8000, 0x20])
        put(node, 0, 'I', flags)
        # the first node's child is the next node, and each other a sibling (+8) or a child (+0xc) of the one before it
        put(node, 8, 'I', 0)
        put(node, 0xc, 'I', 0)
        put(node, 0x10, '16f', *matrix(r, plain))
        groups = r.deal([0, 1, 1, 2])
        put(node, 0x5a, 'H', groups)
        normals_at, groups_at, faces_at, index_at = 0x98, 0xc8, 0xe8, 0x128
        put(node, 0x64, 'I', at + normals_at)
        put(node, 0x6c, 'I', at + groups_at)
        put(node, 0x70, 'I', at + faces_at)
        put(node, 0x94, 'I', at + index_at)
        face = 0
        for g in range(groups):
            many = r.deal([0, 1, 2, 2])
            put(node, groups_at + 0x10 * g + 4, 'H', face)
            put(node, groups_at + 0x10 * g + 6, 'H', many)
            for _ in range(many):
                base = len(verts)
                hit = r.random() < 0.3
                t = r.uniform(5, 40)
                point = [origin[i] + direction[i] * t for i in range(3)] if hit else [r.uniform(-60, 60) for _ in range(3)]
                # the triangle in the xz plane round the point, its normal (0, 1, 0), when the ray comes down on it
                spin = r.uniform(0, 6.28)
                size = r.choice([0.0, r.uniform(1, 9), r.uniform(1, 9), r.uniform(1, 9), r.uniform(1, 9), r.uniform(1, 9), r.uniform(1, 9), r.uniform(1, 9)])   # (no size: NaNs)
                if r.random() < 0.5:   # the ray's point just inside or outside the edge opposite the first corner: the angles' sum against the limit
                    out = 0.4976 * size * (1 + r.choice([r.uniform(-0.1, 0.1), r.uniform(0, 0.006)]))
                    point = [point[0] - out * math.cos(spin + 3.15), point[1], point[2] - out * math.sin(spin + 3.15)]
                for a in (0.0, 2.1, 4.2):
                    verts.append((point[0] + size * math.cos(spin + a), point[1], point[2] + size * math.sin(spin + a)))
                # (not of length one; none: NaNs)
                normal = (0.0, r.uniform(0.2, 3), 0.0) if hit or r.random() < 0.5 else (r.uniform(-1, 1), r.uniform(-1, 1), r.uniform(0.1, 1))
                if r.random() < 0.05:
                    normal = (0.0, 0.0, 0.0)
                put(node, normals_at + 0xc * face, '3f', *normal)
                # (each face its own three: the index's entries point at them)
                put(node, index_at + 2 * (3 * face), '3H', base, base + 1, base + 2)
                put(node, faces_at + 8 * face, '4H', r.getrandbits(1) << 15 | face, 3 * face, 3 * face + 1, 3 * face + 2)
                face += 1
        regions[at] = node
    links = [NODES + NODE * k for k in range(count)]
    for k in range(1, count):
        parent = regions[links[k - 1]]
        put(parent, r.choice([8, 0xc]) if k > 1 else 0xc, 'I', links[k])
    regions[VERTS] = struct.pack('<%df' % (3 * len(verts)), *[c for v in verts for c in v]) if verts else b'\0' * 12
    return regions


def case(r):
    log = log_region()
    blob(log, 1, 1, 0xffff)   # (the matrix on the stack, as it has come to be)
    origin = [r.uniform(-30, 30), r.uniform(20, 60), r.uniform(-30, 30)]
    direction = [r.uniform(-0.5, 0.5), -1.0, r.uniform(-0.5, 0.5)]
    plain = r.random() < 0.7
    regions = build(r, plain, origin, direction)
    regions.update({LOG: log, ORIGIN: struct.pack('<3f', *origin), DIR: struct.pack('<3f', *direction), 0x7a1cb0: ORIGIN.to_bytes(4, 'little'), 0x7a1cd8: DIR.to_bytes(4, 'little'),
                    0x7aecd4: VERTS.to_bytes(4, 'little'), 0x7a1cc8: r.bytes(12), 0x7a1c80: r.bytes(12),
                    0xfb1330: b'\1\0\0\0'})   # (the CRT's flag: acos past 1, a dot of rounded unit vectors, returns its NaN without the math error path, whose start-up tables a bench does not have)
    args = [NODES] + [struct.unpack('<I', struct.pack('<f', x))[0] for x in matrix(r, plain)]
    return Call(0x45d090, args, regions, ret='a')


if __name__ == '__main__':
    B.main(case)

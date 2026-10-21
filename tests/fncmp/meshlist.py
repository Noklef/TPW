# Readying meshes (src/game/render/meshlist: 0x5744d0, 0x574110) against the originals: zero-ended arrays of
# up to 5 meshes, with and without flags 0x20 and 0x10000000, one mesh listed twice, empty arrays; and any
# directions (zero, tiny, huge and NaN (signalling ones too) now and then) made unit length. View (0x5743a0): a matrix of floats (sometimes
# with a zero or NaN, or a rotation and translation) inverted and 0-4 listed meshes with flags 2 / 4 either way, their vectors floats
# (unit length, zero or NaN now and then); the object's direction.
# Light (0x5741b0): the light at a point, see light(). Points (0x574530) and Mesh (0x574660): colours lit, see points() and mesh().
# usage: meshlist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ready,Direction,View,Light,Points,Mesh)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402

B = Bench([(0x574110, 0x5741a7), (0x5744d0, 0x574526), (0x5741b0, 0x57439b), (0x574530, 0x57465b), (0x574660, 0x5747d0), (0x5743a0, 0x5744e0), (0x578820, 0x578bd0), (0x578d20, 0x579020)])
OPS = [k for k in os.environ.get('OPS', 'Ready,Direction,View,Light,Points,Mesh').split(',') if k]
MESHES, LIST = RAM + 0x1000, RAM + 0x2000
LIGHTS, VEC = RAM + 0x3000, RAM + 0x3800
OBJ, OUT, NORMALS, POINTS, MESH, INDEX = RAM + 0x4000, RAM + 0x4100, RAM + 0x4200, RAM + 0x4400, RAM + 0x4600, RAM + 0x4700


def light(r):
    """0x5741b0 ((normal, x, y, z), pops 16; ECX any: it seeds the kind 6 factor before a kind 4): the ambient any, 0-5 lights of kinds 2,
    4, 6 or others (the flags' other bits any), colours and directions small floats (special now and then), positions near the point or
    not, +0x48 a squared radius (now and then negative or any bits), +0x4c a fall-off; the normal unit-ish or any."""
    lights = bytearray(r.bytes(0x400))
    n = r.choice([0, 1, 2, 3, 4, 5])
    p = [r.uniform(-20, 20) for _ in range(3)]
    for k in range(n):
        o = 0x60 * k
        put(lights, o + 0x50, 'I', r.getrandbits(32) & ~0xf | r.choice([2, 4, 6, 2, 4, 6, r.randrange(16)]))
        put(lights, o + 0x58, 'I', LIGHTS + 0x60 * (k + 1) if k + 1 < n else 0)
        put(lights, o + 4, '4I', *(r.f(2) for _ in range(4)))
        put(lights, o + 0x20, '3I', *(r.choice([fb(c + r.uniform(-10, 10)), r.f(30)]) for c in p))
        put(lights, o + 0x38, '3I', *(r.f(1) for _ in range(3)))
        put(lights, o + 0x48, 'II', r.choice([fb(r.uniform(0, 300)), fb(r.uniform(-50, 50)), r.getrandbits(32), r.f(100)]), r.f(1))
    vec = bytearray(b''.join(struct.pack('<I', r.f(1)) for _ in range(3)))
    regions = {LIGHTS: lights, VEC: vec, 0x879db0: r.bytes(0x3c), 0x879dec: struct.pack('<I', LIGHTS if n else 0)}
    point = [r.choice([fb(c), r.f(30)]) for c in p]
    return Call(0x5741b0, [r.choice([r.getrandbits(32), fb(r.uniform(0, 300))]), VEC] + point, regions, conv='t')


def lit(r):
    """light()'s lights, the object (its +0x2c mask all, the colour bits or any), the flags 0x8bcbc8 with 0x800 now and then; the
    point light()'s, for the vertices near it."""
    c = light(r)
    regions = dict(c.regions)
    obj = put(bytearray(r.bytes(0x40)), 0x2c, 'I', r.choice([0xffffffff, 0xffffffff, 0x00ffffff, r.getrandbits(32)]))
    regions.update({OBJ: obj, OUT: r.bytes(0x40), 0x8bcbc8: struct.pack('<I', r.getrandbits(32) & ~0x800 | (0x800 if r.random() < 0.1 else 0)),
                    0xfb34a4: r.bytes(4)})
    point = [struct.unpack('<f', struct.pack('<I', b))[0] for b in c.args[2:5]]
    near = lambda k: fb(point[k] + r.uniform(-8, 8)) if point[k] == point[k] and abs(point[k]) < 1e30 and r.random() < 0.9 else r.f(30)  # noqa: E731
    normals = struct.pack('<24I', *(r.f(1) for _ in range(24)))
    return regions, near, normals


def points(r):
    """0x574530 (thiscall (colours, normals, points, count)): lit()'s scene, 0-6 points near its point, their normals any."""
    regions, near, normals = lit(r)
    n = r.choice([0, 1, 2, 3, 6])
    pts = bytearray(r.bytes(0x80))
    for i in range(n):
        put(pts, 0x10 * i, '3I', near(0), near(1), near(2))
    regions.update({NORMALS: normals, POINTS: pts})
    return Call(0x574530, [OBJ, OUT, NORMALS, POINTS, n], regions, conv='t')


def mesh(r):
    """0x574660 (thiscall (colours, mesh)): lit()'s scene, a mesh of 0-6 vertices (+0x5e) indexing three groups of four positions near
    its point (the index list's words below 12, the rest of their word any), their normals any."""
    regions, near, normals = lit(r)
    n = r.choice([0, 1, 2, 3, 6])
    pos = bytearray(0x90)
    for g in range(3):
        for k in range(3):
            put(pos, 0x30 * g + 0x10 * k, '4I', *(near(k) for _ in range(4)))
    m = put(bytearray(r.bytes(0x98)), 0x5e, 'H', n)
    put(m, 0x60, '2I', POSITIONS, NORMALS)
    put(m, 0x94, 'I', INDEX)
    regions.update({NORMALS: normals, POSITIONS: pos, MESH: m, INDEX: struct.pack('<8H', *(r.randrange(12) for _ in range(8)))})
    return Call(0x574660, [OBJ, OUT, MESH], regions, conv='t')


POSITIONS = RAM + 0x4800


def view(r):
    if r.random() < 0.4:   # a rotation about z and a translation, as a view is
        import math
        a = r.uniform(-4, 4)
        c, s = math.cos(a), math.sin(a)
        m = [c, s, 0, 0, -s, c, 0, 0, 0, 0, 1, 0, r.uniform(-50, 50), r.uniform(-50, 50), r.uniform(-50, 50), 1]
        m = [fb(x) for x in m]
    else:
        m = [r.f(r.choice([1, 10])) for _ in range(16)]
    if r.random() < 0.1:
        m[r.randrange(16)] = r.choice([0, 0x7fc00000, 0xff812345])
    n = r.choice([0, 1, 2, 3, 4])
    meshes = bytearray(r.bytes(0x60 * 4))
    for i in range(4):
        put(meshes, 0x60 * i + 0x50, 'I', r.getrandbits(32) & ~6 | r.choice([0, 2, 4, 6, 2, 6]))
        put(meshes, 0x60 * i + 0x58, 'I', MESHES + 0x60 * (i + 1) if i + 1 < n else 0)
        for off in (0x14, 0x2c):
            v = [r.f(r.choice([1, 20])) for _ in range(3)]
            if r.random() < 0.15:
                v = r.choice([[0, 0, 0], [0x3f800000, 0, 0], [0, 0x80000000, 0x3f800000], [0x7fc00000, 0, 0]])
            put(meshes, 0x60 * i + off, '3I', *v)
    globals_ = bytearray(r.bytes(0x64))
    put(globals_, 0x20, 'I', MESHES if n else 0)
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x34, '3I', *[r.f() for _ in range(3)])
    regions = {RAM: obj, MESHES: meshes, LIST: struct.pack('<16I', *m), 0x879dcc: globals_}
    return Call(0x5743a0, [RAM, LIST, r.getrandbits(32)], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Light':
        return light(r)
    if op == 'Points':
        return points(r)
    if op == 'Mesh':
        return mesh(r)
    if op == 'View':
        return view(r)
    if op == 'Direction':
        v = [r.f() if r.random() < 0.7 else r.choice([0, 0x80000000, r.f(1e-20), r.f(1e19), 0x7f800001, 0xff812345]) for _ in range(3)]
        if r.random() < 0.15:
            v = [r.choice([0, 0x80000000]) for _ in range(3)]
        return Call(0x574110, [RAM] + v, {RAM: r.bytes(0x40)}, conv='t')
    count = r.choice([0, 1, 2, 3, 5])
    meshes = bytearray(r.bytes(0x100 * 5))
    for i in range(5):
        flags = r.getrandbits(32) & ~0x10000020
        flags |= r.choice([0, 0x20]) | r.choice([0, 0x10000000, 0, 0])
        put(meshes, 0x100 * i + 0x50, 'I', flags)
    pick = [r.randrange(5) for _ in range(count)]
    if count > 1 and r.random() < 0.2:
        pick[1] = pick[0]
    entries = [MESHES + 0x100 * i for i in pick] + [0]
    regions = {RAM: r.bytes(0x40), MESHES: meshes, LIST: struct.pack('<%dI' % len(entries), *entries),
               0x879de8: r.bytes(8)}
    return Call(0x5744d0, [RAM, LIST], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

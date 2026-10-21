# Readying meshes (src/game/render/meshlist: 0x5744d0, 0x574110) against the originals: zero-ended arrays of
# up to 5 meshes, with and without flags 0x20 and 0x10000000, one mesh listed twice, empty arrays; and any
# directions (zero, tiny, huge and NaN (signalling ones too) now and then) made unit length. View (0x5743a0): a matrix of floats (sometimes
# with a zero or NaN, or a rotation and translation) inverted and 0-4 listed meshes with flags 2 / 4 either way, their vectors floats
# (unit length, zero or NaN now and then); the object's direction.
# usage: meshlist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ready,Direction,View)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402

B = Bench([(0x574110, 0x5741a7), (0x5744d0, 0x574526), (0x5743a0, 0x5744e0), (0x578820, 0x578bd0), (0x578d20, 0x579020)])
OPS = [k for k in os.environ.get('OPS', 'Ready,Direction,View').split(',') if k]
MESHES, LIST = RAM + 0x1000, RAM + 0x2000


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

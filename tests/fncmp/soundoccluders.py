# The sound occluders' test (src/game/core/soundoccluders: 0x6c4c80) and the clamp it uses (src/game/math/vector: 0x6c4f00) against the
# originals. 'clamp': x, a and b small integers (ties) or zeros of either sign, ordinary or odd floats (NaNs quiet and signalling,
# infinities). 'occlusion':
# a list (+4 the head, +0xc the count, 0 now and then; +0x14 the smallest radius, NaN now and then) of 0-6 occluders (x +0, z +4, the radius
# +0xc), sorted by radius (largest first) or not; from and to and the occluders mostly small integers, so that points fall on the circles
# (ties), to now and then put on the first occluder's circle; now and then ordinary or odd floats.
# usage: soundoccluders.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

B = Bench([(0x6c4c80, 0x6c4eff), (0x6c4f00, 0x6c4f54)])
LIST, NODES, OCCS, FROM, TO, OUT = (RAM + 0x100 * i for i in range(6))
ODD = [0x7fc00000, 0xffc00000, 0x7f800001, 0xff800001, 0x7fa00000, 0x7f800000, 0xff800000, 0x80000000, 0]
OPS = [n for n in ('clamp', 'occlusion') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def value(r, small=8):
    x = r.random()
    if x < 0.1:
        return r.choice([0, 0x80000000])
    if x < 0.7:
        return fb(float(r.randint(-small, small)))
    if x < 0.85:
        return r.f(small * 2)
    if x < 0.95:
        return fb(r.uniform(-small, small))
    return r.choice(ODD + [r.getrandbits(32)])


def radius(r):
    x = r.random()
    if x < 0.7:
        return fb(float(r.randint(0, 8)))
    if x < 0.9:
        return fb(r.uniform(0, 10))
    return r.choice(ODD + [fb(-1.0), r.getrandbits(32)])


def case(r):
    name = r.deal(OPS)
    if name == 'clamp':
        return Call(0x6c4f00, [value(r, 4) for _ in range(3)], {RAM: bytes(4)}, conv='c', ret='d')
    n = r.choice([0, 1, 1, 2, 3, 4, 6])
    occs = []
    for _ in range(n):
        occs.append([value(r), value(r), r.getrandbits(32), radius(r)])
    if r.random() < 0.7:
        occs.sort(key=lambda o: -struct.unpack('<f', struct.pack('<I', o[3]))[0] if o[3] & 0x7f800000 != 0x7f800000 else 0)
    nodes = bytearray(r.bytes(8 * max(n, 1)))
    for i in range(n):
        struct.pack_into('<II', nodes, 8 * i, OCCS + 0x10 * i, NODES + 8 * (i + 1) if i + 1 < n else 0)
    lst = bytearray(r.bytes(0x20))
    struct.pack_into('<I', lst, 4, NODES if n else r.choice([0, 0, NODES]))
    struct.pack_into('<I', lst, 0xc, r.choice([n] * 8 + [0, r.getrandbits(32)]) if n else r.choice([0, 0, 1]))
    struct.pack_into('<I', lst, 0x14, r.choice([0, fb(1.0), fb(2.0), radius(r), radius(r)]))
    if not n:
        struct.pack_into('<II', nodes, 0, OCCS, 0)
        occs = [[value(r), value(r), r.getrandbits(32), radius(r)]]
    points = []
    for _ in range(2):
        points.append(struct.pack('<3I', value(r), r.getrandbits(32), value(r)))
    if r.random() < 0.3:   # to on the first occluder's circle (an integer radius, a Pythagorean offset)
        k = r.randint(1, 2)
        dx, dz = r.choice([(3, 4), (4, 3), (5, 0), (0, 5), (-3, 4), (3, -4), (0, -5), (-5, 0)])
        cx, cz = r.randint(-6, 6), r.randint(-6, 6)
        occs[0][0], occs[0][1], occs[0][3] = fb(float(cx)), fb(float(cz)), fb(float(5 * k))
        points[1] = struct.pack('<3I', fb(float(cx + k * dx)), r.getrandbits(32), fb(float(cz + k * dz)))
    regions = {LIST: lst, NODES: nodes, OCCS: b''.join(struct.pack('<4I', *o) for o in occs), FROM: points[0], TO: points[1], OUT: r.bytes(4)}
    return Call(0x6c4c80, [LIST, FROM, TO, OUT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

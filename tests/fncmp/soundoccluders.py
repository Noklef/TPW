# The sound occluders' test (src/game/core/soundoccluders: 0x6c4c80) and the clamp it uses (src/game/math/vector: 0x6c4f00) against the
# originals. 'clamp': x, a and b small integers (ties) or zeros of either sign, ordinary or odd floats (NaNs quiet and signalling,
# infinities). 'occlusion':
# a list (+4 the head, +0xc the count, 0 now and then; +0x14 the smallest radius, NaN now and then) of 0-6 occluders (x +0, z +4, the radius
# +0xc), sorted by radius (largest first) or not; from and to and the occluders mostly small integers, so that points fall on the circles
# (ties), to now and then put on the first occluder's circle; now and then ordinary or odd floats.
# The list's teardown (0x6c4c20 'clear', 0x6c4940 'dtor', 0x6c4ff0 'basedtor', 0x6c4f60 'basedel', 0x6c5030 'del', 0x6c4920 'single' on
# the list at 0xfb2038 with its guard byte 0xfb2054): 0-5 nodes {occluder (0 now and then), next, prev}, the count the nodes' or fewer
# (more would loop for ever in the clear, sic: see soundoccluders.h) for the clear and its callers, anything for the others; the head 0
# with a count of 0 now and then; flags 0-3 or random; the frees' order hashed by heap().
# usage: soundoccluders.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, fb, heap  # noqa: E402

B = Bench([(0x6c4920, 0x6c49c0), (0x6c4c20, 0x6c4eff), (0x6c4f00, 0x6c4f54), (0x6c4f60, 0x6c4fc0), (0x6c4ff0, 0x6c5060)])
LIST, NODES, OCCS, FROM, TO, OUT = (RAM + 0x100 * i for i in range(6))
ODD = [0x7fc00000, 0xffc00000, 0x7f800001, 0xff800001, 0x7fa00000, 0x7f800000, 0xff800000, 0x80000000, 0]
SINGLE = 0xfb2038
TEARDOWN = {'clear': 0x6c4c20, 'dtor': 0x6c4940, 'basedtor': 0x6c4ff0, 'basedel': 0x6c4f60, 'del': 0x6c5030, 'single': 0x6c4920}
OPS = [n for n in ('clamp', 'occlusion') + tuple(TEARDOWN) if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


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


def teardown(r, name):
    at = SINGLE if name == 'single' else LIST
    n = r.choice([0, 1, 1, 2, 3, 5])
    nodes = bytearray(r.bytes(0xc * max(n, 1)))
    for i in range(n):
        struct.pack_into('<3I', nodes, 0xc * i, r.choice([0, OCCS + 0x10 * i, OCCS + 0x10 * i, r.getrandbits(32)]),
                         NODES + 0xc * (i + 1) if i + 1 < n else 0, r.choice([NODES + 0xc * (i - 1) if i else 0, r.getrandbits(32)]))
    lst = bytearray(r.bytes(0x20))
    clears = name in ('clear', 'dtor', 'single')
    count = r.choice([n, n, n, r.randint(0, n)]) if clears else r.choice([n, n, 0, r.getrandbits(32)])
    struct.pack_into('<4I', lst, 0, r.getrandbits(32), NODES if n else 0, r.choice([NODES + 0xc * (n - 1), r.getrandbits(32)]) if n else 0, count)
    if name == 'single':
        lst[0x1c] = r.choice([0, 1, r.getrandbits(8)])
    regions = {at: lst, NODES: nodes, HEAP: heap(0x100)}
    if name in ('basedel', 'del'):
        return Call(TEARDOWN[name], [at, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    return Call(TEARDOWN[name], [] if name == 'single' else [at], regions, conv='c' if name == 'single' else 't')


def case(r):
    name = r.deal(OPS)
    if name in TEARDOWN:
        return teardown(r, name)
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

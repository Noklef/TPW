# The mesh builder's geometry helpers (src/game/math/geometry: 0x5812f0, 0x581320) against the originals: any floats (special values
# now and then), points sharing coordinates so the winding products tie, and NaNs. AtX / AtZ (0x56d520, 0x56d590): two points and a
# plane of ordinary floats or fncmp's (signalling NaNs too, and quiet ones of either sign, whose ties show the NaN rule),
# coordinates shared so the span or the distance is zero.
# usage: geometry.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Dot,Winding,BoxExtend,AtX,AtZ)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, bf, fb  # noqa: E402

B = Bench([(0x5812f0, 0x581315), (0x581320, 0x581403), (0x57b410, 0x57b48f), (0x56d520, 0x56d5fd)])
OPS = [k for k in os.environ.get('OPS', 'Dot,Winding,BoxExtend,AtX,AtZ').split(',') if k]


def point(r, shared):
    v = [r.f() for _ in range(4)]
    for i in (0, 1, 3):
        if shared and r.random() < 0.4:
            v[i] = r.choice(shared)
    return struct.pack('<4I', *v)


def coordinate(r):
    return r.choice([fb(r.uniform(-1000.0, 1000.0)), fb(float(r.randrange(-64, 65))), fb(r.uniform(-1e-3, 1e-3)), r.f(), 0x7fa00000,
                     0xff800001, 0x7fc00000, 0xffc00000])


def case(r):
    op = r.deal(OPS)
    if op in ('AtX', 'AtZ'):
        a, b = [coordinate(r) for _ in range(3)], [coordinate(r) for _ in range(3)]
        k = 0 if op == 'AtX' else 2
        plane = coordinate(r)
        roll = r.random()
        if roll < 0.1:
            b[k] = a[k]
        elif roll < 0.2:
            plane = r.choice([a[k], b[k]])
        return Call(0x56d520 if op == 'AtX' else 0x56d590, [RAM] + a + b + [plane], {RAM: r.bytes(0x10)}, ret='a')
    if op == 'Dot':
        return Call(0x5812f0, [r.f() for _ in range(6)], {}, ret='d')
    if op == 'BoxExtend':
        box = [r.f() for _ in range(6)]
        p = [r.choice(box) if r.random() < 0.2 else r.f() for _ in range(3)]
        if r.random() < 0.3:  # a sorted box
            for i in range(3):
                lo, hi = sorted((box[i], box[3 + i]), key=bf)
                box[i], box[3 + i] = lo, hi
        return Call(0x57b410, [RAM] + p, {RAM: struct.pack('<6I', *box)}, conv='t')
    shared = [r.f(), r.f(), 0] if r.random() < 0.5 else []
    a, b, c = RAM, RAM + 0x10, RAM + r.choice([0x20, 0x20, 0])
    regions = {RAM: point(r, shared) + point(r, shared) + point(r, shared), 0x8c63b0: r.bytes(0x24)}
    return Call(0x581320, [a, b, c], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

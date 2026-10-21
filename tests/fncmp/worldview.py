# The world view object's ranges and the pair set at +0x30 of src/game/render/worldview (0x56b830-0x56b93d) against the originals: an object of random bytes, its
# range floats (+0x3c-+0x48) now and then ordinary floats or fncmp's specials; the arguments ordinary floats, specials (zeros, NaNs,
# signalling ones too, infinities) or steps that make the count tiny, huge (past 32 and 63 bits) or negative.
# Ray (0x56b940): a point along a view ray, see ray().
# usage: worldview.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Diagonal,RangeX,RangeY,Set3034,Ray)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

CALLS = {'Diagonal': 0x56b850, 'RangeX': 0x56b880, 'RangeY': 0x56b8d0, 'Set3034': 0x56b830, 'Ray': 0x56b940}
B = Bench([(0x56b830, 0x56b93e), (0x56b940, 0x56baf3), (0x578d20, 0x579020), (0x67a830, 0x67a860)])
VIEW, OUT, VEC, CAM, GRID, ORIGIN = RAM + 0x1000, RAM + 0x1400, RAM + 0x1500, RAM + 0x1600, RAM + 0x1700, RAM + 0x1800


def ray(r):
    """0x56b940 (thiscall (out, vector)): the view's heights (+0x30, +0x34) ordinary, equal now and then, or any; the camera matrix
    [0x87afe8] a turn about y and a position, or any floats; the vector unit-ish, flat (y 0) now and then, or any; [0x8bd4c0] (the
    reach) ordinary or any; [0x879d60] a grid with an origin (only read; now and then none, a crash in both where it is read)."""
    import math
    view = bytearray(r.bytes(0x60))
    a = ordinary(r)
    put(view, 0x30, '2I', a, r.choice([a, ordinary(r), value(r)]) if r.random() < 0.9 else value(r))
    if r.random() < 0.7:
        t = r.uniform(-4, 4)
        c, s = math.cos(t), math.sin(t)
        m = [fb(x) for x in [c, 0, s, 0, 0, 1, 0, 0, -s, 0, c, 0, r.uniform(-500, 500), r.uniform(0, 300), r.uniform(-500, 500), 1]]
    else:
        m = [value(r) for _ in range(16)]
    vec = [fb(r.uniform(-1, 1)) for _ in range(4)]
    if r.random() < 0.15:
        vec[1] = r.choice([0, 0x80000000])
    if r.random() < 0.1:
        vec[r.randrange(3)] = value(r)
    regions = {VIEW: view, OUT: r.bytes(0x10), VEC: struct.pack('<4I', *vec), CAM: struct.pack('<16I', *m),
               GRID: put(bytearray(r.bytes(0x20)), 0, 'I', ORIGIN if r.random() < 0.97 else 0), ORIGIN: r.bytes(0x20),
               0x87afe8: struct.pack('<I', CAM), 0x879d60: struct.pack('<I', GRID),
               0x8bd4c0: struct.pack('<I', r.choice([fb(r.uniform(0, 2000)), ordinary(r), value(r)]))}
    return Call(0x56b940, [VIEW, OUT, VEC], regions, conv='t', ret='a')
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def ordinary(r):
    return fb(r.choice([r.uniform(-5000.0, 5000.0), r.uniform(-1.0, 1.0), float(r.randrange(-256, 257)), r.uniform(-1e30, 1e30)]))


def value(r):
    return r.choice([ordinary(r), ordinary(r), ordinary(r), r.f(), r.choice([0x7fa00000, 0xff800001])])


def case(r):
    op = r.deal(OPS)
    if op == 'Ray':
        return ray(r)
    obj = bytearray(r.bytes(0x2d8))
    for off in (0x3c, 0x40, 0x44, 0x48):
        if r.random() < 0.85:
            put(obj, off, 'I', value(r))
    if op == 'Set3034':
        return Call(CALLS[op], [RAM, r.getrandbits(32), r.getrandbits(32)], {RAM: obj}, conv='t', ret='a')
    if op == 'Diagonal':
        return Call(CALLS[op], [RAM], {RAM: obj}, conv='t')
    start, end = value(r), value(r)
    step = r.choice([value(r), fb(r.choice([0.5, 1.0, 2.0, 64.0, 1e-6, 1e-30, -1.0, -0.25])), 0, 0x80000000, 0x7fc00000, 0x7fa00000])
    return Call(CALLS[op], [RAM, start, end, step], {RAM: obj}, conv='t')


if __name__ == '__main__':
    B.main(case)

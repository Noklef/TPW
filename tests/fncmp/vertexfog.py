# The vertex fog (src/game/render/vertexfog: 0x5747d0, 0x574960) against the originals: fogs from start to end (and backwards, empty
# or NaN now and then), their colours in 0-255 or any floats, depths before, at, between and past the ends (and any floats), any
# colours, 0-6 vertices at strides that overlap them now and then, and the fistp dword checked after.
# usage: vertexfog.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Colour,Alpha)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, bf, fb  # noqa: E402

B = Bench([(0x5747d0, 0x5749db)])
OPS = [k for k in os.environ.get('OPS', 'Colour,Alpha').split(',') if k]
FOG, SCRATCH = 0x8bd47c, 0xfb34a4


def case(r):
    op = r.deal(OPS)
    start = r.uniform(-50, 200)
    end = start + r.choice([r.uniform(1, 400), r.uniform(-50, 0), 0.0, r.uniform(0, 1e-3)])
    start, end = fb(start), fb(end)
    if r.random() < 0.05:
        start, end = r.choice([(r.f(), end), (start, r.f()), (r.f(), r.f())])
    span = bf(end) - bf(start)
    fraction = fb(1 / span) if span and span == span and r.random() < 0.9 else r.f()
    colour = [fb(r.choice([r.uniform(0, 255), float(r.randrange(256)), r.uniform(-300, 600)])) if r.random() < 0.95 else r.f()
              for _ in range(3)]
    fog = struct.pack('<I4I4I', r.getrandbits(32), r.getrandbits(32), *colour, start, end, fraction, r.getrandbits(32))
    count = r.choice([0, 1, 2, 3, 4, 6])
    stride = r.choice([0x18, 0x20, 0x18, 4, 8, 0, -0x18])
    base = RAM + 0x100
    buf = bytearray(r.bytes(0x200))
    for k in range(count):
        off = base - RAM + k * stride
        lo, hi = sorted((bf(start), bf(end))) if bf(start) == bf(start) and bf(end) == bf(end) else (0, 100)
        z = r.choice([fb(r.uniform(lo - 20, hi + 20)), start, end, fb(r.uniform(lo, hi)), r.f()])
        struct.pack_into('<I', buf, off + 8, z)
    regions = {RAM: buf, FOG: fog, SCRATCH: r.bytes(4)}
    return Call(0x5747d0 if op == 'Colour' else 0x574960, [base, count, stride], regions, conv='s')


if __name__ == '__main__':
    B.main(case)

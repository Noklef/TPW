# The mesh builder's geometry helpers (src/game/math/geometry: 0x5812f0, 0x581320) against the originals: any floats (special values
# now and then), points sharing coordinates so the winding products tie, and NaNs.
# usage: geometry.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Dot,Winding)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x5812f0, 0x581315), (0x581320, 0x581403)])
OPS = [k for k in os.environ.get('OPS', 'Dot,Winding').split(',') if k]


def point(r, shared):
    v = [r.f() for _ in range(4)]
    for i in (0, 1, 3):
        if shared and r.random() < 0.4:
            v[i] = r.choice(shared)
    return struct.pack('<4I', *v)


def case(r):
    if r.deal(OPS) == 'Dot':
        return Call(0x5812f0, [r.f() for _ in range(6)], {}, ret='d')
    shared = [r.f(), r.f(), 0] if r.random() < 0.5 else []
    a, b, c = RAM, RAM + 0x10, RAM + r.choice([0x20, 0x20, 0])
    regions = {RAM: point(r, shared) + point(r, shared) + point(r, shared), 0x8c63b0: r.bytes(0x24)}
    return Call(0x581320, [a, b, c], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

# The width x height x depth buffer methods (src/game/gfx/array3d: 0x5e7740, 0x5e77c0, 0x5e77f0,
# 0x5e7ef0) against the originals: sizes from -1 to 6, depths 1 to 4, without pixels now and then,
# the other buffer's rows narrower, equal or wider (so the destination steps back as well).
# usage: array3d.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Fill,CopyTo,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5e7740, 0x5e77b2), (0x5e77c0, 0x5e77ed), (0x5e77f0, 0x5e7838), (0x5e7ef0, 0x5e7f50)])
CALLS = {'Fill': 0x5e7740, 'CopyTo': 0x5e77c0, 'ReverseBytes': 0x5e77f0, 'MaxInto': 0x5e7ef0}
ONE, TWO, DATA1, DATA2 = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x2000


def buf(r, pixels, w=None, h=None, depth=None):
    b = bytearray(r.bytes(0x10))
    w = r.choice([0, 1, 2, 3, 5, 6, -1]) if w is None else w
    h = r.choice([0, 1, 2, 3, 4, 5, -1]) if h is None else h
    depth = r.choice([4, 4, 1, 1, 2, 3]) if depth is None else depth
    put(b, 0, 'iii', w, h, depth)
    put(b, 0xc, 'I', pixels)
    return b


def case(r):
    op = r.choice(OPS)
    pix = r.choice([DATA1, DATA1, DATA1, 0])
    one = buf(r, pix)
    regions = {ONE: one, DATA1: r.bytes(0x400), DATA2: r.bytes(0x400)}
    if op == 'Fill':
        return Call(CALLS[op], [ONE] + [r.getrandbits(32) for _ in range(4)], regions, conv='t', ret='a')
    if op == 'ReverseBytes':
        return Call(CALLS[op], [ONE], regions, conv='t', ret='a')
    regions[TWO] = buf(r, DATA2 + 0x200 if op == 'MaxInto' else DATA2, w=r.choice([0, 1, 2, 3, 5, 6, 8]), depth=r.choice([1, 2, 4]))
    return Call(CALLS[op], [ONE, TWO], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

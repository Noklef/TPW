# The clipper's new vertex on an edge (src/game/render/clipedge: 0x561c40, 0x561ea0, 0x562100, 0x562360, 0x5625c0, 0x562820) against the
# originals: two vertices of any floats (now and then the same, on the plane, or with the distance of one 0), colours and speculars the same
# or not (any dwords), strides of 0x18 and up by 8 (now and then one that runs on to a fault), the globals it keeps of any bytes.
# usage: clipedge.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x561c40, 0x562a6d)])
OPS = [0x561c40, 0x561ea0, 0x562100, 0x562360, 0x5625c0, 0x562820]
OUT, A, VB = RAM, RAM + 0x100, RAM + 0x200
PLANE = {0x561c40: 4, 0x561ea0: 4, 0x562100: 0, 0x562360: 0, 0x5625c0: 8, 0x562820: 8}   # the coordinate each plane is on


def vertex(r, scale):
    v = r.floats(bytearray(r.bytes(0x40)), range(0, 0x40, 4), scale)
    if r.random() < 0.2:   # the w and the coordinate the same (on the plane y = w and the like) or opposite
        sign = r.choice([0, 0x80000000])
        for off in (0, 4, 8):
            if r.random() < 0.5:
                put(v, off, 'I', struct.unpack_from('<I', v, 0xc)[0] ^ sign)
    return v


def case(r):
    op = r.deal(OPS)
    scale = r.choice([1, 100, None])
    a, b = vertex(r, scale), vertex(r, scale)
    k = r.random()
    if k < 0.05:
        b = bytearray(a)   # dA == dB: 0 / 0
    elif k < 0.1:
        put(b, PLANE[op], 'f', 0.0)   # (z = 0 for the near plane)
    for off in (0x10, 0x14):
        if r.random() < 0.3:
            b[off:off + 4] = a[off:off + 4]
        elif r.random() < 0.3:   # only some bytes differ
            b[off:off + 4] = a[off:off + 4]
            b[off + r.randrange(4)] ^= 1 << r.randrange(8)
    stride = r.choice([0x18, 0x20, 0x20, 0x28, 0x30, 0x38]) if r.random() < 0.97 else r.choice([0x1c, 0x10, 0])
    regions = {OUT: r.bytes(0x80), A: a, VB: b, 0x879960: r.bytes(0xc0), 0xfb34a0: r.bytes(8)}
    return Call(op, [OUT, A, VB, stride], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

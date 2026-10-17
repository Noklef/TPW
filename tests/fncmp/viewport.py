# The render view's geometry (src/game/render/viewport: 0x582eb0-0x5832dc) against the originals: screen sizes of any sign and size (and the
# 32-bit ones that round twice through a double quotient), windows as fractions (and any floats), the projection's and depth range's floats
# (special values now and then), the driver kind 0x8bd524 4 or not. The view is random bytes; the depth range maps only its 0x18 bytes.
# usage: viewport.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

B = Bench([(0x582eb0, 0x5830f0), (0x583220, 0x5832f0)])
OPS = [k for k in os.environ.get('OPS', 'SetScreen,SetWindow,ResetWindow,SetProjection,SetDepthRange').split(',') if k]
VIEW = RAM
FAR = RAM + 0x2000  # the depth range's view: only +0x8c8..+0x8e0 mapped


def size(r):
    return r.choice([r.randrange(0, 2049), r.randrange(-2048, 2049), r.getrandbits(32), r.getrandbits(32) | 0x40000000,
                     r.choice([0, 1, 640, 480, 800, 600, 1024, 768, 0x7fffffff, 0x80000000, 0xffffffff])]) & 0xffffffff


def fraction(r):
    return r.choice([fb(r.random()), fb(r.uniform(-1, 1)), fb(1.0), 0, r.f()])


def view(r):
    v = r.bytes(0x80)
    struct.pack_into('<2I', v, 8, size(r), size(r))
    for off in (0x10, 0x14, 0x40, 0x4c, 0x50, 0x54):
        if r.random() < 0.8:
            struct.pack_into('<I', v, off, r.f())
    return v


def case(r):
    op = r.deal(OPS)
    regions = {VIEW: view(r), 0x8bd524: struct.pack('<I', r.choice([4, 4, 1, 0, r.getrandbits(32)]))}
    if op == 'SetScreen':
        return Call(0x582eb0, [VIEW, size(r), size(r), r.getrandbits(32)], regions, conv='t')
    if op == 'SetWindow':
        return Call(0x582f70, [VIEW, fraction(r), fraction(r), fraction(r), fraction(r)], regions, conv='t')
    if op == 'ResetWindow':
        return Call(0x583080, [VIEW], regions, conv='t')
    if op == 'SetProjection':
        return Call(0x583220, [VIEW, r.getrandbits(32), r.f(), r.f()], regions, conv='t')
    near = r.f()
    far = r.choice([r.f(), near, near ^ 0x80000000])
    return Call(0x583280, [FAR, near, far], {FAR + 0x8c8: r.bytes(0x18)}, conv='t')


if __name__ == '__main__':
    B.main(case)

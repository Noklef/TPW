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


def tie_quotient(r):
    """height, width (dwords) whose quotient's bits below the 24 kept are exactly a half with more beyond: the sticky bit decides."""
    while True:
        h, w = r.getrandbits(32), r.getrandbits(32) | 0x80000000
        a, b = abs(h - (h >> 31 << 32)), abs(w - (w >> 31 << 32))
        if not a:
            continue
        shift = 63 - a.bit_length()
        q, rem = divmod(a << shift, b)
        cut = q.bit_length() - 24
        if rem and q & ((1 << cut) - 1) == 1 << (cut - 1):
            return h, w


def tie_product(r):
    """a width (dword) and a fraction (float bits) whose exact product is just above a 24-bit midpoint and rounds onto it in a double."""
    while True:
        m = r.randrange(1 << 23, 1 << 24) | 1
        w = ((1 << 30) + 1) * pow(m, -1, 1 << 31) % (1 << 31)
        if (w * m).bit_length() == 55 and not (w * m >> 31) & 1:
            return w, fb(m / float(1 << 24))


SNAN = [0x7f800001, 0xff812345, 0x7fbfffff]


def case(r):
    op = r.deal(OPS)
    regions = {VIEW: view(r), 0x8bd524: struct.pack('<I', r.choice([4, 4, 1, 0, r.getrandbits(32)]))}
    if op == 'SetScreen':
        height, width = tie_quotient(r) if r.random() < 0.2 else (size(r), size(r))
        return Call(0x582eb0, [VIEW, width, height, r.getrandbits(32)], regions, conv='t')
    if op == 'SetWindow':
        args = [fraction(r), fraction(r), fraction(r), fraction(r)]
        if r.random() < 0.2:
            width, args[2] = tie_product(r)
            struct.pack_into('<I', regions[VIEW], 8, width)
        return Call(0x582f70, [VIEW] + args, regions, conv='t')
    if op == 'ResetWindow':
        return Call(0x583080, [VIEW], regions, conv='t')
    if op == 'SetProjection':
        return Call(0x583220, [VIEW, r.getrandbits(32), r.f(), r.f()], regions, conv='t')
    near = r.choice(SNAN) if r.random() < 0.1 else r.f()
    far = r.choice(SNAN) if r.random() < 0.1 else r.choice([r.f(), near, near ^ 0x80000000])
    return Call(0x583280, [FAR, near, far], {FAR + 0x8c8: r.bytes(0x18)}, conv='t')


if __name__ == '__main__':
    B.main(case)

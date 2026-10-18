# Transformed vertices packed to fixed point in place (src/game/render/fixedverts: 0x563220) against the original: 0-9 vertices of
# floats (any, small, ones about the __ftol limits, NaNs and infinities) and a colour dword, 0x20 apart as the one caller has them or any
# other stride (overlapping the 0x18-byte packed ones: 0, 4, 0x10, 0x18, ...), and now and then a count that runs off the buffer.
# Also the rounding packer (0x563130: x and y * 8 or 16 by the flags 0x8bd508's 0x80000020, z * 32768, u and v * 16, each rounded to
# nearest through the dwords at 0xfb34a0 / 0xfb34a4), with halves that tie.
# usage: fixedverts.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

B = Bench([(0x563220, 0x5632a1), (0x563130, 0x563213), (0x67a830, 0x67a857)])
EDGE = [0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000, 0x80000000, 0, 0x7f7fffff, 0x4f000000, 0xcf000000, 0x5f000000, 0x7f800001]


def value(r, scale):
    k = r.random()
    if k < 0.1:
        return r.choice(EDGE)
    if k < 0.2:
        return fb(r.uniform(-1, 1) * 2.0 ** 31 / scale * r.choice([0.99, 1.0, 1.01, 2.0]))   # about where __ftol overflows
    if k < 0.25:
        return r.getrandbits(32)
    if k < 0.32:
        return fb((r.randrange(-300, 300) + 0.5) / scale)   # (ties for the rounding packer)
    return r.f()


def case(r):
    op = r.deal([0x563220, 0x563130])
    flags = r.getrandbits(32) & ~0x80000020 | r.choice([0, 0x80000020, 0x80000020, 0x20, 0x80000000])
    xy = 8 if flags & 0x80000020 == 0x80000020 else 16
    scales = ((0, 32), (4, 16), (8, 65535)) if op == 0x563220 else ((0, xy), (4, xy), (8, 32768))
    count = r.deal([0, 1, 1, 2, 3, 4, 5, 9])
    stride = r.deal([0x20, 0x20, 0x20, 0x18, 0x1c, 0x24, 0x10, 4, 0, 0x40])
    size = max(stride, 0x18) * count + 0x40
    verts = r.bytes(size)
    for k in range(count):
        base = stride * k
        for off, scale in scales + ((0x18, 16), (0x1c, 16), (0xc, 1)):
            struct.pack_into('<I', verts, base + off, value(r, scale))
    if r.random() < 0.02:
        count = r.choice([0x100, 0x7fffffff])   # off the end: faults in both
    regions = {RAM: verts}
    if op == 0x563130:
        regions.update({0x8bd508: struct.pack('<I', flags), 0xfb34a0: r.bytes(8)})
    return Call(op, [RAM, count, stride], regions, ret='v')

if __name__ == '__main__':
    B.main(case)

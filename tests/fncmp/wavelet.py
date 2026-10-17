# The wavelet codec (src/game/gfx/wavelet) against the original:
# - the coefficient writer (0x591d30): values at and around the byte range's edges, cursors into a buffer and over the cursor itself;
# - the encoder's steps: pixels unpacked into float planes (0x5925e0; planes now and then the same buffer, strides below the width),
#   RGB to YUV in place (0x592150; any floats, planes sharing a buffer), the forward D4 row step (0x5924f0; the game's filter or any
#   floats, n a power of two or not, zero and negative) and the 2 x 2 halving (0x592670; odd sizes too).
# usage: wavelet.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=PUT,UNPACK,YUV,D4ROW,HALVE)
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402

B = Bench([(0x591d30, 0x591d6c), (0x592150, 0x592211), (0x5924f0, 0x592726)])
OPS = [k for k in os.environ.get('OPS', 'PUT,UNPACK,YUV,D4ROW,HALVE').split(',') if k]
CURSOR = RAM + 8
CODEC, PLANES, SRC, DST, FILTER = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, 0xc27208
S3 = math.sqrt(3)
D4 = struct.pack('<4f', (1 + S3) / (4 * math.sqrt(2)), (3 + S3) / (4 * math.sqrt(2)), (3 - S3) / (4 * math.sqrt(2)),
                 (1 - S3) / (4 * math.sqrt(2)))


def put_coefficient(r):
    mem = bytearray(r.bytes(0x40))
    edge = r.choice([-0x81, -0x80, -0x7f, -0x7e, 0, 0x7e, 0x7f, 0x80, 0x81, -0x8000, 0x7fff])
    value = r.choice([edge, r.randrange(-0x200, 0x200), r.getrandbits(16) << 16 | edge & 0xffff, r.getrandbits(32)])
    put(mem, CURSOR - RAM, 'I', RAM + r.choice([0x10, 0x20, 0x3d, r.randrange(5, 12)]))   # 5-11: over the cursor
    return Call(0x591d30, [value & 0xffffffff, CURSOR], {RAM: mem}, conv='s')


def codec(r, size):
    """the codec's 0x30 bytes with three planes of size bytes each at PLANES (now and then two or all three the same)."""
    obj = r.bytes(0x30)
    starts = [PLANES, PLANES + 0x400, PLANES + 0x800]
    if r.random() < 0.15:
        starts[r.randrange(3)] = starts[r.randrange(3)]
    for off, start in zip((0x2c, 0x1c, 0xc), starts):
        put(obj, off, 'I', start)
    planes = r.floats(bytearray(0xc00), range(0, 0xc00, 4))
    return obj, planes


def case(r):
    op = r.deal(OPS)
    if op == 'PUT':
        return put_coefficient(r)
    if op == 'UNPACK':
        width, height = r.randrange(0, 7), r.choice([0, 1, 2, 3, 5])
        stride = r.choice([width, width + r.randrange(1, 4), r.randrange(0, 8)])
        obj, planes = codec(r, 0x400)
        return Call(0x5925e0, [CODEC, width, height, stride, SRC], {CODEC: obj, PLANES: planes, SRC: r.bytes(max(1, width * height * 3))},
                    conv='t')
    if op == 'YUV':
        obj, planes = codec(r, 0x400)
        return Call(0x592150, [CODEC, r.choice([0, 1, 2, r.randrange(3, 40)])], {CODEC: obj, PLANES: planes}, conv='t')
    if op == 'D4ROW':
        n = r.choice([1, 2, 4, 8, 16, 32, 3, 6, 0, -2])
        size = 4 * max(4, 2 * abs(n) + 4)
        filt = D4 if r.random() < 0.6 else r.floats(bytearray(0x10), range(0, 0x10, 4))
        return Call(0x5924f0, [CODEC, SRC, DST, n & 0xffffffff],
                    {CODEC: r.bytes(0x10), SRC: r.floats(bytearray(size), range(0, size, 4)), DST: r.floats(bytearray(size), range(0, size, 4)),
                     FILTER: filt}, conv='t')
    n = r.choice([0, 1, 2, 3, 4, 5, 8, r.randrange(6, 16)])
    size = 4 * max(1, n * n)
    plane = r.floats(bytearray(size), range(0, size, 4), r.choice([1, 300, None]))
    return Call(0x592670, [CODEC, CODEC, n], {CODEC: struct.pack('<I', PLANES) + r.bytes(0xc), PLANES: plane}, conv='t')


if __name__ == '__main__':
    B.main(case)

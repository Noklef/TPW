# The wavelet codec (src/game/gfx/wavelet) against the original:
# - the coefficient writer (0x591d30): values at and around the byte range's edges, cursors into a buffer and over the cursor itself;
# - the encoder's steps: pixels unpacked into float planes (0x5925e0; planes now and then the same buffer, strides below the width),
#   RGB to YUV in place (0x592150; any floats, planes sharing a buffer), the forward D4 row step (0x5924f0; the game's filter or any
#   floats, n a power of two or not, zero and negative) and the 2 x 2 halving (0x592670; odd sizes too);
# - the 2-D forward D4 (0x592220; strides above n, a failing allocation now and then) and the grey and colour encoders (0x591d70, 0x591ab0):
#   any sizes up to 20 (sides 8 to 32), planes empty or with blocks to fit in or grow from, any quantisers; the compressor is a recorder,
#   so its arguments (the stream's length) and the stream at 0xa067f8 are compared. Now and then the grey one gets one pixel and a
#   quantiser that puts a coefficient just inside or outside the byte range;
# - the decoder's steps: the coefficient reader (0x592b80; its tables built or not, true or any floats, escapes often, the destination
#   now and then over the stream, a count of 0 now and then), the inverse D4 along a row (0x592c50; n below and above 6, odd, zero and
#   negative, the destination now and then over the source) and down the columns (0x5933a0; widths above n, in place now and then);
# - the grey decoder (0x592e60): sizes as the encoders', the plane at +0x2c empty or a block to fit in or grow from, the one at +0x3c any
#   floats, a stream of any bytes (escapes often) as the decompressor recorder leaves it, the coefficient tables built or not, any quantisers;
# - YUV to BGR pixels (0x5935a0): sizes odd and even (now and then 0 or 1: 2^32 loops, sic), luma strides at, above and below the width,
#   Cr above or below Cb (now and then not a dword apart), Y and chroma floats in range or any, the clamp tables any bytes.
# usage: wavelet.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=PUT,UNPACK,YUV,D4ROW,HALVE,FWD,GREY,ENCODE,READ,IROW,ICOLS,DGREY,RGB)
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, fb, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x591ab0, 0x592010), (0x592150, 0x593ab0), (0x598630, 0x598790), (0x5f8050, 0x5f8120)])
OPS = [k for k in os.environ.get('OPS', 'PUT,UNPACK,YUV,D4ROW,HALVE,FWD,GREY,ENCODE,READ,IROW,ICOLS,DGREY,RGB').split(',') if k]
CURSOR = RAM + 8
CODEC, PLANES, SRC, DST, FILTER = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, 0xc27208
OUT, SIZE, OLD, STREAM = RAM + 0x4000, RAM + 0x4100, RAM + 0x5000, 0xa067f8
GUARD, COMPRESSOR_VTABLE = 0xfa75f8, RAM + 0x4200   # Buffer_Compress's codec (built: guard bit 0) and its vtable of recorders
DPLANE = RAM + 0x8000   # the decoders' row plane (+0x3c)
BUILT, TABLES = 0xf75ad4, 0xfb27e0   # the coefficient reader's tables: built flag; s8 * 256, s8, u8 (0x100 floats each)
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
    if op == 'RGB':
        return yuv_to_rgb(r)
    if op in ('READ', 'IROW', 'ICOLS', 'DGREY'):
        return decoder(r, op)
    if op != 'HALVE':
        return coder(r, op)
    n = r.choice([0, 1, 2, 3, 4, 5, 8, r.randrange(6, 16)])
    size = 4 * max(1, n * n)
    plane = r.floats(bytearray(size), range(0, size, 4), r.choice([1, 300, None]))
    return Call(0x592670, [CODEC, CODEC, n], {CODEC: struct.pack('<I', PLANES) + r.bytes(0xc), PLANES: plane}, conv='t')


def side(extent):
    bits, rest = 1, ((extent - 1) & 0xffffffff) >> 1
    while rest:
        bits, rest = bits + 1, rest >> 1
    return 1 << (max(bits, 3) & 31)


def extent(r):
    if r.random() < 0.06:
        return r.choice([0, 17, 20])
    return r.choice([r.randrange(1, 9), r.randrange(1, 9), r.randrange(9, 17), r.randrange(1, 17)])


def array(r, regions, at, count, k):
    """a {data, size, max, grow by} array for count elements: empty, or a block (at OLD + 0x800 k) that count fits in or has to grow from."""
    if count > 0x100 or r.random() < 0.5:
        return struct.pack('<4I', 0, 0, 0, r.choice([0, 0, 4, 0x40]))
    size = r.choice([0, 1, count // 2, count, r.randrange(0, 0x40)])
    cap = max(size, r.choice([size, size + 1, count, count + 1, count + 4, r.randrange(1, 0x110)]))
    regions[OLD + 0x800 * k] = r.floats(bytearray(4 * max(cap, 1)), range(0, 4 * max(cap, 1), 4))
    return struct.pack('<4I', OLD + 0x800 * k, size, cap, r.choice([0, 0, 1, 4, 0x40]))


def encoder(r, colour):
    width, height = extent(r), extent(r)
    n = max(side(width), side(height))
    count = n * n + (1 if colour else 0)
    obj = bytearray(r.bytes(0x58))
    log = log_region()
    regions = {CODEC: obj, SRC: r.bytes(max(1, width * height * (3 if colour else 1))), OUT: r.bytes(0x10), SIZE: r.bytes(4), LOG: log,
               GUARD: struct.pack('<II', 1 | r.getrandbits(8) & 0xfe, COMPRESSOR_VTABLE), COMPRESSOR_VTABLE: vtable({1: 4}),
               HEAP: heap(0x1000 + 16 * count + (12 * count if colour else 0)), FILTER: D4 if r.random() < 0.8 else r.floats(bytearray(0x10), range(0, 0x10, 4)),
               STREAM: r.bytes(3 * count * (3 if colour else 2) // 2 + 0x10)}
    for k, off in enumerate((0xc, 0x1c, 0x2c) if colour else (0x2c,)):
        obj[off:off + 0x10] = array(r, regions, off, count, k)
    quantisers = [r.choice([fb(r.uniform(0.5, 4)), fb(r.uniform(1, 64)), r.f(), fb(r.uniform(1e-3, 0.1))]) for _ in range(3)]
    return width, height, regions, quantisers


def coder(r, op):
    if op == 'FWD':
        n = r.choice([0, 1, 2, 4, 8, 8, 16, 3, 6])
        stride = r.choice([n, n, n + r.randrange(1, 4)])
        size = 4 * max(1, stride * n)
        plane = r.floats(bytearray(size), range(0, size, 4), r.choice([1, 300, None]))
        return Call(0x592220, [CODEC, CODEC + 0x2c, stride, n],
                    {CODEC: put(bytearray(r.bytes(0x58)), 0x2c, 'I', PLANES), PLANES: plane, HEAP: heap(0x1000, r.randrange(1, 5) if r.random() < 0.05 else 0),
                     FILTER: D4 if r.random() < 0.6 else r.floats(bytearray(0x10), range(0, 0x10, 4))}, conv='t')
    if op == 'GREY':
        width, height, regions, q = encoder(r, False)
        if r.random() < 0.2:   # one pixel p: the coefficient h0 h3 p / q lands at -128.5, -127.5, 127.5 or 128.5
            width = height = 1
            p = r.randrange(1, 0x100)
            regions[SRC], regions[FILTER] = bytes([p]), D4
            h = struct.unpack('<4f', D4)
            q[0] = fb(h[0] * h[3] * p / r.choice([-128.5, -127.5, 127.5, 128.5]))
        return Call(0x591d70, [CODEC, width, height, SRC, OUT, SIZE, q[0]], regions, conv='t', ret='a')
    width, height, regions, q = encoder(r, True)
    colour = r.choice([1, 1, 1, 0xff, 0x100, 0x1ff, 0])
    return Call(0x591ab0, [CODEC + 0x54, width, height, colour, SRC, OUT, SIZE] + q, regions, conv='t', ret='a')


def tables(r):
    """the coefficient reader's flag and tables: not built (any bytes, overwritten), or built, the true tables or any floats."""
    if r.random() < 0.4:
        return {BUILT: struct.pack('<I', 0), TABLES: r.bytes(0xc00)}
    if r.random() < 0.5:
        values = [float((i ^ 0x80) - 0x80) * 256 for i in range(0x100)] + [float((i ^ 0x80) - 0x80) for i in range(0x100)] + [float(i) for i in range(0x100)]
        return {BUILT: struct.pack('<I', r.choice([1, 1, 0x100])), TABLES: struct.pack('<768f', *values)}
    return {BUILT: struct.pack('<I', 1), TABLES: r.floats(bytearray(0xc00), range(0, 0xc00, 4))}


def decode_grey(r):
    width, height = extent(r), extent(r)
    n = max(side(width), side(height))
    count = n * n
    obj = bytearray(r.bytes(0x58))
    regions = {CODEC: obj, SRC: r.bytes(0x20), DST: r.bytes(max(1, width * height) + 0x10), LOG: log_region(),
               GUARD: struct.pack('<II', 1 | r.getrandbits(8) & 0xfe, COMPRESSOR_VTABLE), COMPRESSOR_VTABLE: vtable({1: 4, 2: 3}),
               HEAP: heap(0x1000 + 8 * count + 8 * n), FILTER: D4 if r.random() < 0.8 else r.floats(bytearray(0x10), range(0, 0x10, 4)),
               DPLANE: r.floats(bytearray(4 * count), range(0, 4 * count, 4)), **tables(r)}
    stream = bytearray(r.bytes(3 * count + 0x10))
    for k in range(len(stream)):
        if r.random() < 0.15:
            stream[k] = 0x80
    regions[STREAM] = stream
    obj[0x2c:0x3c] = array(r, regions, 0x2c, count, 0)
    put(obj, 0x3c, 'I', DPLANE)
    q = r.choice([fb(1.0), fb(r.uniform(0.5, 4)), fb(r.uniform(1, 64)), r.f()])
    return Call(0x592e60, [CODEC, width, height, SRC, DST, q], regions, conv='t')


def yuv_to_rgb(r):
    def size():
        return r.choice([2, 4, 6, 8, 3, 5, 7, 9, r.randrange(2, 13)]) if r.random() < 0.97 else r.choice([0, 1])
    width, height = size(), size()
    stride = r.choice([width, width, width + r.randrange(1, 4), max(0, width - r.randrange(1, 3))])
    luma, chroma = (PLANES, PLANES + 0x400), PLANES + 0x800
    cb, cr = (luma[1], chroma) if r.random() < 0.6 else (chroma, luma[1])
    if r.random() < 0.06:
        cr += r.choice([1, 2, 3])
    obj = bytearray(r.bytes(0x30))
    for off, at in ((0x2c, luma[0]), (0x1c, cb), (0xc, cr)):
        put(obj, off, 'I', at)
    planes = bytearray(0xc10)
    ranges = [(range(0, 0x400, 4), r.choice([None, 255, 255])), (range(0x400, 0xc10, 4), r.choice([None, 128, 128]))]
    for offsets, scale in ranges:
        r.floats(planes, offsets, scale)
    return Call(0x5935a0, [CODEC, width, height, stride, DST], {CODEC: obj, PLANES: planes, DST: r.bytes(max(1, width * height * 3) + 8),
                                                               0xc27008: r.bytes(0x280), 0xfb2768: r.bytes(0x60)}, conv='t')


def decoder(r, op):
    if op == 'DGREY':
        return decode_grey(r)
    if op == 'READ':
        count = r.choice([1, 2, r.randrange(1, 8), r.randrange(8, 41)]) if r.random() < 0.97 else 0
        mem = bytearray(r.bytes(0x200))
        start = 0x100 + r.randrange(0, 4)
        for k in range(start, 0x200):
            if r.random() < 0.3:
                mem[k] = 0x80
        put(mem, CURSOR - RAM, 'I', RAM + start)
        dst = DST if r.random() < 0.9 else RAM + start + r.randrange(0, 0x10)   # now and then over the stream
        q = r.choice([fb(1.0), fb(r.uniform(0.5, 64)), r.f()])
        return Call(0x592b80, [CODEC, CURSOR, dst, count, q], {**tables(r), RAM: mem, DST: r.bytes(4 * 0x30)}, conv='t')
    if op == 'IROW':
        n = r.choice([2, 4, 6, 8, 16, 32, 3, 5, 7, 10, 12, 0, 1, -2])
        size = 0x40 + 4 * max(abs(n), 2) + 0x10   # 0x40 before the row, for n <= 1's wrapped reads
        src = SRC + 0x40
        dst = DST + 0x40 if r.random() < 0.85 else src + 4 * r.randrange(-2, 4)   # now and then over the source
        return Call(0x592c50, [CODEC, src, dst, n & 0xffffffff],
                    {CODEC: r.bytes(0x10), SRC: r.floats(bytearray(size), range(0, size, 4)), DST: r.floats(bytearray(size), range(0, size, 4)),
                     FILTER: D4 if r.random() < 0.6 else r.floats(bytearray(0x10), range(0, 0x10, 4))}, conv='t')
    n = r.choice([4, 4, 8, 8, 16, 6, 10, 5]) if r.random() < 0.95 else r.choice([2, 3])   # 2 and 3: no row pairs, 2^32 of them (sic)
    width = n + r.choice([0, 0, 1, 3])
    size = 4 * width * n
    dst = DST if r.random() < 0.85 else PLANES   # now and then in place
    return Call(0x5933a0, [CODEC, CODEC, CODEC + 4, width, n],
                {CODEC: struct.pack('<II', PLANES, dst) + r.bytes(8), PLANES: r.floats(bytearray(size), range(0, size, 4)),
                 DST: r.floats(bytearray(size), range(0, size, 4)), FILTER: D4 if r.random() < 0.6 else r.floats(bytearray(0x10), range(0, 0x10, 4))},
                conv='t')


if __name__ == '__main__':
    B.main(case)

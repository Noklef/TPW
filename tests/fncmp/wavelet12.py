# The v0x12 wavelet decoder (src/game/gfx/wavelet: WaveletCodec_DecodeV12 0x593ab0) against the original, dealt over its four outputs (32-bit and 16-bit, with and without an
# alpha block), TexFormat_Table and Buffer_Decompress running as they are, the decompressor's slot a recorder that leaves the stream as it is. Sizes 1 to 16 (now and then
# up to 32, or one of them 0), so that the chroma planes run both inverse loops (below 6 values, side 8) and the pipelined one; coefficient streams with 0x80 escapes, small or
# any bytes; the coefficient tables built or not, true or any floats; any quantisers and filters for the 32-bit outputs, whose packing saturates. The 16-bit outputs index the
# pixel words by value: their streams, quantisers and filter stay small, so that the words' region holds what they read (the original faults out of it, as the port does).
# usage: wavelet12.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, fb, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x593ab0, 0x595920), (0x5f8050, 0x5f8120), (0x5839e0, 0x583a00)])
CODEC, SRC, ALPHA, COMPRESSOR_VTABLE, DPLANE, PIXELS = RAM, RAM + 0x1000, RAM + 0x1100, RAM + 0x1200, RAM + 0x8000, RAM + 0x10000
FILTER, STREAM, GUARD, BUILT, TABLES = 0xc27208, 0xa067f8, 0xfa75f8, 0xf75ad4, 0xfb27e0
Y, CB, CR, A, FLOATS, WORDS = 0xeb4a48, 0xa667f8, 0xcaf228, 0xef4a48, 0xd70a38, 0xced228
S3 = math.sqrt(3)
D4 = struct.pack('<4f', (1 + S3) / (4 * math.sqrt(2)), (3 + S3) / (4 * math.sqrt(2)), (3 - S3) / (4 * math.sqrt(2)), (1 - S3) / (4 * math.sqrt(2)))
KINDS = [(4, True), (4, False), (2, True), (2, False)]


def side(extent):
    bits, rest = 1, ((extent - 1) & 0xffffffff) >> 1
    while rest:
        bits, rest = bits + 1, rest >> 1
    return 1 << (max(bits, 3) & 31)


def coefficients(r, count, wide):
    """count coefficients: bytes, 0x80 escaping the two bytes after it; small ones, or (wide) any size."""
    out = bytearray()
    for _ in range(count):
        if r.random() < (0.15 if wide else 0.02):
            out += struct.pack('<Bh', 0x80, r.randrange(-0x8000, 0x8000) if wide else r.randrange(-300, 301))
        else:
            v = r.getrandbits(8) if wide else r.choice([0, 0, 0, r.randrange(-3, 4), r.randrange(-20, 21)])
            out.append(v & 0xff if v & 0xff != 0x80 else 0)
    return out


def tables(r, wide):
    """the coefficient reader's flag and tables: not built (any bytes, overwritten), or built: true, or (wide) any floats."""
    if r.random() < 0.4:
        return {BUILT: struct.pack('<I', 0), TABLES: r.bytes(0xc00)}
    if not wide or r.random() < 0.5:
        values = [float((i ^ 0x80) - 0x80) * 256 for i in range(0x100)] + [float((i ^ 0x80) - 0x80) for i in range(0x100)] + [float(i) for i in range(0x100)]
        return {BUILT: struct.pack('<I', r.choice([1, 1, 0x100])), TABLES: struct.pack('<768f', *values)}
    return {BUILT: struct.pack('<I', 1), TABLES: r.floats(bytearray(0xc00), range(0, 0xc00, 4))}


def plane(r, count):
    size = 4 * count + 0x100
    return r.floats(bytearray(size), range(0, size, 4))


def case(r):
    pixel, alpha = r.deal(KINDS)
    wide = pixel == 4 and r.random() < 0.5
    width, height = r.choice([(r.randrange(1, 9), r.randrange(1, 9)), (r.randrange(1, 17), r.randrange(1, 17))])
    if r.random() < 0.04:   # (a side of 32 costs four times a side of 16: the bench's time)
        width, height = r.randrange(17, 33), r.randrange(1, 33)
    if r.random() < 0.02:
        width, height = r.choice([(0, height), (width, 0)])
    n = max(side(width), side(height))
    half = n >> 1
    count = n * n + 2 * half * half
    stream = coefficients(r, count, wide)
    regions = {CODEC: put(bytearray(r.bytes(0x58)), 0x3c, 'I', DPLANE), SRC: r.bytes(0x20), COMPRESSOR_VTABLE: vtable({1: 4, 2: 3}), LOG: log_region(), HEAP: heap(0x1000 + 8 * n),
               GUARD: struct.pack('<II', 1 | r.getrandbits(8) & 0xfe, COMPRESSOR_VTABLE), FILTER: D4 if not wide or r.random() < 0.7 else r.floats(bytearray(0x10), range(0, 0x10, 4)),
               DPLANE: plane(r, n * n), STREAM: stream + r.bytes(0x10), Y: plane(r, n * n), CB: plane(r, half * half), CR: plane(r, half * half), A: plane(r, n * n), **tables(r, wide)}
    if alpha:
        regions[ALPHA] = r.bytes(0x20)
    if pixel == 4:
        regions[FLOATS] = r.bytes((width + 2) * (height + 2) * 16)
        regions[PIXELS] = r.bytes((width * height * 4 + 7 & ~7) + 0x10)
    else:
        regions[WORDS] = r.bytes(0x3800)   # (the words' middles 0xcef328, 0xcef728, 0xceff28, 0xcf0328)
        regions[0x8bd550] = put(bytearray(r.bytes(0xc8)), 4, 'I', r.randrange(0, 5))
        regions[PIXELS] = r.bytes(max(8 * ((width + 1) >> 1) * height, 2 * width * (height + 1)) + 0x10)
    if wide:
        q = [r.choice([fb(1.0), fb(r.uniform(0.5, 4)), fb(r.uniform(1, 64)), r.f()]) for _ in range(4)]
    else:
        q = [fb(r.uniform(0.25, 3)) if r.random() < 0.97 else r.f() for _ in range(4)]
    return Call(0x593ab0, [CODEC, width, height, pixel, SRC, ALPHA if alpha else 0, PIXELS] + q, regions, conv='t')


if __name__ == '__main__':
    B.main(case)

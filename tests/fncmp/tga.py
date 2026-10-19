# The TGA reader and its row reader (src/game/gfx/tga: 0x62fe00-0x6306b1, thiscall) against the originals: TGA files (raw or run-length, mapped,
# true colour or grey, 8 to 32 bits or odd, an ID field, a colour map used or skipped, either row order, mirrored or not, cut short now and then)
# in a memory stream (fncmp.memory_stream: +8 read, +0x10 seek); a destination of the image's own format or another (so the pixel conversion runs:
# its palette or RGB converter) of its size or not; the reader the game's vtable (0x703630). The game's four formats (0xfaa268-0xfaa2c7) are
# mapped as the game sets them. Rows read, mirrored and copied on their own too, from readers set as ReadHeader leaves them.
# Gap: a paletted destination only takes 8-bit images (a true-colour one needs the index converter's inverse map, pixconv's own).
# usage: tga.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Load,Decode,ReadHeader,Begin,ReadRow,ReadRunRow,MirrorRow,RowBytes,CopyRow)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, memory_stream, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pixconv import fmt  # noqa: E402

B = Bench([(0x62fe00, 0x6306b2), (0x630880, 0x630c97), (0x639230, 0x63b2c0), (0x642860, 0x6428a5), (0x5f5660, 0x5f568c), (0x5f7360, 0x5f739b),
           (0x67acb0, 0x67acb1), (0x67acc0, 0x67acc1)])
READER, OUT, DSTI, STREAM, CODEC, ROW, DATA, BITS = RAM, RAM + 0x60, RAM + 0x80, RAM + 0xc0, RAM + 0x100, RAM + 0x600, RAM + 0x1000, RAM + 0x4000
PIXELS = BITS + 0x1000   # (the destination's bits: room before them for a bottom-up copy with no rows)
VTABLE = 0x703630
FORMATS = 0xfaa268   # rgb888, argb8888, pal8, rgb555: masks, bit count, paletted
RGB888, ARGB8888, PAL8, RGB555 = (0xff0000, 0xff00, 0xff, 0, 24, 0), (0xff0000, 0xff00, 0xff, 0xff000000, 32, 0), (0, 0, 0, 0, 8, 1), (0x7c00, 0x3e0, 0x1f, 0, 16, 0)
CALLS = {'Load': 0x6300f0, 'Decode': 0x62fe00, 'ReadHeader': 0x630150, 'Begin': 0x630400, 'ReadRow': 0x630440, 'ReadRunRow': 0x630580,
         'MirrorRow': 0x630500, 'RowBytes': 0x6306a0, 'CopyRow': 0x6300c0}


def header(r):
    """A TGA header: (id length, map type, image type, map first, map length, map bits, x, y, width, height, bits, descriptor)."""
    kind = r.choice([1, 2, 2, 3, 9, 10, 10, 11, 0, r.randrange(256)])
    mapped = kind in (1, 9)
    maptype = 1 if mapped else r.choice([0, 0, 0, 1])
    if r.random() < 0.05:
        maptype = r.choice([0, 2, r.randrange(256)])
    maplen = r.choice([1, 16, 256, r.randrange(0x101)]) if maptype == 1 else r.choice([0, 0, r.randrange(16)])
    if r.random() < 0.05:
        maplen = r.getrandbits(16)
    mapbits = 24 if r.random() < 0.9 else r.choice([8, 16, 32, r.randrange(256)])
    bits = r.choice([8, 8, 15, 16, 24, 32, 24, 32, r.randrange(256)])
    desc = r.choice([0, 0x10, 0x20, 0x30]) | (r.randrange(256) if r.random() < 0.1 else 0)
    return (r.choice([0, 0, 3, r.randrange(256)]), maptype, kind, r.getrandbits(16), maplen, mapbits, r.getrandbits(16), r.getrandbits(16),
            r.choice([0, 1, 2, 7, 16, 33]), r.choice([0, 1, 2, 5, 12]), bits, desc)


def rows(r, h, run):
    """Pixel data for h's image: raw rows, or run-length packets that now and then run past the width."""
    width, height, bpp = h[8], h[9], h[10] >> 3
    if not run:
        return r.bytes(width * height * bpp)
    out = bytearray()
    for _ in range(height):
        done = 0
        while done < width:
            n = r.randrange(1, min(width - done, 128) + 1) if r.random() < 0.9 else r.randrange(1, 129)
            if r.random() < 0.5:
                out += bytes([0x80 | (n - 1)]) + r.bytes(bpp)
            else:
                out += bytes([n - 1]) + r.bytes(n * bpp)
            done += n
    return bytes(out)


def tga(r, h):
    data = struct.pack('<3B2HB4H2B', *h) + r.bytes(h[0])
    if h[1] == 1:
        data += r.bytes(min(h[4], 0x100) * 3)
    data += rows(r, h, h[2] >= 9)
    if r.random() < 0.15:
        data = data[:r.randrange(len(data) + 1)]
    return data


def source_format(bits):
    return {8: PAL8, 15: RGB555, 16: RGB555, 24: RGB888, 32: ARGB8888}.get(bits)


def dest(r, h):
    """The destination SurfaceInfo: the image's format or another (paletted only for an 8-bit image), its size or not."""
    own = source_format(h[10])
    if own and r.random() < 0.5:
        f = own
    else:
        f = fmt(r) if r.random() < 0.8 else PAL8
        if f[5] and h[10] != 8:
            f = RGB888
    width = r.choice([0, 1, h[8], h[8], h[8] + 3, 40])
    height = r.choice([0, 1, h[9], h[9], h[9] + 2, 16])
    pitch = ((f[4] + 7) >> 3) * width + r.choice([0, 0, 4])
    return struct.pack('<6I4I', *f, PIXELS, width, height, pitch)


def reader(r, h, row=ROW):
    """A reader as ReadHeader leaves one (now and then a run-length flag past 1, or rows left set)."""
    b = bytearray(r.bytes(0x44))
    put(b, 0, '2I', VTABLE, row)
    put(b, 0x14, 'I', int(not h[11] & 0x20))
    put(b, 0x20, '2I', int(h[2] >= 9) if r.random() < 0.95 else r.choice([2, r.getrandbits(32)]), int(bool(h[11] & 0x10)))
    put(b, 0x28, 'I', r.choice([h[9], 0, 1, r.getrandbits(32)]))
    put(b, 0x2c, 'I', h[8] * (h[10] >> 3))
    b[0x30:0x42] = struct.pack('<3B2HB4H2B', *h)
    return b


def case(r):
    op = r.deal(OPS)
    h = header(r)
    regions = {CODEC: r.bytes(0x450), FORMATS: struct.pack('<24I', *RGB888, *ARGB8888, *PAL8, *RGB555), HEAP: heap(), BITS: r.bytes(0x3000),
               ROW: r.bytes(0x800), OUT: r.bytes(4)}
    regions.update(global_heap())
    address = CALLS[op]
    if op in ('Decode', 'Begin', 'ReadRow', 'ReadRunRow', 'MirrorRow', 'RowBytes', 'CopyRow'):   # (a header ReadHeader passed, mostly)
        if r.random() < 0.9:
            h = h[:2] + (r.choice([1, 2, 3, 9, 10, 11]),) + h[3:10] + (r.choice([8, 16, 24, 32]),) + h[11:]
        data = rows(r, h, h[2] >= 9)
        if r.random() < 0.15:
            data = data[:r.randrange(len(data) + 1)]
    else:
        data = tga(r, h)
    regions[DATA] = data + bytes(1)
    regions[STREAM] = memory_stream(STREAM, DATA, len(data))
    regions[READER] = reader(r, h)
    if op in ('Load', 'Decode'):
        regions[DSTI] = dest(r, h)
    if op == 'Decode':   # (the codec as ReadHeader leaves it: the image's format and size)
        codec = bytearray(regions[CODEC])
        f = source_format(h[10]) or RGB888
        put(codec, 4, '6I', *f)
        put(codec, 0x20, '3I', h[8], h[9], ((f[4] + 7) >> 3) * h[8])
        regions[CODEC] = codec
    if op in ('Load', 'Decode'):
        return Call(address, [READER, OUT, DSTI, STREAM, CODEC], regions, conv='t', ret='a')
    if op == 'ReadHeader':
        return Call(address, [READER, OUT, STREAM, CODEC], regions, conv='t', ret='a')
    if op == 'MirrorRow':
        return Call(address, [READER, OUT, ROW + r.choice([0, 0, 1, 0x100])], regions, conv='t', ret='a')
    if op == 'RowBytes':
        return Call(address, [READER], regions, conv='t', ret='a')
    if op == 'CopyRow':   # (a copy of the row to the target, stepping either way)
        put(regions[READER], 0xc, 'I', r.choice([0, 1, 4, h[8] * (h[10] >> 3), 0x100]))
        put(regions[READER], 0x18, '2I', PIXELS + r.choice([0, 0x40, 0x400]), r.choice([0, 0x40, -0x40 & 0xffffffff, 0x100]))
        return Call(address, [READER], regions, conv='t')
    return Call(address, [READER, OUT, STREAM], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

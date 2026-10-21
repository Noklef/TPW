# The video library's flat macroblock fills (src/game/gfx/tgqvideo: 0x671340 at 16 bits, 0x671650 at 32) against the originals. The
# scale [0xfb7820] 1.0, 0.5, any or negative; the luma and chroma tables (0xfb7930, 0xfb7b30, 0xfb7d30) random; the six values random
# bytes in dwords whose upper bytes are any; the pitch the squares' width, wider, or odd (the rows then (pitch >> 1) * 2 or
# (pitch >> 2) * 4 apart while the lower squares start at pitch * 8, sic); the output buffer random around the 16 rows.
# Pack (0x66fd70): the channels 0-255, around 0, the table builder's ranges or any; the bits the usual formats (5 6 5, 5 5 5, 8 8 8,
# 4 4 4) or each 0-9 now and then 31 / 32 (shift counts past 31: the CPU's mod 32); the shifts theirs or 0-35. Tables (0x66fc10): the
# same formats and shifts, the three tables random before.
# Quality: 0-100 mostly (else any: the products wrapping), the dequantiser 0xfb7820 random before.
# usage: tgqvideo.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Fill16,Fill32,Pack,Tables,Quality)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'Fill16': 0x671340, 'Fill32': 0x671650, 'Pack': 0x66fd70, 'Tables': 0x66fc10, 'Quality': 0x671000}
B = Bench([(0x671340, 0x671532), (0x671650, 0x6717fc), (0x66fd70, 0x66ff96), (0x66fc10, 0x66fd69), (0x671000, 0x6710b9)])
FORMATS = [(5, 6, 5, 11, 5, 0), (5, 5, 5, 10, 5, 0), (8, 8, 8, 16, 8, 0), (4, 4, 4, 8, 4, 0), (5, 6, 5, 0, 5, 11)]


def pixel_format(r):
    if r.random() < 0.7:
        return list(r.choice(FORMATS))
    return [r.choice(list(range(10)) + [31, 32]) for _ in range(3)] + [r.randrange(36) for _ in range(3)]


def channel(r):
    return r.choice([r.randrange(256), r.randrange(-8, 9), r.randrange(-300, 300), 0, 255, -0x80000000, r.getrandbits(32)]) & 0xffffffff
OUT = RAM


def case(r):
    op = r.deal(OPS)
    if op == 'Quality':
        quality = r.randrange(101) if r.random() < 0.85 else r.choice([-1, 101, 200, r.getrandbits(32)])
        return Call(CALLS[op], [quality], {0xfb7820: r.bytes(0x100)}, ret='v')
    if op == 'Pack':
        return Call(CALLS[op], [channel(r), channel(r), channel(r), r.getrandbits(32)] + pixel_format(r), {}, conv='f', ret='a')
    if op == 'Tables':
        return Call(CALLS[op], [r.getrandbits(32)] + pixel_format(r), {0xfb7930: r.bytes(0x600)}, conv='f', ret='v')
    wide = op == 'Fill32'
    pitch = r.choice([0x40, 0x80, 0x42, 0x43, 0x41] if wide else [0x20, 0x40, 0x22, 0x21, 0x23])
    regions = {0xfb7820: struct.pack('<I', r.choice([0x10000, 0x8000, 0x18000, r.getrandbits(20), r.getrandbits(32)])),
               0xfb7930: r.bytes(0x600), OUT: r.bytes(pitch * 16 + 0x40)}
    values = [r.getrandbits(8) | r.choice([0, 0, r.getrandbits(24)]) << 8 for _ in range(6)]
    return Call(CALLS[op], values + [OUT, pitch], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

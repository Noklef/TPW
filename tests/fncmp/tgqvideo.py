# The video library's flat macroblock fills (src/game/gfx/tgqvideo: 0x671340 at 16 bits, 0x671650 at 32) against the originals. The
# scale [0xfb7820] 1.0, 0.5, any or negative; the luma and chroma tables (0xfb7930, 0xfb7b30, 0xfb7d30) random; the six values random
# bytes in dwords whose upper bytes are any; the pitch the squares' width, wider, or odd (the rows then (pitch >> 1) * 2 or
# (pitch >> 2) * 4 apart while the lower squares start at pitch * 8, sic); the output buffer random around the 16 rows.
# usage: tgqvideo.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Fill16,Fill32)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'Fill16': 0x671340, 'Fill32': 0x671650}
B = Bench([(0x671340, 0x671532), (0x671650, 0x6717fc)])
OUT = RAM


def case(r):
    op = r.deal(OPS)
    wide = op == 'Fill32'
    pitch = r.choice([0x40, 0x80, 0x42, 0x43, 0x41] if wide else [0x20, 0x40, 0x22, 0x21, 0x23])
    regions = {0xfb7820: struct.pack('<I', r.choice([0x10000, 0x8000, 0x18000, r.getrandbits(20), r.getrandbits(32)])),
               0xfb7930: r.bytes(0x600), OUT: r.bytes(pitch * 16 + 0x40)}
    values = [r.getrandbits(8) | r.choice([0, 0, r.getrandbits(24)]) << 8 for _ in range(6)]
    return Call(CALLS[op], values + [OUT, pitch], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

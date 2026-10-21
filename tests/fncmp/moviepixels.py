# The video library's pixel row writers and YUV tables (src/game/gfx/moviepixels: 0x676478 the doubled bytes, 0x6764bc the YUV pairs,
# 0x670350 the tables, x87) against the originals: random source rows; Double's count a multiple of 4 from 4 to 0x40 (the listing loops until 0); FromYuv's count 1-16 with
# random tables (the three input ones 256 dwords, the output ones covering every index a sum can make). Tables: the format's bits
# mostly 4-8 (else 0-9 or any), shifts 0-31 (now and then any), depth 15, 16, 24 or 32 (now and then any); the brightness mostly 100
# (else 0-255 or any: the products wrapping); the tables 0xfb8f30-0xfbb598 random before. MaskBits / MaskShift (0x6702d0 / 0x6702f0):
# channel masks (a run of 1-8 bits at 0-31) or any nonzero value (MaskShift loops for ever on 0).
# usage: moviepixels.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Double,FromYuv,Tables,MaskBits,MaskShift)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'Double': 0x676478, 'FromYuv': 0x6764bc, 'Tables': 0x670350, 'MaskBits': 0x6702d0, 'MaskShift': 0x6702f0}
B = Bench([(0x676478, 0x6764bc), (0x6764bc, 0x67658f), (0x670350, 0x670659), (0x6702d0, 0x6702eb), (0x6702f0, 0x670300)])
SOURCE, OUT = RAM, RAM + 0x1000


def tables_case(r):
    bits = [r.randrange(4, 9) if r.random() < 0.85 else r.choice([0, 9, r.getrandbits(32)]) for _ in range(3)]
    shifts = [r.randrange(32) if r.random() < 0.9 else r.getrandbits(32) for _ in range(3)]
    depth = r.choice([15, 16, 16, 24, 32, r.getrandbits(32)])
    fmt = struct.pack('<8I', *bits, *shifts, r.getrandbits(32), depth)
    settings = bytearray(r.bytes(0x80))
    settings[0x78:0x7c] = struct.pack('<I', r.choice([100] * 3 + [0, 50, 150, r.randrange(0x100), r.getrandbits(32)]))
    regions = {SOURCE: fmt, OUT: struct.pack('<I', OUT + 0x10) + bytes(12) + bytes(settings), 0xfb8f30: r.bytes(0x2668)}
    return Call(CALLS['Tables'], [SOURCE, OUT], regions, conv='f', ret='v')


def case(r):
    op = r.deal(OPS)
    if op in ('MaskBits', 'MaskShift'):
        mask = ((1 << r.randrange(1, 9)) - 1) << r.randrange(32) & 0xffffffff if r.random() < 0.7 else r.getrandbits(32)
        return Call(CALLS[op], [mask or 0x80000000], {}, conv='f', ret='a')
    if op == 'Tables':
        return tables_case(r)
    if op == 'Double':
        count = 4 * r.randrange(1, 0x11)
        return Call(CALLS[op], [SOURCE, OUT, count], {SOURCE: r.bytes(0x40), OUT: r.bytes(0x90)}, ret='v')
    count = r.choice([1, 2, 3, 8, r.randrange(1, 17)])
    regions = {SOURCE: r.bytes(0x40), OUT: r.bytes(0x48), 0xfb8f30: r.bytes(0x400), 0xfb9730: r.bytes(0x400),
               0xfb9f30: r.bytes(0x400), 0xfba730: r.bytes(0x10e0)}
    return Call(CALLS[op], [SOURCE, OUT, count], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

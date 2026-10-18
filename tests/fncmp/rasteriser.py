# The software rasteriser device's methods (src/game/render/rasteriser) against the originals: the span routine choice (0x569f60) with flags
# with and without 0x100, 0x200000, 0x200, 0x800, 0x10000000, 0x20000000 and 0x40000000, any routine table; the setting (0x569f30) with a value
# or 0; a pixel read (0x56a510) from a buffer 1 to 64 wide.
# usage: rasteriser.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Span,Setting,Pixel)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x569f60, 0x56a02e), (0x569f30, 0x569f58), (0x56a510, 0x56a540)])
BITS = [0x100, 0x200000, 0x200, 0x800, 0x10000000, 0x20000000, 0x40000000]
BUF = RAM + 0x1000


def span(r):
    flags = r.getrandbits(32) if r.random() < 0.3 else sum(b for b in BITS if r.random() < 0.5) | (r.getrandbits(32) & 0x8ff)
    return Call(0x569f60, [flags & 0xffffffff], {0x6f19f0: r.bytes(0x40), 0xfbe000: r.bytes(0x60)}, conv='s', ret='a')


def setting(r):
    regions = {0xfbe030: r.bytes(0x20), 0x6f1a10: r.bytes(8)}
    return Call(0x569f30, [r.getrandbits(32), r.choice([0, 0, 1, 7, r.getrandbits(32)])], regions, conv='s', ret='a')


def pixel(r):
    obj = bytearray(r.bytes(0x400))
    width = r.choice([1, 4, 16, 64])
    x, y = r.randrange(width), r.randrange(8)
    regions = {RAM: obj, BUF: r.bytes(0x400), 0x879da8: struct.pack('<IIII', 0, RAM + 0x800, 0, 0),
               0x8bcbd0: struct.pack('<II', width, 0)}
    regions[RAM + 0x800] = bytearray(obj)  # the renderer object: +8 = the buffer
    put(regions[RAM + 0x800], 8, 'I', BUF)
    return Call(0x56a510, [x, y, RAM + 0x200], regions, conv='s', ret='a')


def case(r):
    return {'Span': span, 'Setting': setting, 'Pixel': pixel}[r.deal(OPS)](r)


OPS = [k for k in os.environ.get('OPS', 'Span,Setting,Pixel').split(',') if k]

if __name__ == '__main__':
    B.main(case)

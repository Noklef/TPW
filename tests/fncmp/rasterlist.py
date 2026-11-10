# The software rasteriser's indexed triangle lists (src/game/render/rasterdraw: 0x6792f0, 0x679350) against the originals: the triangle routines 0x6ef7f0 / 0x6f0a50 cdecl recorders
# (the vertex pointers they get logged), the indices any words, the counts about the multiples of 3.
# usage: rasterlist.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

TRI, MAPPED = 1, 3   # (the driver's slots for kRasterDrawCalls)
B = Bench([(0x6792f0, 0x67934e), (0x679350, 0x6793ae)], hooks={0x6ef7f0: _recorder(TRI, 3, cdecl=True), 0x6f0a50: _recorder(MAPPED, 3, cdecl=True)})
VERTS, INDICES = RAM + 0x100, RAM + 0x2000


def case(r):
    regions = {LOG: log_region(), INDICES: struct.pack('<24H', *[r.choice([0, 1, 2, 3, r.randrange(0x40), r.getrandbits(16)]) for _ in range(24)])}
    count = r.choice([0, 1, 2, 3, 4, 5, 6, 7, 9, 16, 17, 18])
    return Call(r.choice([0x6792f0, 0x679350]), [r.getrandbits(32), r.getrandbits(32), VERTS, r.getrandbits(32), INDICES, count, r.getrandbits(32)], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

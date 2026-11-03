# The software rasteriser's pixel format and buffers (src/game/render/rasterformat: RasterFormat_Init 0x678e70) against the original. Agent B's 0x679150-0x679230 cdecl recorders (driver.cpp's
# kRasterFormatEnv), both sides answering the same for green's bit count when set; the surface's GetPixelFormat a COM recorder filling the masks; OutputDebugStringA a recorder of the line's hash; sprintf the real CRT's.
# usage: rasterformat.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, fill, import_table, log_region, put, set_return, strings, vtable  # noqa: E402

CRT = [(0x67a650, 0x67a6d0), (0x67ad90, 0x67ae10), (0x67dcd0, 0x67ddb0), (0x67efc0, 0x67f1b0), (0x67fcb0, 0x67fd70), (0x680ad0, 0x6816b0), (0x682460, 0x6824f0), (0x682720, 0x6827d0), (0x682b40, 0x682c10),
       (0x683aa0, 0x683ad0), (0x686440, 0x6866e0), (0x686980, 0x686d40), (0x687140, 0x687240), (0x687530, 0x6875a0), (0x688710, 0x688740), (0x68a170, 0x68a450), (0x68c320, 0x68c450), (0x68ead0, 0x68ebd0)]
B = Bench([(0x678e70, 0x679145)] + CRT, hooks={0x679150: _recorder(10, 1, cdecl=True), 0x679170: _recorder(11, 1, cdecl=True), 0x679190: _recorder(12, 0, cdecl=True),
                                               0x679230: _recorder(13, 2, cdecl=True), 0x6791f0: _recorder(14, 2, cdecl=True)})
GET = 21
GLOBALS = 0xfbe000
OUT = RAM + 0x200


def case(r):
    log = log_region()
    regions = {LOG: log, GLOBALS: bytearray(r.bytes(0x1b0)), 0xfac514: r.bytes(4)}
    regions[IMPORT_TABLE] = import_table(log, {'OutputDebugStringA': (20, 0x6fd1e0, 1, 1, 0)})
    surface = r.deal([0, RAM, RAM, RAM, RAM, RAM, RAM, RAM])
    if surface:
        mem = bytearray(0x300)
        put(mem, 0, 'I', RAM + 0x100)
        mem[0x100:0x100 + 0x80] = vtable({}, com={GET: 2})
        regions[RAM] = mem
        masks = r.choice([(0xf800, 0x7e0, 0x1f), (0x7c00, 0x3e0, 0x1f), (0xff0000, 0xff00, 0xff), (0x1f, 0x7e0, 0xf800)] + [tuple(r.getrandbits(32) for _ in range(3))] * 2 + [(0x80000000, 0x80000000 >> r.randrange(4), r.getrandbits(32) | 0xf0000000)])
        count = r.deal([16, 16, 16, 24, 32, 8, r.getrandbits(32)])
        fill(log, GET, 1, struct.pack('<4I', count, *masks), offset=0xc)
        strings(log, GET, 0, 1 << 1)
        blob(log, GET, 1, 0xe7)   # (size, flags, fourCC and the alpha mask and its next: what the code set; the masks are the fill's)
        set_return(log, GET, r.getrandbits(32))
    pick = r.deal([None, 5, 6, 4])
    if pick is not None:
        set_return(log, 10, pick)
    if r.getrandbits(1):
        set_return(log, 11, r.randrange(32))
    out = r.choice([0, OUT])
    if out or surface:
        regions.setdefault(RAM, bytearray(0x300))
    args = [surface, out, r.randrange(0x10000), r.randrange(0x10000)]
    return Call(0x678e70, args, regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

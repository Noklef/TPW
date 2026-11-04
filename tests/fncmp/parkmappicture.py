# The park map's picture loaded (src/game/world/parkmap: 0x5f1ea0) against the original: the strings and SurfaceInfo_Ctor real, the codec's and the bitmap's calls recorders.
# usage: parkmappicture.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, fill, global_heap, heap, log_region, mask_this, set_return, strings  # noqa: E402

FOLDER, INFO, ALLOC, LOAD, CONVERT, FREE = 8, 9, 10, 11, 12, 13
B = Bench([(0x5f1ea0, 0x5f2050), (0x5fa680, 0x5fa6a0), (0x5da3c0, 0x5da3c1)] + gamestring.B_RANGES,
          hooks={0x40ef40: _recorder(FOLDER, 0, cdecl=True), 0x5fa8a0: _recorder(INFO, 4), 0x5fa6f0: _recorder(ALLOC, 1), 0x5faa00: _recorder(LOAD, 5), 0x5fc360: _recorder(CONVERT, 4), 0x5fa740: _recorder(FREE, 0)})
MAP, TEXT, NAME, DATA = RAM, RAM + 0x100, RAM + 0x180, RAM + 0x1c0
SIZE_DWORDS = 0x3f0   # (bitCount on; the masks are stack garbage)


def case(r):
    log = log_region()
    table = bytearray(0x600)
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000)}
    set_return(log, FOLDER, TEXT)
    regions[TEXT] = r.choice([b'', b'maps', b'C:\\game\\data\\frontend\\parkmaps\\x']) + b'\0'
    gamestring.string(r, 0, NAME, DATA, regions, r.choice([b'', b'map.tga', b'a longer name of a map.tga']))
    regions[0xf86d20] = regions.pop(NAME)
    for k in INFO, LOAD, CONVERT, FREE, ALLOC:
        mask_this(log, k)
    fill(log, INFO, 3, struct.pack('<2I', r.getrandbits(32), r.getrandbits(32)), 0, 0x1c)
    strings(log, INFO, 2, 1)
    blob(log, INFO, 3, SIZE_DWORDS)
    strings(log, ALLOC, 0, 1)
    strings(log, LOAD, 4, 1)
    blob(log, LOAD, 1, SIZE_DWORDS)
    strings(log, CONVERT, 0, 1)
    blob(log, CONVERT, 2, SIZE_DWORDS)
    regions[MAP] = r.bytes(0x80)
    regions[0x8bd554] = struct.pack('<I', r.randrange(4))
    regions[0x8bd558] = r.bytes(0x80)
    return Call(0x5f1ea0, [MAP, r.choice([0, 0, 1, r.getrandbits(32)])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

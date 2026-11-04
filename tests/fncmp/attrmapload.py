# The base attribute map loaded (src/game/terrain/attrmap: 0x450900 TerrainAttrMap_Load) against the original: the file's text through the real sprintf,
# GamePaths_Base (k 3), Quickload_Locate (k 4, the text hashed) and TP2M_ReadChunks (k 5, on the game's +0x2d8) recorders; whether it answered.
# usage: attrmapload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return, strings  # noqa: E402

BASE, LOCATE, READ = 3, 4, 5
B = Bench([(0x450900, 0x45094e), (0x67a400, 0x690000)], hooks={0x40ef40: _recorder(BASE, 0, cdecl=True), 0x44a280: _recorder(LOCATE, 3, cdecl=True), 0x5192d0: _recorder(READ, 1)})
TEXT = RAM


def case(r):
    log = log_region()
    set_return(log, BASE, TEXT)
    strings(log, LOCATE, 1 << 0)
    set_return(log, LOCATE, r.choice([0, r.getrandbits(32)]))
    set_return(log, READ, r.choice([0, 1, r.getrandbits(32), 0x80000000]))
    regions = {LOG: log, TEXT: bytes(r.choice(b'abcXYZ') for _ in range(r.randrange(1, 40))) + bytes(1), 0x7cf83c: struct.pack('<I', r.getrandbits(32))}
    return Call(0x450900, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

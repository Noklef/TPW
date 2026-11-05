# The terrain texture's start from outside (src/game/terrain/procedural: 0x56dfe0 TerrainTex_InitGlobal) against the original: the flags [0x8bd550] and [0x8bd508],
# the object [0x879d60] or none; TerrainTex_Init (k 20) and EngineDebug_LogFallback (k 4) recorders.
# usage: terraintexglobal.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

INIT, FALLBACK = 20, 4
B = Bench([(0x56dfe0, 0x56e025)], hooks={0x56e550: _recorder(INIT, 2), 0x5645e0: _recorder(FALLBACK, 1, cdecl=True)})
OBJECT = RAM


def case(r):
    log = log_region()
    set_return(log, INIT, r.getrandbits(32))
    flags = struct.pack('<I', r.getrandbits(32) & ~0x10000000 if r.randrange(3) else r.getrandbits(32) | 0x10000000)
    regions = {LOG: log, OBJECT: r.bytes(0x10), 0x8bd550: flags, 0x8bd508: struct.pack('<I', r.choice([0, 0x2000000, r.getrandbits(32)])),
               0x879d60: struct.pack('<I', r.choice([0, OBJECT, OBJECT]))}
    return Call(0x56dfe0, [r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

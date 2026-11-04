# The texture cache's release and destruction and its surface pool's mip levels (src/game/gfx/texcache: 0x58d090, 0x58dc10; src/game/gfx/surfacepool: 0x58b220) against the originals: recorders for TexCache_ReleaseItems (k 1), Object_FreeSixPointerArrays (k 2),
# SurfacePool_ReleaseAll (k 3), Mem_Free (cdecl, k 4) and SurfacePool_MakeSurfaces (k 5, failing now and then).
# usage: texcachedestroy.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region  # noqa: E402

B = Bench([(0x58d090, 0x58d0e9), (0x58dc10, 0x58dc49), (0x58b220, 0x58b280)],
          hooks={0x58dd60: _recorder(1, 1), 0x58d380: _recorder(2, 0), 0x58b280: _recorder(3, 0), 0x67acb0: _recorder(4, 1, cdecl=True), 0x58b0b0: _recorder(5, 2)})
CACHE, OTHER, TABLE = RAM, RAM + 0x2000, RAM + 0x4000


def case(r):
    log = log_region()
    regions = {LOG: log, CACHE: r.bytes(0x18d0), OTHER: r.bytes(0x18d0), 0x8c4b44: struct.pack('<I', r.choice([0, CACHE, CACHE, CACHE]))}
    op = r.deal(['destroy', 'release', 'levels', 'levels'])
    if op == 'destroy':
        return Call(0x58d090, [], regions, ret='a')
    if op == 'release':
        return Call(0x58dc10, [r.choice([CACHE, OTHER])], regions, conv='t', ret='a')
    answers(log, 5, TABLE)
    regions[TABLE] = struct.pack('<40I', *[0 if r.random() < 0.1 else r.choice([1, 1, r.getrandbits(32)]) for _ in range(40)])
    side = r.choice([0, -1, 1, 2, 3, 0x20, 0x40, 0x100, 0x400, 0x401, r.getrandbits(32), r.getrandbits(16)])
    levels = r.choice([0, 0, 1, 2, 3, 4, 10, 30, -5 & 0xffffffff, -6 & 0xffffffff, -4 & 0xffffffff])
    return Call(0x58b220, [CACHE, side, levels, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

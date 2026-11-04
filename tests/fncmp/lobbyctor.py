# The lobby game's constructor (src/game/frontend/lobbyscene: 0x5d84a0) against the original. Recorders: Class7029d0_Ctor 8, String_CtorStr 9, Strings5_JoinPath 10, String_Dtor 2, Terrain_Load 11,
# Obj_RemoveFromRect 12, Holder_Sleep 21. The string is a local (its address not compared). The renderer's flag 0x4000 set or not, the two small blocks there or not.
# usage: lobbyctor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, mask_this, put, strings  # noqa: E402

BASE, CTOR, JOIN, DTOR, TERRAIN, RECT, SLEEP = 8, 9, 10, 2, 11, 12, 21
B = Bench([(0x5d84a0, 0x5d85b0), (0x67acc0, 0x67acc1)],
          hooks={0x5d5700: _recorder(BASE, 0), 0x5f60b0: _recorder(CTOR, 1), 0x5c5580: _recorder(JOIN, 1), 0x5f61e0: _recorder(DTOR, 0), 0x4504c0: _recorder(TERRAIN, 2, cdecl=True),
                 0x45be80: _recorder(RECT, 3, cdecl=True), 0x45bdd0: _recorder(SLEEP, 1, cdecl=True)})
OBJECT, HOLDER = RAM, RAM + 0x100


def case(r):
    log = log_region()
    mask_this(log, CTOR)
    mask_this(log, DTOR)
    strings(log, JOIN, 0, out=1)
    holder = bytearray(r.bytes(0xd0))
    put(holder, 4, 'I', r.getrandbits(32) | 0x4000 if r.random() < 0.5 else r.getrandbits(32) & ~0x4000)
    regions = {LOG: log, HEAP: heap(fail=r.choice([0, 0, 0, 1, 2])), OBJECT: r.bytes(0x20), HOLDER: holder, 0x0079fcb4: struct.pack('<I', HOLDER)}
    return Call(0x5d84a0, [OBJECT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

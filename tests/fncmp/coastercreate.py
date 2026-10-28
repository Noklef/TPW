# A new coaster link made (src/game/coaster/registry: CoasterLink_Create 0x4368f0) against the original. Its calls are cdecl recorders
# (the driver's CoasterLinksEnv): GlobalMem_Alloc 0x45b9d0 (15 of 2, answering the link's block, then the buffer's), IdTable_Add 0x468760
# (16 of 7, an id or now and then 0), GlobalMem_Free 0x45ba00 (17 of 1), CoasterRing_Init 0x43b080 (18 of 2), Coaster_SetState 0x435730
# (19 of 2), Coaster_SetMix 0x43c3d0 (20 of 3), CoasterLink_SetUpCars 0x436d10 (21 of 3), Coaster_SetupCars 0x43ba90 (22 of 4) and
# CoasterLink_AddFixedPieces 0x436ac0 (23 of 5). The types 0x790b08 (0x790fe8 of them, up to eight; the id asked for one's +4 mostly,
# else none: a fault in both), flags byte bits 2 / 3 either way; the ride table 0x7a4610's holders (+8 -> +4 -> +0x78 -> +0x44 a float, a
# signalling NaN now and then); the loaded flag (fifth argument) 0 mostly. With it set the answer is the caller's ECX (sic): not compared.
# usage: coastercreate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fail_after, log_region, put, set_return  # noqa: E402

HOOKS = {0x45b9d0: _recorder(15, 2, cdecl=True), 0x468760: _recorder(16, 7, cdecl=True), 0x45ba00: _recorder(17, 1, cdecl=True),
         0x43b080: _recorder(18, 2, cdecl=True), 0x435730: _recorder(19, 2, cdecl=True), 0x43c3d0: _recorder(20, 3, cdecl=True),
         0x436d10: _recorder(21, 3, cdecl=True), 0x43ba90: _recorder(22, 4, cdecl=True), 0x436ac0: _recorder(23, 5, cdecl=True)}
RANGES = [(0x4368f0, 0x436ab9)]
B = Bench(RANGES, hooks=HOOKS)
LINK, BUF, HOLD, TABLE, TYPES = (RAM + k * 0x1000 for k in range(5))   # (TYPES: 0x2000 bytes)


def case(r):
    log = log_region()
    answers(log, 15, TABLE)
    if r.random() < 0.15:
        fail_after(log, 16, 0, 0)
    else:
        set_return(log, 16, r.randrange(1, 0x100))
    n = r.randrange(1, 9) if r.random() < 0.98 else 0
    types = bytearray(r.bytes(0x400 * 8))
    ids = [r.getrandbits(32) for _ in range(n)]
    for k in range(n):
        put(types, 0x400 * k + 4, 'I', ids[k])
        types[0x400 * k] = r.getrandbits(8)
    tid = r.choice(ids) if n and r.random() < 0.97 else r.getrandbits(32)
    hold = bytearray(r.bytes(0x400))
    for k in range(4):
        o = 0x100 * k
        put(hold, o + 8, 'I', HOLD + o + 0x20)
        put(hold, o + 0x20 + 4, 'I', HOLD + o + 0x40)
        put(hold, o + 0x40 + 0x78, 'I', HOLD + o + 0x80)
        r.floats(hold, [o + 0x80 + 0x44])
        if r.random() < 0.1:
            put(hold, o + 0x80 + 0x44, 'I', 0x7f800001 | r.getrandbits(22))
    loaded = 0 if r.random() < 0.75 else r.choice([1, r.getrandbits(32)])
    regions = {LOG: log, LINK: r.bytes(0x150), BUF: r.bytes(0x10), TYPES: types, HOLD: hold,
               TABLE: struct.pack('<40I', LINK, BUF, *[r.getrandbits(32) for _ in range(38)]),
               0x790b08: struct.pack('<8I', *[TYPES + 0x400 * k for k in range(8)]), 0x790fe8: struct.pack('<I', n),
               0x790fe0: r.bytes(4), 0x790fec: r.bytes(4), 0x792214: r.bytes(4),
               0x7a4610: struct.pack('<4I', *[HOLD + 0x100 * k for k in range(4)])}
    args = [tid, r.randrange(4), r.getrandbits(32), r.getrandbits(32), loaded, r.getrandbits(32)]
    return Call(0x4368f0, args, regions, ret='v' if loaded else 'a')


if __name__ == '__main__':
    B.main(case)

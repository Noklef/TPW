# The surface pool's surfaces released (src/game/gfx/surfacepool: 0x58b280 SurfacePool_ReleaseAll) against the original: the 32 pointers set
# or not, slots 0x98 and Release COM recorders; a Release's failure logged; the first call's result reaches no test (sic).
# usage: surfacepoolrelease.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put, recorder, set_return, vtable  # noqa: E402

DEBUG, MESSAGE = 3, 4
RELEASE, SLOT98 = 2, 7   # (slot 0x98's recorder: k 7, as recorders stop at 31)
B = Bench([(0x58b280, 0x58b2fd)], hooks={0x564650: _recorder(DEBUG, 1, cdecl=True), 0x56a770: _recorder(MESSAGE, 1, cdecl=True)})
SELF, SURFACE, VT, ANSWERS = RAM, RAM + 0x100, RAM + 0x140, RAM + 0x500


def case(r):
    log = log_region()
    set_return(log, MESSAGE, r.getrandbits(32))
    regions = {LOG: log, SURFACE: struct.pack('<I', VT) + r.bytes(0xc), VT: put(vtable({}, 40, com={RELEASE: 1}), 0x98, 'I', recorder(SLOT98, 2, True)), 0x879cd4: r.bytes(4),
               SELF: struct.pack('<32I', *[r.choice([0, SURFACE, SURFACE]) for _ in range(32)]) + r.bytes(0x10),
               ANSWERS: struct.pack('<40I', *[r.choice([0, 0, 1, r.getrandbits(31) | 0x80000000, r.getrandbits(31)]) for _ in range(40)]),
               ANSWERS + 0x100: struct.pack('<40I', *[r.getrandbits(32) for _ in range(40)])}
    answers(log, RELEASE, ANSWERS)
    answers(log, SLOT98, ANSWERS + 0x100)
    return Call(0x58b280, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

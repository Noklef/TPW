# The display's surfaces restored or opened again (src/game/render/ddopen: 0x564160 Display_RestoreOrOpen) against the original: [0x8bd524] 1 reopens with the
# view's globals (DirectDraw_Open a recorder, k 1); else slot 0x6c (k 27) of the surfaces at +0x10 and +0x14 with answers in turn, a failure logged
# (DxResult_Message k 2, EngineDebug_Log k 3); the status [0x879cd4].
# usage: displayrestore.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put, set_return, vtable  # noqa: E402

OPEN, MESSAGE, DEBUG, RESTORE = 1, 2, 3, 27
B = Bench([(0x564160, 0x564202)], hooks={0x563460: _recorder(OPEN, 6), 0x56a770: _recorder(MESSAGE, 1, cdecl=True), 0x564650: _recorder(DEBUG, 1, cdecl=True)})
SELF, S1, S2, VT, ANSWERS = RAM, RAM + 0x800, RAM + 0x810, RAM + 0x900, RAM + 0xa00


def case(r):
    log = log_region()
    answers(log, RESTORE, ANSWERS)
    set_return(log, OPEN, r.getrandbits(32))
    me = bytearray(r.bytes(0x790))
    put(me, 0x10, '2I', S1, S2)
    results = struct.pack('<2I', *[r.choice([0, 0, 0, 1, 0x7fffffff, 0x80000000 | r.getrandbits(31)]) for _ in range(2)])
    regions = {LOG: log, SELF: me, S1: struct.pack('<I', VT), S2: struct.pack('<I', VT), VT: vtable({}, 32, com={RESTORE: 1}), ANSWERS: results,
               0x879cd4: r.bytes(4), 0x8bd524: struct.pack('<I', r.choice([1, 0, 0, 2, r.getrandbits(32)])), 0x8bcbd0: r.bytes(8), 0x8bcbe4: r.bytes(4)}
    return Call(0x564160, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

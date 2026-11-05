# The C library's _fcloseall 0x6891b0 (src/game/core/crtopen) against the original, with fclose 0x67b620 under it: the stream table made here, some slots empty; slots from 20 up emptied, their sections deleted and their blocks freed.
# Locks and _fclose_lk are cdecl recorders; free the heap's hash; DeleteCriticalSection a COM recorder in its import slot.
# usage: crt_fcloseall.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _free, _recorder, answers, heap, import_table, log_region  # noqa: E402

LOCK, UNLOCK, LOCKFILE, UNLOCKFILE, FCLOSE, DELETE = 7, 8, 11, 12, 13, 14
B = Bench([(0x6891b0, 0x689243), (0x67b620, 0x67b660)],
          hooks={0x682720: _recorder(LOCK, 1, cdecl=True), 0x6827a0: _recorder(UNLOCK, 1, cdecl=True), 0x6827c0: _recorder(LOCKFILE, 1, cdecl=True), 0x682830: _recorder(UNLOCKFILE, 1, cdecl=True),
                 0x67b660: _recorder(FCLOSE, 1, cdecl=True), 0x67ad90: _free})
ANSWERS, STREAMS, FILES = RAM, RAM + 0x100, RAM + 0x200
SLOTS = 24


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x100), IMPORT_TABLE: import_table(log, {'delete': (DELETE, 0x6fd0d0, 1, 0, 0)})}
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([0, 0, 1, 0xffffffff]) for _ in range(40)])
    answers(log, FCLOSE, ANSWERS)
    table = []
    for i in range(SLOTS):
        at = FILES + 0x38 * i
        flags = r.choice([0, 1, 2, 0x80, 0x83, 0x40, 0x41, 0x7c, 0xff00]) | (r.getrandbits(16) << 16 & 0xff000000)
        regions[at] = r.bytes(0xc) + struct.pack('<I', flags) + r.bytes(0x28)
        table.append(at if r.random() < 0.8 else 0)
    regions[STREAMS] = struct.pack('<%dI' % SLOTS, *table)
    regions[0xfb3cbc] = struct.pack('<I', STREAMS)
    regions[0xfb4cc0] = struct.pack('<I', r.choice([0, 3, 4, 8, 20, 21, 22, SLOTS]))
    return Call(0x6891b0, [], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

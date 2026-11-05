# Bench: Engine_ResetDevice 0x5774c0: the steps in order in mode 1 and the other modes, each failure's cleanup.
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return, vtable  # noqa: E402

SLOT1, RELEASE_TEX, RELEASE_OBJ, RESTORE, SLOT5, SLOT6, CHANGE, POST, DESTROY, RELEASE_ALL, ERROR = 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
B = Bench([(0x5774c0, 0x57759b)], hooks={0x58dc10: _recorder(RELEASE_TEX, 0), 0x564140: _recorder(RELEASE_OBJ, 0), 0x564160: _recorder(RESTORE, 0), 0x58db90: _recorder(CHANGE, 1),
                                         0x47ea00: _recorder(POST, 0, cdecl=True), 0x583700: _recorder(DESTROY, 0, cdecl=True), 0x5640a0: _recorder(RELEASE_ALL, 0), 0x56af20: _recorder(ERROR, 2)})
SURFACES, SURFACES_VT, BATCH, BATCH_VT = RAM, RAM + 0x40, RAM + 0x100, RAM + 0x140
STEPS = [RELEASE_TEX, SLOT5, RELEASE_OBJ, RESTORE, SLOT6, CHANGE, SLOT1]


def case(r):
    log = log_region()
    failing = r.deal(STEPS + [None, None])
    for k in STEPS:
        set_return(log, k, 0 if k == failing else r.choice([1, 1, 3]))
    regions = {LOG: log, 0x8bd524: struct.pack('<I', r.deal([1, 1, 1, 4, 2])), 0x8c4b44: r.bytes(4), 0x879a24: r.bytes(4), 0x8bd52c: struct.pack('<I', SURFACES), 0x879ab0: struct.pack('<I', BATCH),
               SURFACES: struct.pack('<I', SURFACES_VT), SURFACES_VT: vtable({}, 8), BATCH: struct.pack('<I', BATCH_VT), BATCH_VT: vtable({}, 8)}
    return Call(0x5774c0, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

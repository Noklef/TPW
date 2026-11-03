# The device's source maker (SoundSource_Make 0x6c4190, src/game/core/soundsource) against the original, with SoundSource_Destroy real. The two sources'
# constructors (0x6d2d70 of 1, 0x6d1e30 of none) are recorders answering objects of a recorder vtable, or null now and then (a fault in both); slot 6 (out, info)
# logs what its out holds on the way in (the maker's last argument, then the second block), writes one status per call through it (non-negative, negative) and
# answers the out or another status word. The heap fails now and then (a fault in both).
# usage: soundmake.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, echo, fill, heap, log_region, set_return, vtable  # noqa: E402

B = Bench([(0x6c4190, 0x6c42a4), (0x6c42b0, 0x6c42d2)], hooks={0x6d2d70: _recorder(20, 1), 0x6d1e30: _recorder(21, 0)})
OUT, SLOT, INFO, SRC_B, SRC_A, VT, STATUS = (RAM + 0x100 * i for i in range(7))
STATUSES = [0, 0, 1, 0x7fffffff, 0xffffffff, 0x80000000, 0x88780078]


def case(r):
    log = log_region()
    set_return(log, 20, SRC_B if r.random() < 0.97 else 0)
    set_return(log, 21, SRC_A if r.random() < 0.97 else 0)
    blob(log, 6, 0, 1)
    if r.random() < 0.8:
        echo(log, 6, 0)
        fill(log, 6, 0, struct.pack('<2I', r.deal(STATUSES), r.choice(STATUSES)), records=2)
    else:
        set_return(log, 6, STATUS)
    regions = {LOG: log, HEAP: heap(0x100, r.choice([0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2])), OUT: r.bytes(4), SLOT: r.bytes(4), INFO: r.bytes(0x10),
               SRC_B: struct.pack('<I', VT) + r.bytes(0xc), SRC_A: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({0: 1, 6: 2}),
               STATUS: struct.pack('<I', r.deal(STATUSES))}
    return Call(0x6c4190, [r.getrandbits(32), OUT, SLOT, INFO, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

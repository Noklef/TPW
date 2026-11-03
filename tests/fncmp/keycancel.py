# The cancel key's action (src/game/core/keyactions: 0x40c180) against the original: Obj7b51f0_ResetDialogChild21 (k 20) answering 0 mostly, GameMode_Set (k 21) a recorder; the mouse mode
# [0x7b05d8] a recorder object (slot 9 answering 0, 1 or another) or none, the holder [0x7b05e8] set or not, a heap (an allocation now and then failing), as focus.py's modes_case.
# usage: keycancel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, set_return, vtable  # noqa: E402
from focus import MODES  # noqa: E402

B = Bench(sorted(set(MODES + [(0x40c180, 0x40c3a0), (0x40bed0, 0x40bf65)])), hooks={0x4816b0: _recorder(20, 0), 0x52f200: _recorder(21, 2, cdecl=True)})
MODE, MODE_VT, HOLDER = RAM + 0xc000, RAM + 0xc100, RAM + 0xc200


def case(r):
    log = log_region()
    set_return(log, 20, r.deal([0, 0, 0, 0, 1, r.getrandbits(32) | 1]))
    set_return(log, 9, r.deal([0, 1, 1, 2, 9, r.getrandbits(32)]))
    regions = {LOG: log, HEAP: heap(0x1000, r.deal([0, 0, 0, 0, 1, 2, 3, 4])), MODE_VT: vtable({0: 1}), MODE: put(bytearray(r.bytes(0x10)), 0, 'I', MODE_VT),
               0x7b05d4: put(put(bytearray(r.bytes(0x18)), 4, 'I', r.deal([MODE, MODE, MODE, 0])), 0x14, 'I', r.deal([0, HOLDER])),
               0xfb3518: bytes([r.choice([1, 3, 0xff])]), 0xfb3b7c: struct.pack('<I', 1)}
    return Call(0x40c180, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

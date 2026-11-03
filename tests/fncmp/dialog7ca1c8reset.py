# Dialog7ca1c8_ResetMouseMode (src/game/ui/dialog7ca1c8, 0x497f00) against the original. Every call a recorder (their own benches have them; driver.cpp's kDialog7ca1c8ResetEnv): Global81ae2c_Get_52f840 10, GameHeap_Alloc 11 (the one return), MouseModeOnlineBase_Ctor
# 12, MouseMode_Set stdcall 13, GameMode_Set 14, the current mode's slot 9 thiscall 15. The game mode 3 or not, the holder there or not, the current mode none or answering 1 or others.
# usage: dialog7ca1c8reset.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x497f00, 0x49808c)], hooks={0x52f840: _recorder(10, 0, cdecl=True), 0x67acc0: _recorder(11, 1, cdecl=True), 0x46c6a0: _recorder(12, 0), 0x46c350: _recorder(13, 1, com=True), 0x52f200: _recorder(14, 2, cdecl=True)})
CURRENT, VT = RAM + 0x100, RAM + 0x200


def case(r):
    log = log_region()
    set_return(log, 10, r.choice([0, 1, 2, 3, 4, 0x103, r.getrandbits(32)]))
    set_return(log, 11, r.choice([RAM + 0x1000, RAM + 0x1000, RAM + 0x1000, 0]))
    set_return(log, 12, RAM + 0x800)
    set_return(log, 15, r.choice([0, 1, 1, 2, 9, 0x101, r.getrandbits(32)]))
    globals_ = bytearray(r.bytes(0x18))   # 0x7b05d4 ..
    put(globals_, 4, 'I', r.choice([CURRENT, CURRENT, CURRENT, 0]))
    put(globals_, 0x14, 'I', r.choice([0, 0, RAM + 0xb00]))
    regions = {LOG: log, CURRENT: struct.pack('<I', VT) + r.bytes(0xc), VT: put(vtable({}, size=0x10), 0x24, 'I', recorder(15, 0)), 0x7b05d4: globals_, 0x7ca1cc: r.bytes(8)}
    return Call(0x497f00, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

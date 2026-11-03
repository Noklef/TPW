# Dialog7cc1f8_TabHandler (src/game/ui/dialog7cc1f8, 0x4aca60) against the original. Every call a recorder (their own benches have them; driver.cpp's kDialog7cc1f8TabEnv): Dialog7cc1f8_ShowStats 10, Dialog7cc1f8_FillList 11, Language_WideText 12,
# HoverLabel_Attach 13, Window_CallDefault thiscall 14. The four tabs and their edges, the message 0x101 and its neighbours, b 1 mostly.
# usage: dialog7cc1f8tab.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x4aca60, 0x4acb56)], hooks={0x4abfb0: _recorder(10, 1, cdecl=True), 0x4aaf70: _recorder(11, 2, cdecl=True), 0x485b00: _recorder(12, 1, cdecl=True), 0x485d20: _recorder(13, 3, cdecl=True), 0x65f66e: _recorder(14, 3)})


def case(r):
    log = log_region()
    set_return(log, 12, r.getrandbits(32))
    set_return(log, 14, r.getrandbits(32))
    message = r.choice([0x101, 0x101, 0x101, 0x101, 0x101, 0x100, 0x102, 0x10101, r.getrandbits(32)])
    a = r.choice([0x1fa, 0x1fb, 0x1fc, 0x1fd, 0x1fa, 0x1fb, 0x1fc, 0x1fd, 0x1f9, 0x1fe, 0, 0x101fa, r.getrandbits(32)])
    b = r.choice([1, 1, 1, 1, 0, 2, 0x101, r.getrandbits(32)])
    return Call(0x4aca60, [RAM + 0x100, message, a, b], {LOG: log, 0x7cc1f8: r.bytes(4)}, ret='a')


if __name__ == '__main__':
    B.main(case)

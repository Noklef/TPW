# The dialog [0x7cc5f4]'s handler (src/game/ui/dialog7cc5f4handler: Dialog7cc5f4_Message 0x4baf10, Dialog7cc5f4_Handler 0x4bb030) against the originals. Recorders (driver.cpp's kDialog7cc5f4HandlerEnv): Window_SendMessage 10, Window_PostMessage 11, Focus_Restore 12, KeyMaps_EnableAllAndClearFlag 13, KeyMaps_ResetAllAndSetFlag 14, Online_Instance 15, 0x5b5d10 16,
# Window_FindChild 17, Window_SetFocus 18, Dialog_BaseHandler 19; the callback a recorder 20 or none.
# usage: dialog7cc5f4handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, recorder, set_return  # noqa: E402

B = Bench([(0x4baf10, 0x4bb02c), (0x4bb030, 0x4bb06f)],
          hooks={0x658d9f: _recorder(10, 4, cdecl=True), 0x658ccc: _recorder(11, 4, cdecl=True), 0x4862a0: _recorder(12, 0, cdecl=True), 0x486b60: _recorder(13, 0, cdecl=True), 0x486b70: _recorder(14, 0, cdecl=True), 0x48b390: _recorder(15, 0, cdecl=True), 0x5b5d10: _recorder(16, 2), 0x65d7d0: _recorder(17, 1), 0x65e59b: _recorder(18, 1), 0x47ee90: _recorder(19, 4, cdecl=True)})
MAIN = RAM


def case(r):
    log = log_region()
    set_return(log, 15, r.getrandbits(32))
    set_return(log, 17, r.getrandbits(32))
    set_return(log, 19, r.getrandbits(32))
    message = r.choice([0x14, 0x15, 0x100, 0x100, 0x100, 0x802, 0x802, 0x803, 0x803, 0x804, 0x13, 0x16, 0xff, 0x101, 0x801, 0x805, r.getrandbits(32)])
    a = r.choice([0xfffffffe, 0xffffffff, 0x49, 0x49, 0x4b, 0xfffffffd, r.getrandbits(32)])
    regions = {LOG: log, MAIN: r.bytes(16), 0xf82884: struct.pack('<I', MAIN), 0x7cc5f4: r.bytes(4), 0x7cc5fc: struct.pack('<I', r.choice([0, recorder(20, 0, cdecl=True)]))}
    fn = r.choice([0x4baf10, 0x4bb030])
    return Call(fn, [r.getrandbits(32), message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

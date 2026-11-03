# The handler of the dialog [0x7cc984] (src/game/ui/dialog7cc984: Dialog7cc984_Handler, 0x4c0880) against the original. Recorders (their own benches have them; driver.cpp's kDialog7cc984Env): Window_SendMessage 20, Class702da0_Clear 21, Online_Instance 22,
# Online_UnpublishPark 23, 0x5b86a0 24, Window_SetFocus 25, Focus_Restore 26; the callback [0x7cca48] 27; the parts' slots 9, 10 and 0x11 vtable recorders (9: the answer). Fn_65d55a and Window_CallDefault run.
# usage: dialog7cc984.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x4c0880, 0x4c0add), (0x65d55a, 0x65d56b), (0x65f66e, 0x65f697)],
          hooks={0x658d9f: _recorder(20, 4, cdecl=True), 0x5d6060: _recorder(21, 0), 0x48b390: _recorder(22, 0, cdecl=True), 0x5b7960: _recorder(23, 1), 0x5b86a0: _recorder(24, 3), 0x65e59b: _recorder(25, 1), 0x4862a0: _recorder(26, 0, cdecl=True)})
WINDOW, PARENT, OWNER, GAME, PARTS, VT = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600


def case(r):
    log = log_region()
    set_return(log, 22, r.getrandbits(32))
    set_return(log, 9, r.choice([0, 1, 0x10000, r.getrandbits(32)]))
    parent = r.choice([PARENT, PARENT, PARENT, 0])
    window = put(put(bytearray(r.bytes(0x120)), 0x60, 'I', parent), 0x118, 'I', CDECL_LOG4)
    parent_window = bytearray(r.bytes(0x20))
    parent_window[4] = r.choice([0, 0, 1, 0x80])
    game = bytearray(r.bytes(0x20))
    parts = bytearray(0x60)
    for k in (2, 3, 5):
        put(game, 4 * k, 'I', PARTS + 0x10 * k)
        put(parts, 0x10 * k, 'I', VT)
    message = r.choice([0x100] * 8 + [0x14, 0x14, 0x1000b, 0x1000b, 0x1000a, 0xff, 0x101, r.getrandbits(32)])
    a = r.choice([0xfffffffe, 0xfffffffe, 0xe592e, 0xe592e, 0xe592f, 0xe592f, 0xe592d, 0xe5930, 0xffffffff, 0x1b, r.getrandbits(32)])
    if message == 0x1000b:
        a = r.choice([0x1b, 0x1b, 0x1a, r.getrandbits(32)])
    regions = {LOG: log, WINDOW: window, PARENT: parent_window, OWNER: put(bytearray(r.bytes(8)), 4, 'I', r.choice([PARENT, PARENT, r.getrandbits(32)])), GAME: game, PARTS: bytes(parts), VT: vtable({9: 0, 10: 0, 0x11: 0}, size=0x20),
               0xf82884: struct.pack('<I', OWNER), 0xf85614: struct.pack('<I', GAME), 0xfaa5d8: struct.pack('<I', r.choice([PARENT, r.getrandbits(32)])), 0x7cc984: r.bytes(4), 0x7cc990: r.bytes(4),
               0x7cca48: struct.pack('<I', r.choice([recorder(27, 0, cdecl=True), 0]))}
    return Call(0x4c0880, [WINDOW, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

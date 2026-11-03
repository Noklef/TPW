# ViewButtons_Handler (src/game/ui/viewbuttons, 0x4c53f0) against the original. Recorders, as ViewButtonsEnv's: Window_CallDefault 10, ViewCtl_UseSecondVector 11, UseFirstVector 12, Lower48 13, Raise48 14,
# Raise50 15, Lower50 16, Online_Instance 17 (it answers SESSION), Obj_HasActivity 18, Dialog7cc944_Open 19, Class702da0_Push 20. Window_DwordA0 runs (the window's array +0xa0 with one or no dword set; it is
# +0xa0's word index), the game's objects' slots 7, 8 and 0x12 recorders; the session's objects with their +0x28 set or not.
# usage: viewbuttons.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x4c53f0, 0x4c56c1), (0x65e705, 0x65e73d)],
          hooks={0x65f66e: _recorder(10, 3), 0x5dd350: _recorder(11, 0), 0x5dd390: _recorder(12, 0), 0x5dd310: _recorder(13, 0), 0x5dd320: _recorder(14, 0), 0x5dd330: _recorder(15, 0), 0x5dd340: _recorder(16, 0),
                 0x48b390: _recorder(17, 0, cdecl=True), 0x5b6230: _recorder(18, 0), 0x4bff70: _recorder(19, 1, cdecl=True), 0x5d6110: _recorder(20, 2)})
WINDOW, GAME, OBJ4, OBJC, OBJ10, OBJ14, VT, SESSION, PARTS = (RAM + 0x1000 * k for k in range(9))
BUTTONS = (0x16000, 0x16001, 0x16002, 0x16003, 0x16004, 0x16005, 0x16006)


def case(r):
    log = log_region()
    set_return(log, 10, r.getrandbits(32))
    set_return(log, 17, SESSION)
    set_return(log, 18, r.choice([0, 0, 0, 1, 0x100, r.getrandbits(32)]))
    message = r.deal([0x14, 0x100, 0x100, 0x100, 0x100, 0x10008, 0x10009, 0x10007, 0x1000a, 0xff, 0x101, 0x13, 0x15, r.getrandbits(32)])
    a = r.choice(BUTTONS * 3 + (0x15fff, 0x16007, 0x16008, r.getrandbits(32)))
    b = r.choice([0, 1, 1, 2, r.getrandbits(32), 0x101])
    window = bytearray(r.bytes(0x100))
    put(window, 0xa0, 'I', r.choice([0, 0, 1, 2, 0x10000, r.getrandbits(32)]))
    session = bytearray(r.bytes(0x100))
    put(session, 0xe0, 'I', PARTS)
    put(session, 0xf8, 'I', PARTS + 0x40)
    put(session, 0xfc, 'I', PARTS + 0x80)
    parts = bytearray(r.bytes(0xc0))
    for k in range(3):
        put(parts, 0x40 * k + 0x28, 'I', 0 if r.random() < 0.2 else r.getrandbits(32) | 1)
    game = bytearray(r.bytes(0x20))
    put(game, 4, 'I', OBJ4)
    put(game, 0xc, 'I', OBJC)
    put(game, 0x10, 'I', OBJ10)
    put(game, 0x14, 'I', OBJ14)
    regions = {LOG: log, WINDOW: window, SESSION: session, PARTS: parts, GAME: game, OBJ4: struct.pack('<I', VT), OBJC: struct.pack('<I', VT), OBJ10: struct.pack('<I', VT), OBJ14: struct.pack('<I', VT),
               VT: vtable({7: 0, 8: 0, 0x12: 0}), 0xf85614: struct.pack('<I', GAME), 0x7cdb0c: r.bytes(4)}
    return Call(0x4c53f0, [WINDOW, message, a, b], regions)


if __name__ == '__main__':
    B.main(case)

# The scroll bar thumb's handler (src/game/ui/window705b20thumb: 0x66bb9b) against the original. Recorders as driver.cpp's kWindow705b20ThumbEnv; the rectangles' code real (MoveInside's and
# SetRect's rectangle logged by its dwords); the mouse object's and the bar's slots on recorders of their own.
# usage: window705b20thumb.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, heap, log_region, put, recorder, set_return  # noqa: E402

B = Bench([(0x66bb9b, 0x66bf85), (0x65d55a, 0x65d56b), (0x66b454, 0x66b47d), (0x65d4a0, 0x65d4cb), (0x66b438, 0x66b454), (0x661770, 0x661810), (0x659d00, 0x659dbc), (0x659dbc, 0x659de3),
           (0x659de5, 0x659e3e), (0x659e3e, 0x659e83), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65ab40), (0x66bff7, 0x66c01b), (0x66c01a, 0x66c7dc), (0x5da3c0, 0x5da3c1)],
          hooks={0x658d9f: _recorder(10, 4, cdecl=True), 0x658ccc: _recorder(11, 4, cdecl=True), 0x65f6d1: _recorder(12, 4, cdecl=True), 0x65cf3d: _recorder(13, 0), 0x66b088: _recorder(14, 2),
                 0x65dd18: _recorder(15, 0), 0x668c99: _recorder(16, 2), 0x668bec: _recorder(17, 2), 0x65e549: _recorder(18, 0), 0x65e56e: _recorder(19, 0), 0x66b9bc: _recorder(25, 0),
                 0x66b9ef: _recorder(26, 0), 0x66b771: _recorder(27, 7, cdecl=True), 0x65cc04: _recorder(28, 1), 0x66af19: _recorder(29, 0)})
SEND, DEFAULT, MOVE, SETRECT, MOUSE8, MOUSE9, MOUSE10, BAR7, BAR8 = 10, 12, 27, 28, 20, 21, 22, 23, 24
WINDOW, BAR, MOUSE, MOUSE_SLOTS, BAR_SLOTS = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200
MESSAGES = [0xc, 0x13, 0x13, 0x15, 0x10001, 0x10001, 0x10002, 0x10002, 0x10003, 0x10003, 0x10003, 0x10003, 0x10004, 0x10004, 0x10005, 0x10005, 0x10005, 0x10006, 0x1000c, 0x1000d, 0x1000d,
            0x1000e, 0x10000, 0x14, 0xd, 0, -1]


def rect(r):
    """a GuiRect with no list, as windowarea.py's"""
    buf = bytearray(r.bytes(0x18))
    struct.pack_into('<I', buf, 0, 0x705920)
    for off in range(0xc, 0x18, 4):
        struct.pack_into('<I', buf, off, 0)
    return buf


def case(r):
    log = log_region()
    blob(log, MOVE, 0, 7)
    blob(log, SETRECT, 0, 7)
    set_return(log, SEND, r.getrandbits(32))
    set_return(log, DEFAULT, r.getrandbits(32))
    set_return(log, BAR8, r.getrandbits(32))
    window = bytearray(r.bytes(0x170))
    window[0x20:0x38] = rect(r)
    put(window, 0x60, 'I', BAR)
    put(window, 0x11c, 'I', r.choice([0, 0, 1, 1, 1, 2, 3, r.getrandbits(32)]))
    bar = bytearray(r.bytes(0x170))
    bar[0x2c:0x44] = rect(r)
    put(bar, 0, 'I', BAR_SLOTS)
    mouse_slots = bytearray(0x80)
    put(mouse_slots, 8 * 4, '3I', recorder(MOUSE8, 2), recorder(MOUSE9, 1), recorder(MOUSE10, 0))
    bar_slots = bytearray(0x80)
    put(bar_slots, 7 * 4, '2I', recorder(BAR7, 1), recorder(BAR8, 0))
    buttons = bytearray(r.bytes(0x200))
    for i in range(0, 0x200, 12):
        put(buttons, i, 'h', r.choice([0, 0, 0, 1, -1, 0x100]))
    put(buttons, 0x3c, 'I', r.choice([0, 1, 0x10000, 0x80000000]))
    regions = {LOG: log, HEAP: heap(0x400), WINDOW: window, BAR: bar, MOUSE: struct.pack('<I', MOUSE_SLOTS), MOUSE_SLOTS: mouse_slots, BAR_SLOTS: bar_slots, 0xfaa598: buttons,
               0x77c48c: struct.pack('<I', MOUSE)}
    message = r.choice(MESSAGES)
    a = r.choice([0, 0, 1, 2, 3, 33]) if message in (0x10004, 0x10005) else r.getrandbits(32)
    return Call(0x66bb9b, [WINDOW, message & 0xffffffff, a, r.getrandbits(32)], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

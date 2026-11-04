# The GUI's handler of every window's messages (src/game/ui/guimessagefx: 0x485780) against the original. Recorders as driver.cpp's kGuiMessageFxEnv; the window's slot 3 a recorder of its own; the rectangle's words through Window_GetRect's recorder.
# usage: guimessagefx.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, put, recorder, set_return, strings  # noqa: E402

PLAY, ATTACH, DETACH, ONWINDOW, FIND, RECT, FINISH, EMPTY, POSITION, SETUP, KIND = range(10, 21)
B = Bench([(0x485780, 0x485a68), (0x65d57c, 0x65d58d), (0x668abd, 0x668ad3), (0x659d00, 0x659dbc), (0x659dbc, 0x659de3), (0x659de5, 0x659e3e), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65ab40), (0x66bff7, 0x66c01b), (0x66c01a, 0x66c7dc), (0x5da3c0, 0x5da3c1)],
          hooks={0x51bfc0: _recorder(PLAY, 6, cdecl=True), 0x480ec0: _recorder(ATTACH, 1), 0x480f50: _recorder(DETACH, 0), 0x480f90: _recorder(ONWINDOW, 1), 0x65d7d0: _recorder(FIND, 1), 0x65d42e: _recorder(RECT, 2),
                 0x5eda70: _recorder(FINISH, 0, cdecl=True), 0x5edac0: _recorder(EMPTY, 0, cdecl=True), 0x5eda40: _recorder(POSITION, 2, cdecl=True), 0x5ed920: _recorder(SETUP, 4, cdecl=True)})
WINDOW, SLOTS = RAM + 0x1000, RAM + 0x1200
MESSAGES = [0x100, 0x1f, 0x14, 0x14, 0x20, 0x801, 0x11, 0x11, 0x13, 0x13, 0x13, 0x10001, 0x10001, 0x10001, 0x10002, 0x1e, 0x12, 0x10000, 0x10003, -1]


def word(r):
    return r.deal([0, 1, 2, 3, 0x300, 0x400, 639, 640, 1024, -1, -7, 0x7fff, -0x8000, r.getrandbits(16) - 0x8000])


def case(r):
    log = log_region()
    set_return(log, FIND, r.getrandbits(32))
    set_return(log, KIND, r.deal([2, 2, 2, 5, 5, 0, 0x102, 0x105]))
    strings(log, RECT, 0, 0b1)
    fill(log, RECT, 0, struct.pack('<4h', word(r), word(r), word(r), word(r)), offset=4)
    window = bytearray(r.bytes(0x170))
    put(window, 0, 'I', SLOTS)
    put(window, 0x48, 'I', r.deal([0, 0x10, r.getrandbits(32)]))
    window[0x138] = r.deal([0, 1, 1, 2, 0x101 & 0xff, r.getrandbits(8)])
    window[0x139] = r.deal([0, 1, 1, 2, r.getrandbits(8)])
    regions = {LOG: log, WINDOW: window, SLOTS: put(bytearray(0x20), 0xc, 'I', recorder(KIND, 0)), 0x78d90e: bytes([r.deal([1, 1, 1, 1, 0, 0x80])]), 0x803a2c: r.bytes(4)}
    return Call(0x485780, [WINDOW, r.deal(MESSAGES) & 0xffffffff, r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

# The button window 0x705a50's handler (src/game/ui/window705a50handler: 0x668f9c, the getter 0x668abd) against the original. Recorders as driver.cpp's kWindow705a50HandlerEnv; the mouse [0x77c48c]'s slots on recorders of their own.
# usage: window705a50handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return  # noqa: E402

REPEAT, ACTIVATE, NOW, FLAG, STATE, START, STOP, POST, CURSOR, DEFAULT, MOUSE7, MOUSE8, MOUSE9, MOUSE10 = range(10, 24)
B = Bench([(0x668f9c, 0x66938c), (0x668abd, 0x668ad3), (0x65d57c, 0x65d58d), (0x65d7bc, 0x65d7d0), (0x65d55a, 0x65d56b)],
          hooks={0x668dd9: _recorder(REPEAT, 0), 0x65dd18: _recorder(ACTIVATE, 0), 0x65968e: _recorder(NOW, 0, cdecl=True), 0x668c99: _recorder(FLAG, 2), 0x668bec: _recorder(STATE, 2), 0x668ecf: _recorder(START, 0),
                 0x668eff: _recorder(STOP, 0), 0x658ccc: _recorder(POST, 4, cdecl=True), 0x65f150: _recorder(CURSOR, 0), 0x65f6d1: _recorder(DEFAULT, 4, cdecl=True)})
WINDOW, MOUSE, SLOTS = RAM + 0x1000, RAM + 0x1200, RAM + 0x1300
MESSAGES = [0x1e, 0x10, 0x10001, 0x10002, 0x10003, 0x10004, 0x10004, 0x10004, 0x10005, 0x10005, 0x10005, 0x10000, 0x10006, 0, -1]


def case(r):
    log = log_region()
    for k in (FLAG, STATE):
        struct.pack_into('<I', log, LOG_BYTES + 4 * k, 3)
    set_return(log, DEFAULT, r.getrandbits(32))
    set_return(log, NOW, r.getrandbits(32))
    set_return(log, STOP, r.deal([0, 1, 0x10000, 0x10001]))
    window = bytearray(r.bytes(0x170))
    put(window, 0x11c, 'I', r.deal([0, 0, 1, 3, r.getrandbits(32)]))
    window[0x138] = r.deal([0, 1, 2, 3, 0x82, 0xff, r.getrandbits(8)])
    put(window, 0x48, 'I', r.deal([0, 0x10, r.getrandbits(32)]))
    buttons = bytearray(r.bytes(0x24))
    for i in range(3):
        put(buttons, 12 * i, 'h', r.deal([0, 1, 1, 2, -1, 0x100]))
    slots = bytearray(0x40)
    put(slots, 7 * 4, '4I', recorder(MOUSE7, 0), recorder(MOUSE8, 2), recorder(MOUSE9, 1), recorder(MOUSE10, 0))
    regions = {LOG: log, WINDOW: window, MOUSE: struct.pack('<I', SLOTS), SLOTS: slots, 0x77c48c: struct.pack('<I', MOUSE), 0xfaa598: buttons}
    if r.deal([0, 0, 0, 0, 1]):
        return Call(0x668abd, [WINDOW], regions, conv='t', ret='w')
    message = r.deal(MESSAGES)
    return Call(0x668f9c, [WINDOW, message & 0xffffffff, r.deal([0, 0, 0, 1, r.getrandbits(32)]), r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

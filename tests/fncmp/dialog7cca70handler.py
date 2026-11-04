# The dialog [0x7cca70]'s handler (src/game/ui/dialog7cca70handler: Dialog7cca70_Handler 0x4c1a10) against the original. Window_FindChild among decoys and Online_Instance real; recorders as driver.cpp's kDialog7cca70HandlerEnv, the function [0x7cca8c] a cdecl recorder or none.
# usage: dialog7cca70handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fill, heap, log_region, put, recorder, set_return, strings  # noqa: E402

FOCUS, LEAVE, CLEAR, SEND, NTH, OPEN, DEFAULT, DONE = range(10, 18)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4c1a10, 0x4c1b30), (0x65d7bc, 0x65d840), (0x48b390, 0x48b3fd)],
          hooks={0x65e59b: _recorder(FOCUS, 1), 0x5b6070: _recorder(LEAVE, 0), 0x5d6060: _recorder(CLEAR, 0), 0x658d9f: C(SEND, 4), 0x664c71: _recorder(NTH, 2), 0x4c0ae0: C(OPEN, 5), 0x65f66e: _recorder(DEFAULT, 3)})
WINDOW, HOLDER, CENTRE, SESSION, KIDS = (RAM + 0x1000 * k for k in range(1, 6))


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(), HOLDER: put(bytearray(r.bytes(0x20)), 0x14, 'I', CENTRE), CENTRE: r.bytes(4), SESSION: r.bytes(0x10)}
    regions[0xf82884] = struct.pack('<I', HOLDER)
    regions[0xf85614] = struct.pack('<I', HOLDER)
    regions[0xf7d88c] = struct.pack('<I', SESSION)
    first, _ = kids(r, regions, KIDS, (0x14beb, 0x14bec))
    regions[WINDOW] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    regions[0x7cca70] = struct.pack('<I', r.deal([WINDOW, WINDOW, 0, RAM]))
    regions[0x7cca8c] = struct.pack('<I', r.deal([recorder(DONE, 0, cdecl=True), 0]))
    message = r.deal([0x14, 0x100, 0x100, 0x100, 0x1000b, 0x1000b, 0x1000b, 0x400, 0x400, 0x400, 0x400, 0x1000a, 0xff, 0x101, 5, r.getrandbits(32)])
    a = r.deal({0x100: [0xfffffffe, 0xfffffffe, 0xffffffff, 0x1b, 0], 0x1000b: [0x1b, 0x1b, 0xd, 0xfffffffe], 0x400: [0x14beb, 0x14beb, 0x14bec, 0x77]}.get(message, [0xfffffffe, 0x1b, 0x14beb, r.getrandbits(32)]))
    b = r.deal([0xffffffff, 0, 1, r.getrandbits(32)])
    fill(log, NTH, 1, struct.pack('<I', r.getrandbits(32)))
    strings(log, NTH, 0, 1 << 1)
    set_return(log, DEFAULT, r.getrandbits(32))
    return Call(0x4c1a10, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

# The handler of the dial-up form (src/game/ui/dialog7cc5f4dial: 0x4b9d00) against the original. Window_FindChild among decoys and CRT wcslen real; the others recorders as driver.cpp's kDialog7cc5f4DialEnv; the import slot 0x6fd5b4 recorder 10.
# usage: dialog7cc5f4dial.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, recorder, set_return  # noqa: E402

DIAL, KEYS, FOCUS_BACK, SEND, POST, DWORD, TEXT, BOX, HANDLER, FOCUS, BASE, DONE, SLOT = range(10, 23)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4b9d00, 0x4b9f26), (0x65d7bc, 0x65d840), (0x67a400, 0x690000)],
          hooks={0x486b60: C(KEYS, 0), 0x4862a0: C(FOCUS_BACK, 0), 0x658d9f: C(SEND, 4), 0x658ccc: C(POST, 4), 0x65e705: _recorder(DWORD, 1), 0x485b00: C(TEXT, 1), 0x47f020: C(BOX, 8), 0x65e526: _recorder(HANDLER, 1),
                 0x65e59b: _recorder(FOCUS, 1), 0x47ee90: C(BASE, 4)})
WINDOW, HOLDER, MADE, TABLE = (RAM + 0x1000 * k for k in range(1, 5))
KIDS = RAM + 0x10000


def wide(r):
    return b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(r.deal([0, 1, 1, 9, 20]))) + b'\0\0'


def case(r):
    log = log_region()
    regions = {LOG: log, HOLDER: r.bytes(8), TABLE: put(bytearray(0x40), 0x24, 'I', recorder(SLOT, 1))}
    regions[0xf82884] = struct.pack('<I', HOLDER)
    regions[MADE] = put(bytearray(r.bytes(0x20)), 0, 'I', TABLE)
    first, _ = kids(r, regions, KIDS, (0x49, 0x4b))
    regions[WINDOW] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    regions[0x7cc4f0] = put(bytearray(r.bytes(0x50)), 0x3c, 'I', r.deal([0, recorder(DONE, 0, cdecl=True)]))
    regions[0x7cc548] = wide(r)
    regions[0x7cc590] = wide(r)
    regions[0x6fd5b4] = struct.pack('<I', recorder(DIAL, 5))
    set_return(log, DIAL, r.deal([0, 0, 0, 1, 0x100, 0x80000000]))
    set_return(log, DWORD, r.getrandbits(32))
    set_return(log, TEXT, r.getrandbits(32))
    set_return(log, BOX, MADE)
    set_return(log, BASE, r.getrandbits(32))
    blob_bytes(log, BOX, 0, 0xff7f)
    message = r.deal([0x14, 0x15, 0x100, 0x100, 0x100, 0x100, 0x100, 0x802, 0x802, 0x803, 0x803, 0x804, 0x13, 0x16, 0x101, 0x801, 0x805, r.getrandbits(32)])
    if message == 0x100:
        a = r.deal([0xffffffff, 0xffffffff, 0xffffffff, 0xfffffffe, 0xfffffffd, 0])
    else:
        a = r.deal([0x49, 0x49, 0x4b, 0x149, r.getrandbits(32)])
    return Call(0x4b9d00, [WINDOW, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

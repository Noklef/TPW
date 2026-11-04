# The chat line's handler (src/game/ui/window7cc8bcedit: 0x4be3a0) against the original, with 0x4051d0 (B's range; the port's Grow, copy and length), the strings' code and Window_FindChild real; the others recorders as driver.cpp's kWindow7cc8bcEditEnv.
# The said text logged by its string's pointer, length and room and the first 15 bytes of its text (byte 0 of the by-value string's allocator dword is the window's low byte, the rest unset: out).
# usage: window7cc8bcedit.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob_bytes, heap, log_region, put, recorder, set_return, strings  # noqa: E402

FIND, KILL, FOCUS, HOLDER_CTOR, ONLINE_CTOR, BASE_CTOR, SET_MODE, CURSOR, OPEN, SEND, CHAT, SAY, RESET, ENABLE, DEFAULT, SLOT_WINDOW, SLOT_MODE = range(10, 27)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4be3a0, 0x4be72d), (0x65d7bc, 0x65d840), (0x4051d0, 0x405280), (0x405280, 0x405390), (0x479230, 0x479270), (0x4f23f0, 0x4f2450), (0x67a400, 0x690000), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)],
          hooks={0x40c870: _recorder(FIND, 2), 0x65e5fa: _recorder(KILL, 1), 0x65e59b: _recorder(FOCUS, 1), 0x46d550: _recorder(HOLDER_CTOR, 0), 0x46ce80: _recorder(ONLINE_CTOR, 0), 0x46c6a0: _recorder(BASE_CTOR, 0),
                 0x46c350: _recorder(SET_MODE, 1), 0x65f11d: _recorder(CURSOR, 1), 0x48c830: C(OPEN, 1), 0x658d9f: C(SEND, 4), 0x46cf10: C(CHAT, 0), 0x4ea490: _recorder(SAY, 4), 0x486b70: C(RESET, 0), 0x486b60: C(ENABLE, 0),
                 0x65f66e: _recorder(DEFAULT, 3)})
WINDOW, VIEW, ENTRY, VT_WINDOW, MODE, VT_MODE, MADE = (RAM + 0x1000 * k for k in range(1, 8))
KIDS1, KIDS2, KIDS3 = RAM + 0x10000, RAM + 0x12000, RAM + 0x14000


def chain(r, regions, holder, at, ident):
    first, top = kids(r, regions, at, (ident,))
    put(regions[holder], 0x5c, 'I', first)
    return top[ident]


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x2000, r.deal([0] * 9 + [1, 2, 3, 4]))}
    regions[WINDOW] = put(bytearray(r.bytes(0x100)), 0, 'I', VT_WINDOW)
    regions[VT_WINDOW] = put(bytearray(0x40), 0x24, 'I', recorder(SLOT_WINDOW, 1))
    regions[MODE] = put(bytearray(r.bytes(0x20)), 0, 'I', VT_MODE)
    regions[VT_MODE] = put(bytearray(0x40), 0x24, 'I', recorder(SLOT_MODE, 0))
    regions[VIEW] = bytearray(r.bytes(0x100))
    regions[ENTRY] = r.bytes(0x100)
    list_ = chain(r, regions, VIEW, KIDS1, 0xc3123)
    column = chain(r, regions, list_, KIDS2, 0xc312b)
    chain(r, regions, column, KIDS3, 0xc312c)
    views = bytearray(r.bytes(0x10))
    put(views, 4, 'B', r.deal([0, 0, 1, 0xff]))
    put(views, 0xc, 'I', VIEW)
    views += struct.pack('<I', ENTRY)
    regions[0x7cc8b0] = views
    regions[0x7cc6a0] = struct.pack('<B', r.deal([0, 0, 0, 1, 0x80]))
    regions[0x7cc6a8] = b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(r.deal([0, 1, 7, 31, 32, 40]))) + b'\0\0'
    regions[0x78721c] = r.bytes(4)
    modes = bytearray(r.bytes(0x20))
    put(modes, 8, 'I', r.deal([MODE, MODE, MODE, 0]))
    put(modes, 0x18, 'I', r.deal([0, 0, MADE]))
    regions[0x7b05d0] = modes
    set_return(log, FIND, r.deal([0, 0, 0, 1, 0x10000, 0x10001, 0xffffffff]))
    set_return(log, HOLDER_CTOR, r.deal([MADE + 0x10, MADE + 0x10, 0]))
    set_return(log, ONLINE_CTOR, r.deal([MADE + 0x20, 0]))
    set_return(log, BASE_CTOR, r.deal([MADE + 0x30, 0]))
    set_return(log, CHAT, MADE + 0x40)
    set_return(log, DEFAULT, r.getrandbits(32))
    set_return(log, SLOT_MODE, r.deal([9, 9, 9, 0, 8, 0x109]))
    blob_bytes(log, SAY, 1, 0x7fff)
    strings(log, SAY, 0, 1 << 0)
    message = r.deal([0x1000b] * 8 + [0x1f, 0x20, 0x1e, 0x21, 0x1000a, 0x1000c, r.getrandbits(32)])
    key = r.deal([9, 0xd, 0xd, 0x73, 0x73, 0x41, 0x41, 0x1b, r.getrandbits(32)])
    b = r.deal([0, 0, 0, 0x10000, 0xff0000, 0x100, 0xff00ffff, 0x7f000000])
    return Call(0x4be3a0, [WINDOW, message, key, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

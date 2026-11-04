# The menu's online rows and its boxes' answers (src/game/ui/panel7c2534online: 0x48bd40, 0x48b400, 0x48b5b0, 0x48b620, 0x48bc30, 0x48bc50) against the originals. Online_Instance, UserSlots_Instance, Obj_Self, WString_Dtor and Mem_Free real; the rest recorders as driver.cpp's kPanel7c2534OnlineEnv, the slots of [0xf85614]'s objects recorders 7, 8, 9, 11.
# The WString out of 0x5b8160 filled (its block's address the free's); the table out of 0x51ac20 filled and hashed at Obj_SetNameAndWords.
# usage: panel7c2534online.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, blob_bytes, fill, heap, log_region, put, set_return, strings, vtable  # noqa: E402

CONTROLS, F662, HIDE, TELL, SET_GLOBAL, SESSION_NAME = range(1, 7)
SESSION_CTOR = 10
ASSIGN, SAVE, WORDS, NAME_WORDS, FORGET, SAVES, MODE3, START, LEAVE, ACTIVITY, OPTIONS, TEXT, BOX, LOG_OFF, FIND_KEY, PUSH = range(12, 28)
HANDLER = 28
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x48b400, 0x48b5a6), (0x48b5b0, 0x48b61c), (0x48b620, 0x48b691), (0x48bc30, 0x48bc41), (0x48bc50, 0x48bcc3), (0x48bd40, 0x48c12e), (0x5da3c0, 0x5da3c1), (0x5c5520, 0x5c5523), (0x5f6a60, 0x5f6a73), (0x48b390, 0x48b3fd), (0x48bcd0, 0x48bd3d)],
          hooks={0x409300: _recorder(CONTROLS, 0), 0x662420: C(F662, 0), 0x4931b0: _recorder(HIDE, 0), 0x492e60: _recorder(TELL, 0), 0x5508b0: C(SET_GLOBAL, 1), 0x5b8160: _recorder(SESSION_NAME, 1), 0x5b50b0: _recorder(SESSION_CTOR, 0), 0x424db0: C(ASSIGN, 2), 0x414920: _recorder(SAVE, 3), 0x51ac20: _recorder(WORDS, 1), 0x5b7990: _recorder(NAME_WORDS, 3),
                 0x5c8650: _recorder(FORGET, 0), 0x4a6580: C(SAVES, 0), 0x5d5db0: C(MODE3, 0), 0x5b5cc0: _recorder(START, 2), 0x5b5e40: _recorder(LEAVE, 0), 0x5b6230: _recorder(ACTIVITY, 0), 0x4a3a30: C(OPTIONS, 0), 0x485b00: C(TEXT, 1), 0x47f020: C(BOX, 8), 0x5b6070: _recorder(LOG_OFF, 0), 0x40c870: _recorder(FIND_KEY, 2), 0x5d6110: _recorder(PUSH, 2), 0x492e00: C(HANDLER, 4)})
WINDOW, MENU, SESSION, SLOTS, HOLDER, VTABLE, PLACES, TURNS, NAME = (RAM + 0x1000 * k for k in range(1, 10))


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x1000), WINDOW: r.bytes(0x10), MENU: r.bytes(0x10), SESSION: r.bytes(0x10), SLOTS: r.bytes(0x10), VTABLE: vtable({}, size=0x10), NAME: r.bytes(8)}
    owner = bytearray(r.bytes(0x40))   # (0x786b68: its +0x3c the flag 0x786ba4)
    put(owner, 0x3c, 'I', r.deal([0, 0, 1, 5]))
    regions[0x786b68] = owner
    regions[0x7c2518] = bytes([r.deal([0, 0, 1, 0xff])])
    regions[0x7c2534] = struct.pack('<I', r.deal([MENU, MENU, MENU, 0]))
    regions[0xf7d88c] = struct.pack('<I', r.deal([SESSION, SESSION, SESSION, 0]))
    set_return(log, SESSION_CTOR, SESSION)
    regions[0xf81ebc] = struct.pack('<I', SLOTS)
    regions[0xf82884] = r.bytes(4)

    places = bytearray(r.bytes(0x60))
    holder = bytearray(r.bytes(0x18))
    for offset, at in ((4, 0), (8, 0x10), (0xc, 0x20), (0x10, 0x30), (0x14, 0x40)):
        put(holder, offset, 'I', PLACES + at)
        put(places, at, 'I', VTABLE)
    regions[HOLDER] = holder
    regions[PLACES] = places
    regions[0xf85614] = struct.pack('<I', HOLDER)
    turns = bytearray(0xa0)
    put(turns, 0, '2I', *r.deal([(0, 0), (0, 0), (0, 1), (1, 0), (0x100, 0), (0, 0x10000)]))
    regions[TURNS] = turns
    answers(log, 9, TURNS)

    function = r.deal([0x48bd40] * 12 + [0x48b400, 0x48b5b0, 0x48b620, 0x48bc30, 0x48bc50])
    if function != 0x48bd40:
        strings(log, SESSION_NAME, 0, 1 << 0)
        fill(log, SESSION_NAME, 0, struct.pack('<5I', 0, 0, HEAP + 0x800, 0, 0))
        set_return(log, SESSION_NAME, NAME)
        regions[0xf82838] = r.bytes(0x10)
        regions[0x7cf83c] = r.bytes(4)
        strings(log, SAVE, 0, 1 << 1)
        strings(log, WORDS, 0, 1 << 0)
        fill(log, WORDS, 0, r.bytes(0x40), which=1)
        blob_bytes(log, NAME_WORDS, 2, 0xffffffff)
        return Call(function, [], regions, ret='a')

    regions[0x78721c] = r.bytes(4)
    set_return(log, FIND_KEY, r.deal([0, 1, 0x10000, 0x10001, 0xffff0000]))
    set_return(log, ACTIVITY, r.deal([0, 0, 1, 0x100]))
    set_return(log, TEXT, r.getrandbits(32))
    set_return(log, HANDLER, r.getrandbits(32))
    blob_bytes(log, BOX, 0, 0xff7f)
    message = r.deal([0x100001] * 12 + [0x1000b] * 5 + [0x100000, 0x1000a, 5, r.getrandbits(32)])
    if message == 0x1000b:
        a = r.deal([0x1b, 0x1b, 0xd, 9, r.getrandbits(32)])
    else:
        a = r.deal([9, 10, 10, 10, 11, 11, 12, 13, 14, 15, 8, 16, 0xfffffff9, r.getrandbits(32)])
    return Call(0x48bd40, [WINDOW, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

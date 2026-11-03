# The game's menu's handler (src/game/ui/panel7c2534handler: 0x48b6a0) against the original. Every callee a recorder as driver.cpp's kPanel7c2534HandlerEnv; the box's record hashed but its colour's last byte (never written). Mem_Free the heap's.
# usage: panel7c2534handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob_bytes, heap, log_region, set_return  # noqa: E402

CONTROLS, F662, HIDE, TELL, DESTROY, LOAD, SAVE, OPTIONS, TEXT, BOX, ONLINE, ACTIVITY, LEAVE, SET, FIND, HANDLER = range(10, 26)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x48b6a0, 0x48bbc8), (0x5da3c0, 0x5da3c1)],
          hooks={0x409300: _recorder(CONTROLS, 0), 0x662420: C(F662, 0), 0x4931b0: _recorder(HIDE, 0), 0x492e60: _recorder(TELL, 0), 0x48c8b0: _recorder(DESTROY, 1), 0x49efb0: C(LOAD, 1), 0x49f280: C(SAVE, 1), 0x4a3a30: C(OPTIONS, 0),
                 0x485b00: C(TEXT, 1), 0x47f020: C(BOX, 8), 0x48b390: C(ONLINE, 0), 0x5b6230: _recorder(ACTIVITY, 0), 0x5b5e40: _recorder(LEAVE, 0), 0x5508b0: C(SET, 1), 0x40c870: _recorder(FIND, 2), 0x492e00: C(HANDLER, 4)})
MENU, SESSION, WORDS = RAM + 0x20000, RAM + 0x20400, RAM + 0x20800


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x100)}
    regions[0x7c2534] = struct.pack('<I', r.deal([MENU, MENU, MENU, 0]))
    regions[MENU] = r.bytes(0x20)
    regions[0x786ba4] = struct.pack('<I', r.deal([0, 0, 1, r.getrandbits(32)]))
    regions[0x7c2518] = bytes([r.deal([0, 0, 1, r.getrandbits(8)])])
    regions[0x786b68] = r.bytes(4)
    regions[0x78721c] = r.bytes(4)
    regions[0xf8288c] = r.bytes(4)
    set_return(log, ONLINE, SESSION)
    set_return(log, TEXT, WORDS)
    set_return(log, ACTIVITY, r.deal([0, 0, 1, r.getrandbits(32) & 0xffff0000]))
    set_return(log, FIND, r.deal([0, 0, 1, 0x10000, 0xffff0000, r.getrandbits(32)]))
    set_return(log, HANDLER, r.getrandbits(32))
    blob_bytes(log, BOX, 0, 0xff7f)
    message = r.deal([0x100001] * 8 + [0x1000b] * 3 + [0x100000, 0x100002, 0x1000a, 0x1000c, r.getrandbits(32)])
    a = r.deal(list(range(0x1a)) + [0x1a, 0xffffffff]) if message == 0x100001 else r.deal([0x1b, 0x1b, 0, 1, 0x1c, r.getrandbits(32)])
    return Call(0x48b6a0, [RAM + 0x20c00, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

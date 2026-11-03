# The chat's name menus' handler (src/game/ui/chatnamemenu: 0x4bede0) against the original, with 0x4051d0 (B's range; the port's Grow, copy and length) and the strings' code real; the others recorders as driver.cpp's kChatNameMenuEnv.
# A command's name logged by its string's pointer, length and room and the first 15 bytes of its text (blob_bytes).
# usage: chatnamemenu.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob_bytes, heap, log_region, set_return  # noqa: E402

DWORD, CTOR, BLACKMARK, PAGE, LOCATE, GOTO, ADD_BUDDY, REMOVE_BUDDY, ADD_IGNORE, REMOVE_IGNORE, SHOWN, UNLINK, FOCUS, DEFAULT = range(10, 24)
B = Bench([(0x4bede0, 0x4bf34c), (0x46cf10, 0x46cf80), (0x4051d0, 0x405280), (0x405280, 0x405390), (0x479230, 0x479270), (0x4f23f0, 0x4f2450), (0x67a400, 0x690000), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0)],
          hooks={0x65e705: _recorder(DWORD, 1), 0x4e5260: _recorder(CTOR, 0), 0x4ead00: _recorder(BLACKMARK, 1), 0x4eae80: _recorder(PAGE, 1), 0x4eb000: _recorder(LOCATE, 1), 0x4eb180: _recorder(GOTO, 1), 0x4eb720: _recorder(ADD_BUDDY, 1),
                 0x4eb910: _recorder(REMOVE_BUDDY, 1), 0x4ebb00: _recorder(ADD_IGNORE, 1), 0x4ebcf0: _recorder(REMOVE_IGNORE, 1), 0x65d9dd: _recorder(SHOWN, 1), 0x65f538: _recorder(UNLINK, 0), 0x65e59b: _recorder(FOCUS, 1),
                 0x65f66e: _recorder(DEFAULT, 3)})
NAME, CHAT = RAM + 0x20000, RAM + 0x20400


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x2000, r.deal([0] * 9 + [1, 2]))}
    regions[NAME] = b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(r.deal([0, 1, 7, 31, 32, 40]))) + b'\0\0'
    regions[0x7cddec] = struct.pack('<I', r.deal([CHAT, CHAT, 0]))
    regions[0x7cc8cc] = r.bytes(4)
    regions[0x7cc8bc] = r.bytes(4)
    set_return(log, DWORD, NAME)
    set_return(log, CTOR, CHAT)
    set_return(log, DEFAULT, r.getrandbits(32))
    for k in range(BLACKMARK, REMOVE_IGNORE + 1):
        blob_bytes(log, k, 0, 0xfff0, (4, 0x7fff))
    message = r.deal([0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x1e, 0xff, 0x101, r.getrandbits(32)])
    a = r.deal([0x1789, 0x178a, 0x178b, 0x178c, 0x178c, 0x178d, 0x178e, 0x178f, 0x1790, 0x3003, 0x3004, 0x3005, 0x3006, 0x3006, 0x3007, 0x3008, r.getrandbits(32)])
    return Call(0x4bede0, [RAM + 0x20800, message, a, r.deal([0, 1, 1, 2, r.getrandbits(32)])], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

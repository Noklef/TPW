# The game window's message procedure (src/game/ui/windowproc: 0x46b600) against the original: every message it knows and others, the mouse's before all else, the screen's size words (0 now and then), keys of the numeric pad or not, keys held, every callee a recorder.
# usage: windowproc.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, blob, fill, import_table, log_region, set_return  # noqa: E402

SIZE, MOVE, DOWN, UP, WHEEL, KEY_DOWN, KEY_UP, KEY_CHAR, TELL_ON, SOUND_ON, MOVIE, TELL_OFF, SOUND_OFF, POST, REMAP = range(1, 16)
CD = {0x6590d0: (SIZE, 2), 0x6588ef: (MOVE, 2), 0x658af1: (DOWN, 3), 0x658b5b: (UP, 3), 0x658bcb: (WHEEL, 4), 0x658c38: (KEY_DOWN, 2), 0x658c67: (KEY_UP, 2), 0x658c96: (KEY_CHAR, 2),
      0x659333: (TELL_ON, 0), 0x51bd30: (SOUND_ON, 0), 0x51b350: (MOVIE, 0), 0x6593c5: (TELL_OFF, 0), 0x51bcf0: (SOUND_OFF, 0), 0x47e9d0: (POST, 1), 0x46b460: (REMAP, 1)}
RELEASE, FIRE_DOWN, FIRE_UP, PAUSE, HEBREW = 16, 17, 18, 19, 20
TH = {0x58dd60: (RELEASE, 1), 0x40c900: (FIRE_DOWN, 2), 0x40c990: (FIRE_UP, 2), 0x4092a0: (PAUSE, 2), 0x41a850: (HEBREW, 0)}
HOOKS = {a: _recorder(k, n, cdecl=True) for a, (k, n) in CD.items()}
HOOKS.update({a: _recorder(k, n) for a, (k, n) in TH.items()})
B = Bench([(0x46b600, 0x46bc76), (0x47eaa0, 0x47eaa6)], hooks=HOOKS)
MBTOWC, DEFAULT, MAP_KEY, KEY_STATE, SHOW_CURSOR = 40, 41, 42, 43, 44
IMPORTS = {'MultiByteToWideChar': (MBTOWC, 0x6fd174, 6, 0, 0x10), 'DefWindowProcA': (DEFAULT, 0x6fd340, 4, 0, 0), 'MapVirtualKeyA': (MAP_KEY, 0x6fd344, 2, 0, 0), 'GetKeyState': (KEY_STATE, 0x6fd324, 1, 0, 0), 'ShowCursor': (SHOW_CURSOR, 0x6fd30c, 1, 0, 0)}
STATES = RAM
MESSAGES = [0x200, 0x201, 0x202, 0x203, 0x204, 0x205, 0x206, 0x207, 0x208, 0x209, 0x1c, 0x1c, 0x1c, 0x14, 0x20, 0x100, 0x100, 0x100, 0x101, 0x101, 0x102, 0x102, 0x102, 0x103, 0x104, 0x104, 0x105, 0x105, 0x20a, 0x20a, 0x410, 0xc123, 0xc123, 0x1b, 0x1d, 0xff, 0x106, 0x20b, 0x40f, 0x411]
PAD = [0x6a, 0x6b, 0x6d, 0x6e, 0x6f]


def case(r):
    log = log_region()
    table = import_table(log, IMPORTS)
    struct.pack_into('<I', log, LOG_BYTES + 4 * SIZE, 3 << 16)   # (the words of the two sizes, over registers that hold other bytes above)
    blob(log, MBTOWC, 2, 1)   # (what is made wide, on the stack: one dword)
    fill(log, MBTOWC, 4, r.choice([r.bytes(2), r.bytes(4), struct.pack('<H', r.randrange(0x1e, 0x22)), struct.pack('<I', 0x41), struct.pack('<HH', r.randrange(0x1e, 0x22), r.getrandbits(16) | 1)]))
    set_return(log, MBTOWC, r.choice([1, 1, 1, 2, 0, 0xffffffff]))
    set_return(log, DEFAULT, r.getrandbits(32))
    set_return(log, MAP_KEY, r.choice([0, 0x41, 0x10000, 0x12345641, r.getrandbits(32)]))
    set_return(log, HEBREW, r.choice([0, 0, 1, 0x100, 0x101]))
    set_return(log, REMAP, r.getrandbits(32))
    answers(log, KEY_STATE, STATES)
    message = r.choice(MESSAGES)
    wparam = r.choice([r.choice(PAD), r.choice(PAD), 0x41, 0x70, 0, 0x10000 | r.choice(PAD), r.getrandbits(16) << 16 | r.choice(PAD), r.getrandbits(32), r.getrandbits(16), 0xff78 << 16])
    if message == 0x1c:
        wparam = r.choice([0, 0, 1, r.getrandbits(32)])
    lparam = r.getrandbits(32)
    size = struct.pack('<hxxh', r.choice([600, 768, 480, 1, -3, r.randrange(1, 0x7fff)]), r.choice([800] * 9 + [1024, 640, 1, -7, r.randrange(1, 0x7fff), r.randrange(1, 0x7fff), 0]))
    regions = {LOG: log, IMPORT_TABLE: table, STATES: struct.pack('<40I', *(r.choice([0, 1, 0x8000, 0xffff8001, 0x10000, 0xffff0000]) for _ in range(40))), 0xfaa5c0: size,
               0x7a1968: r.bytes(4), 0x7a1930: r.bytes(4), 0x7b4b88: struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)])), 0x8c4b44: struct.pack('<I', r.choice([0, RAM + 0x100])), 0x7a1a14: r.bytes(4),
               0xf82884: struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)])), 0x78718c: struct.pack('<I', RAM + 0x200), 0x7a1c5c: struct.pack('<I', r.choice([0xc123, 0xc123, 0x1b, message, r.getrandbits(32)]))}
    return Call(0x46b600, [RAM + 0x300, message, wparam, lparam], regions, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)

# The dialog of the script 0x74fb20 in the overlay [0x7cb2b4] (src/game/ui/dialog74fb20: Dialog74fb20_Open 0x48adb0 and Dialog74fb20_Handler 0x48a740) against the original. Every call a recorder (their own benches have them;
# driver.cpp's kDialog74fb20Env): Global7cb2e8_Get 10, Overlay7cb2b4_Toggle 11, WindowScript_Build thiscall 12, Window_GetRect thiscall 13 (the rectangle the stack's, logged as 0; its width and height words filled),
# Window_SetWordsE4 thiscall 14 (the words logged: the original's upper halves are dirty), Window_RunScript thiscall 15, Window_SetFocus thiscall 16, GuiClock_Now 17, Window_SendMessage 18, the unported 0x42a190 19, KeyMaps_EnableAll 20,
# Window_CallDefault thiscall 21. The handler's messages 0x14, 0x15, 0x10006 and 0x1000b mostly; its flag [0x78d911], the clock from [0x7c2508] at or over 0x7d0, up [0x7c24cc], the key 0x1b and the mask 0xff0000 of b.
# usage: dialog74fb20.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Open,Handler)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, fill, log_region, set_return, strings  # noqa: E402

CALLS = {'Open': 0x48adb0, 'Handler': 0x48a740}
B = Bench([(0x48adb0, 0x48ae6c), (0x48a740, 0x48a96a), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b)],
          hooks={0x4a2a90: _recorder(10, 0, cdecl=True), 0x4a29d0: _recorder(11, 0, cdecl=True), 0x65fd0e: _recorder(12, 2), 0x65d42e: _recorder(13, 2), 0x65f345: _recorder(14, 2),
                 0x65e72d: _recorder(15, 2), 0x65e59b: _recorder(16, 1), 0x65968e: _recorder(17, 0, cdecl=True), 0x658d9f: _recorder(18, 4, cdecl=True), 0x42a190: _recorder(19, 0, cdecl=True),
                 0x40cf60: _recorder(20, 0, cdecl=True), 0x65f66e: _recorder(21, 3)})
WINDOW = RAM + 0x100
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def world(r, log):
    struct.pack_into('<I', log, LOG_BYTES + 4 * 14, 0b11 << 16)
    strings(log, 13, 0, 0b1)
    fill(log, 13, 0, struct.pack('<hh', r.choice([0, 1, 640, 800, -1, -640, r.randrange(-0x8000, 0x8000)]), r.choice([0, 1, 480, 600, -1, -480, r.randrange(-0x8000, 0x8000)])), offset=8)
    set_return(log, 12, r.getrandbits(32))
    return {LOG: log, 0x7c24c8: r.bytes(12), 0x7cb2b4: r.bytes(4)}


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = world(r, log)
    if op == 'Open':
        set_return(log, 10, r.choice([0, 0, 1, r.getrandbits(32)]))
        return Call(CALLS['Open'], [], regions, ret='v')
    regions[0x78d910] = bytes([r.getrandbits(8), r.choice([0, 1, 1, 1, r.getrandbits(8)])])
    since = r.randrange(0, 1 << 31)
    regions[0x7c2508] = struct.pack('<I', since)
    set_return(log, 17, since + r.choice([0x7d0, 0x7d1, 0x7d1, 5000, 5000, 0, -1, r.randrange(-1 << 31, 1 << 31)]))
    set_return(log, 21, r.getrandbits(32))
    up = bytearray(regions[0x7c24c8])
    up[4:8] = struct.pack('<I', r.choice([1, 1, 1, 0, r.getrandbits(32)]))
    regions[0x7c24c8] = up
    message = r.choice([0x14, 0x15, 0x10006, 0x10006, 0x10006, 0x1000b, 0x1000b, 0x1000b, 0x13, 0x16, 0x10005, 0x10007, 0x1000a, 0x1000c, r.getrandbits(32)])
    a = r.choice([1, 1, 1, 0x1b, 0x1b, 0x1b, 0, 2, r.getrandbits(32)])
    b = r.getrandbits(32) & r.choice([0xff00ffff, 0xff00ffff, 0xff00ffff, 0xffffffff])
    return Call(CALLS['Handler'], [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

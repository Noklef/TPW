# The front end's windows and panels made (src/game/ui/panel7b9940: Panel7b9940_Open, 0x489f50) against the original. Every call a recorder (their own benches have them; driver.cpp's kPanel7b9940OpenEnv): Gui_ResetQueues 10,
# MsgTags_Create 11, Mem_Alloc 12, MouseModeOnlineBase_Ctor 13, MouseMode_Set 14, Renderer_Add 15, Window_CreateChild 16, Window_SetShown 17, Window_SetHandler 18, Window_GetRect 19 (its rectangle filled), Window_SetCursor 20,
# WindowScript_Build 21, the six dialogs 22-27, Focus_Restore 28, the manager's slots 29 and 30; [0x7b05d8]'s slot 9 a vtable recorder. SceneFlags_Touch and the GuiRect's run. Each allocation and each window failing now and then.
# usage: panel7b9940open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fail_mask, fill, log_region, put, strings, vtable  # noqa: E402

B = Bench([(0x489f50, 0x48a288), (0x5da3c0, 0x5da3c1), (0x5836b0, 0x5836c2), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b)],
          hooks={0x659064: _recorder(10, 0, cdecl=True), 0x48eaa0: _recorder(11, 0, cdecl=True), 0x67acc0: _recorder(12, 1, cdecl=True), 0x46c6a0: _recorder(13, 0), 0x46c350: _recorder(14, 1, True), 0x576410: _recorder(15, 2),
                 0x65dd7b: _recorder(16, 7), 0x65d9dd: _recorder(17, 1), 0x65e526: _recorder(18, 1), 0x65d42e: _recorder(19, 2), 0x65f11d: _recorder(20, 1), 0x65fd0e: _recorder(21, 2), 0x4a1d70: _recorder(22, 0, cdecl=True),
                 0x498b50: _recorder(23, 0, cdecl=True), 0x4b3cf0: _recorder(24, 0, cdecl=True), 0x497b20: _recorder(25, 0, cdecl=True), 0x48d8b0: _recorder(26, 0, cdecl=True), 0x498790: _recorder(27, 0, cdecl=True),
                 0x4862a0: _recorder(28, 0, cdecl=True), 0x659607: _recorder(29, 0, cdecl=True), 0x65961c: _recorder(30, 0, cdecl=True)})
TABLE, OBJECT, VT = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    log = log_region()
    fail_mask(log, 12, r.choice([3, 3, 3, 1, 0]))
    fail_mask(log, 16, r.choice([3, 3, 3, 1, 2, 0]))
    strings(log, 19, 0, out=1)
    extreme = r.random() < 0.15
    corner = (lambda: r.randrange(-0x8000, 0x8000)) if extreme else (lambda: r.randrange(-40, 900))
    fill(log, 19, 0, struct.pack('<4h', corner(), corner(), corner(), corner()), offset=4)
    second, virtual = r.choice([(0x10, 8), (0x20, 8), (0x40, 8), (0x40, 0x20)])
    regions = {LOG: log, TABLE: struct.pack('<III', r.getrandbits(32), virtual, second), 0x7b9940: put(bytearray(r.bytes(0x100)), 0, 'I', TABLE), OBJECT: put(bytearray(0x10), 0, 'I', VT), VT: vtable({}), 0x8bd508: r.bytes(8),
               0x87a6d0: r.bytes(4), 0xfaa5d8: r.bytes(4), 0x7b05e8: struct.pack('<I', r.choice([0, 0, 0, r.getrandbits(32) | 1])), 0x7b05e0: r.bytes(4), 0x7b05d4: r.bytes(4), 0x7b05d8: struct.pack('<I', r.choice([0, OBJECT])),
               0x7cb2ac: r.bytes(4), 0x7cb2b0: r.bytes(4), 0x7c24c8: r.bytes(4), 0x78d90e: bytes([r.choice([0, 0, 1, 0x80])]), 0xfaa5c0: struct.pack('<h', r.choice([480, 600, 768, 0, r.randrange(-0x8000, 0x8000)])),
               0xfaa5c4: struct.pack('<h', r.choice([640, 800, 1024, 0, r.randrange(-0x8000, 0x8000)])), 0x7a1c54: r.bytes(8), 0x74d104: r.bytes(8)}
    return Call(0x489f50, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

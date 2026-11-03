# The staff rename dialog opened (src/game/ui/staffrename: StaffRename_Open, 0x4b6110) against the original (the handler's bench is its own). Every call a recorder (their own benches have them; driver.cpp's kStaffRenameEnv): WindowScript_Build 10,
# Window_SetDepth 11, Window_SetDwordA0 12, GameHeap_Alloc 13, TextPart6ff530_Ctor 14, Window_SetImpl 15, Language_Font 16, Window_Set4c 17, Window_NotifyC00 18, the children's slot 0x24 19, Language_WideText 20, HoverLabel_SetYellow 21,
# TextPart6ff4d0_Ctor 22, Window_Notify40000 23, Window705a20_SetBuffer 24, GuiWindows_FindByName 25, Window705a20_SetCaret 26, Staff_GetName 27, Window705a20_SelectAll 28, KeyMaps_ResetAllAndSetFlag 29, Window_SetFocus 30; Window_FindChild runs.
# usage: staffrenameopen.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x4b6110, 0x4b631f), (0x65d7bc, 0x65d7d0), (0x65d7d0, 0x65d840)],
          hooks={0x65fd0e: _recorder(10, 2), 0x65f16b: _recorder(11, 1), 0x65e6d5: _recorder(12, 2), 0x67acc0: _recorder(13, 1, cdecl=True), 0x491920: _recorder(14, 1), 0x65d1c8: _recorder(15, 1), 0x485a70: _recorder(16, 1, cdecl=True),
                 0x65c388: _recorder(17, 1), 0x65c428: _recorder(18, 2), 0x485b00: _recorder(20, 1, cdecl=True), 0x492d20: _recorder(21, 1, cdecl=True), 0x491170: _recorder(22, 1), 0x65c5d5: _recorder(23, 4), 0x6662b7: _recorder(24, 2),
                 0x659bc9: _recorder(25, 1, cdecl=True), 0x66656c: _recorder(26, 1), 0x506860: _recorder(27, 0), 0x6677ae: _recorder(28, 0), 0x486b70: _recorder(29, 0, cdecl=True), 0x65e59b: _recorder(30, 1)})
SELF, PARENT, WINDOW, KIDS, VT, THING = RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x600, RAM + 0x800, RAM + 0xa00


def case(r):
    log = log_region()
    set_return(log, 10, WINDOW)
    set_return(log, 13, r.choice([RAM + 0x1000, RAM + 0x1000, RAM + 0x1000, 0]))
    for k in (14, 16, 20, 22, 25, 27):
        set_return(log, k, r.getrandbits(32))
    ids = [0x5daa, 0x5dab]
    r.shuffle(ids)
    kids = bytearray()
    for i, ident in enumerate(ids):
        kid = bytearray(r.bytes(0x100))
        put(kid, 0, 'I', VT)
        put(kid, 0x8c, 'I', ident)
        put(kid, 0x54, 'I', KIDS + 0x100 if i == 0 else 0)
        kids += kid
    self_ = bytearray(r.bytes(0x20))
    put(self_, 0xc, 'I', r.choice([0, 0, 0, 0, 0, r.getrandbits(32) | 1]))
    put(self_, 0x10, 'I', PARENT)
    thing = bytearray(r.bytes(0x10))
    thing[2] = r.choice([4, 5, 6, 7, 8, 4, 5, 6, 7, 8, 3, 9, 0, 0x84, r.getrandbits(8)])
    regions = {LOG: log, SELF: self_, PARENT: put(bytearray(r.bytes(0x100)), 0xe0, 'h', r.choice([0, 1, 5, -1, -20, r.randrange(-0x8000, 0x8000)])), WINDOW: put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS), KIDS: bytes(kids),
               VT: put(vtable({}, size=0x10), 0x24, 'I', recorder(19, 1)), THING: thing, 0x7c2654: r.bytes(4) + struct.pack('<I', r.choice([THING, THING, THING, THING, 0]))}
    return Call(0x4b6110, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

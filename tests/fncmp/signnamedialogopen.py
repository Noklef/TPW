# The dialogs that name a thing opened (src/game/ui/signnamedialog: SignNameDialog_Open 0x48d370, SignNameDialog7ca214_Open 0x4990f0) against the originals. Every call a recorder (their own benches have them; driver.cpp's kSignNameDialogOpenEnv):
# the children's slot 9, WindowScript_Build 10, Mem_Alloc 11, TextPart6ff530_Ctor 12, Window_SetImpl 13, Language_Font 14, Window_Set4c 15, Window_NotifyC00 16, Language_WideText 17, HoverLabel_SetYellow 18, Window_SetDwordA0 19, 0x4e2860 20,
# GuiWindows_FindByName 21, 0x4e2920 22, Window705a20_SelectAll 23, TextPart6ff4d0_Ctor 24, Window_Notify40000 25, Window_SetFocus 26, Window705a20_SetBuffer 27, Window705a20_SetCaret 28, KeyMaps_ResetAllAndSetFlag 29. Window_FindChild and Window_SetDepth run.
# usage: signnamedialogopen.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fail_mask, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x48d370, 0x48d638), (0x4990f0, 0x4993d3), (0x65d7bc, 0x65d840), (0x65f16b, 0x65f246), (0x5da3c0, 0x5da3c1)],
          hooks={0x65fd0e: _recorder(10, 2), 0x67acc0: _recorder(11, 1, cdecl=True), 0x491920: _recorder(12, 1), 0x65d1c8: _recorder(13, 1), 0x485a70: _recorder(14, 1, cdecl=True), 0x65c388: _recorder(15, 1), 0x65c428: _recorder(16, 2),
                 0x485b00: _recorder(17, 1, cdecl=True), 0x492d20: _recorder(18, 1, cdecl=True), 0x65e6d5: _recorder(19, 2), 0x4e2860: _recorder(20, 2), 0x659bc9: _recorder(21, 1, cdecl=True), 0x4e2920: _recorder(22, 2), 0x6677ae: _recorder(23, 0),
                 0x491170: _recorder(24, 1), 0x65c5d5: _recorder(25, 4), 0x65e59b: _recorder(26, 1), 0x6662b7: _recorder(27, 2), 0x66656c: _recorder(28, 1), 0x486b70: _recorder(29, 0, cdecl=True)})
SELF, PARENT, THING, WINDOW, KIDS, VT, INDEX = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x800, RAM + 0x900


def case(r):
    op = r.deal([0x48d370, 0x4990f0])
    log = log_region()
    set_return(log, 10, WINDOW)
    fail_mask(log, 11, r.choice([7, 7, 7, 6, 5, 3, r.randrange(8)]))
    for k in (12, 14, 17, 21, 24):
        set_return(log, k, r.getrandbits(32))
    ids = [0x3943e, 0x3943c, 0x3943d]
    r.shuffle(ids)
    kids = bytearray()
    for i, ident in enumerate(ids):
        kid = bytearray(r.bytes(0x100))
        put(kid, 0, 'I', VT)
        put(kid, 0x8c, 'I', ident)
        put(kid, 0x54, 'I', KIDS + 0x100 * (i + 1) if i < 2 else 0)
        put(kid, 0x5c, 'I', 0)
        put(kid, 0x98, '2I', 0, 0)
        kids += kid
    window = put(put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS), 0x98, '2I', 0, 0)
    parent = put(bytearray(r.bytes(0x100)), 0xe0, 'h', r.choice([0, 1, 5, -1, -20, 0x7fff, r.randrange(-0x8000, 0x8000)]))
    thing = bytearray(r.bytes(0x20))
    thing[2] = r.choice([3, 3, 3, 3, 3, 2, 4, r.randrange(256)])
    regions = {LOG: log, PARENT: parent, WINDOW: window, KIDS: bytes(kids), VT: vtable({9: 1}), THING: thing}
    if op == 0x48d370:
        self_ = bytearray(r.bytes(0x20))
        put(self_, 0xc, 'I', r.choice([0] * 9 + [r.getrandbits(32) | 1]))
        put(self_, 0x10, 'I', PARENT)
        regions[SELF] = self_
        regions[0x7c2658] = struct.pack('<I', r.choice([THING] * 9 + [0]))
        return Call(op, [SELF], regions, conv='t', ret='v')
    index = r.randrange(0x20)
    regions[INDEX] = struct.pack('<H', index)
    regions[0x7cf83c] = struct.pack('<I', INDEX - 0x1da732 & 0xffffffff)
    regions[0x7cfb90] = struct.pack('<I', r.getrandbits(32) | 1) + b''.join(struct.pack('<I', THING) for _ in range(0x20 * 5))
    regions[0x7ca210] = struct.pack('<I', PARENT)
    regions[0x7ca214] = struct.pack('<I', r.choice([0] * 9 + [r.getrandbits(32) | 1]))
    return Call(op, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

# The dialog [0x7cc188] opened (src/game/ui/dialog7cc188: Dialog7cc188_Open, 0x4a8140) against the original. Every call a recorder (their own benches have them; driver.cpp's kDialog7cc188OpenEnv): WindowScript_Build 10,
# Mem_Alloc 11, TextPart6ff530_Ctor 12, Window_SetImpl 13, Language_Font 14, Window_Set4c 15, Window_NotifyC00 16, Language_WideText 17, HoverLabel_SetYellow 18, Window7059f0_BuildRows 19, Window7059f0_SetColumnBottom 20,
# GuiWindows_FindByName 21, Window7059f0_SetHighlight 22, Dialog7cc188_FillList 23, TextPart6ff4d0_Ctor 24, Window_Notify40000 25, Window_SetHandler 26, Window705a20_SetBuffer 27, Window705a20_SetCaret 28, Advisor_PostId 29, the
# children's slot 9 30; Window_FindChild and Window_SetDepth run. The window set already now and then, each allocation failing now and then.
# usage: dialog7cc188open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x4a8140, 0x4a8379), (0x65d7bc, 0x65d840), (0x65f16b, 0x65f246)],
          hooks={0x65fd0e: _recorder(10, 2), 0x67acc0: _recorder(11, 1, cdecl=True), 0x491920: _recorder(12, 1), 0x65d1c8: _recorder(13, 1), 0x485a70: _recorder(14, 1, cdecl=True), 0x65c388: _recorder(15, 1),
                 0x65c428: _recorder(16, 2), 0x485b00: _recorder(17, 1, cdecl=True), 0x492d20: _recorder(18, 1, cdecl=True), 0x6639cb: _recorder(19, 2), 0x6636b2: _recorder(20, 2), 0x659bc9: _recorder(21, 1, cdecl=True),
                 0x662eee: _recorder(22, 1), 0x4a7a80: _recorder(23, 1, cdecl=True), 0x491170: _recorder(24, 1), 0x65c5d5: _recorder(25, 4), 0x65e526: _recorder(26, 1), 0x6662b7: _recorder(27, 2), 0x66656c: _recorder(28, 1),
                 0x486b00: _recorder(29, 1, cdecl=True)})
PARENT, WINDOW, KIDS, VT = RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x800


def case(r):
    log = log_region()
    set_return(log, 10, WINDOW)
    set_return(log, 11, r.choice([RAM + 0x1000] * 6 + [0]))
    for k in (12, 14, 17, 21, 24):
        set_return(log, k, r.getrandbits(32))
    ids = [0x27, 0x26, 0x25]
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
    regions = {LOG: log, PARENT: put(bytearray(r.bytes(0x100)), 0xe0, 'h', r.choice([0, 1, 5, -1, -20, 0xf5, 0xf6, 0xf7, r.randrange(-0x8000, 0x8000)])), WINDOW: window, KIDS: bytes(kids),
               VT: put(vtable({}, size=0x10), 0x24, 'I', recorder(30, 1)), 0x7cc184: struct.pack('<I', PARENT), 0x7cc188: struct.pack('<I', r.choice([0] * 9 + [r.getrandbits(32) | 1]))}
    return Call(0x4a8140, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

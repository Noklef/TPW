# The dialog [0x7cc184] opened (src/game/ui/dialog7cc184: Dialog7cc184_Open, 0x4a8c00) against the original. Every call a recorder (their own benches have them; driver.cpp's kDialog7cc184OpenEnv): Setting_ReadMode 0, atexit 1, the holder's
# constructor 2, Online_Restricted 3, CommandQueue_Post 4, Window7c24c8_Close 5, Window7c24c8_Set 6, Window_ImplSlot6 7, the edits' slot 9, WindowScript_Build 10, Mem_Alloc 11, HoverLabel_Attach 12, Window_SetImpl 13, Language_Font 14,
# Window_Set4c 15, Window_NotifyC00 16, Language_WideText 17, Window_Notify3000 18, RendererPart_Ctor 19, Window_ImplSlot7 20 (its two outputs on the stack, filled), GuiWindows_FindByName 21, Panel_Reset 22, Panel_ShrinkInto 23,
# TextPart6ff4d0_Ctor 24, Window_Notify40000 25, Window_SetHandler 26, Window705a20_SetBuffer 27, Window705a20_SetCaret 28, Advisor_PostId 29. Window_FindChild runs on the children, each missing now and then.
# usage: dialog7cc184open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fail_mask, fill, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x4a8c00, 0x4a9176), (0x65d7bc, 0x65d840), (0x5fa680, 0x5fa6a0), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b)],
          hooks={0x550ca0: _recorder(0, 0), 0x67a7d0: _recorder(1, 1, cdecl=True), 0x5b50b0: _recorder(2, 0), 0x5b6460: _recorder(3, 0), 0x47f410: _recorder(4, 2, cdecl=True), 0x485b40: _recorder(5, 0, cdecl=True), 0x485b70: _recorder(6, 1, cdecl=True),
                 0x65d745: _recorder(7, 0), 0x65fd0e: _recorder(10, 2), 0x67acc0: _recorder(11, 1, cdecl=True), 0x485d20: _recorder(12, 3, cdecl=True), 0x65d1c8: _recorder(13, 1), 0x485a70: _recorder(14, 1, cdecl=True), 0x65c388: _recorder(15, 1),
                 0x65c428: _recorder(16, 2), 0x485b00: _recorder(17, 1, cdecl=True), 0x65c4b9: _recorder(18, 2), 0x48f150: _recorder(19, 1), 0x65d6db: _recorder(20, 2), 0x659bc9: _recorder(21, 1, cdecl=True), 0x5d48d0: _recorder(22, 0),
                 0x5d4970: _recorder(23, 4), 0x491170: _recorder(24, 1), 0x65c5d5: _recorder(25, 4), 0x65e526: _recorder(26, 1), 0x6662b7: _recorder(27, 2), 0x66656c: _recorder(28, 1), 0x486b00: _recorder(29, 1, cdecl=True)})
WINDOW, KIDS, VT = RAM + 0x100, RAM + 0x400, RAM + 0x800


def case(r):
    log = log_region()
    set_return(log, 10, r.choice([WINDOW] * 9 + [0]))
    set_return(log, 3, r.choice([1] * 9 + [0, 0x100]))
    fail_mask(log, 11, r.choice([0xff, 0xff, 0xff, 0xfe, 0xfd, 0xfb, 0xf7, r.randrange(256)]))
    for k in (2, 14, 17, 19, 21, 24):
        set_return(log, k, r.getrandbits(32))
    strings(log, 20, 0, out=0b11)
    fill(log, 20, 0, r.bytes(4))
    fill(log, 20, 1, r.bytes(4), which=1)
    set_return(log, 20, r.choice([r.getrandbits(32) | 1] * 5 + [0]))
    ids = [ident for ident in (0x5da5, 0x5da4, 0x5da3, 0x5d9f) if r.random() < 0.85]
    r.shuffle(ids)
    kids = bytearray()
    for i, ident in enumerate(ids):
        kid = bytearray(r.bytes(0x100))
        put(kid, 0, 'I', VT)
        put(kid, 0x8c, 'I', ident)
        put(kid, 0x54, 'I', KIDS + 0x100 * (i + 1) if i < len(ids) - 1 else 0)
        put(kid, 0x5c, 'I', 0)
        kids += kid
    window = put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS if ids else 0)
    regions = {LOG: log, WINDOW: window, VT: vtable({9: 1}), 0x7cc184: struct.pack('<I', r.choice([0] * 9 + [r.getrandbits(32) | 1])), 0xf7d88c: struct.pack('<I', r.choice([r.getrandbits(32) | 1] * 5 + [0])),
               0xfb34bf: bytes([r.choice([1, 1, 3, 0, 0xfe])]), 0xfb3b7c: struct.pack('<I', r.choice([1, 1, 0, 2, 0x101])), 0x7cc8c0: r.bytes(4), 0x7cb2ac: r.bytes(4), 0x7cb408: r.bytes(0xc8), 0x7cb4d0: r.bytes(0x408), 0x7cbf3c: r.bytes(8),
               0x7cbf48: r.bytes(0x208), 0x7cc18c: struct.pack('<I', r.choice([r.getrandbits(32) | 1] * 5 + [0])), 0x8bd554: struct.pack('<I', r.choice([2, 2, 3, 3, 0, 1, 4]))}
    if ids:
        regions[KIDS] = bytes(kids)
    return Call(0x4a8c00, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

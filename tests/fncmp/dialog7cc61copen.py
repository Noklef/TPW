# The window [0x7cc61c] made (src/game/ui/dialog7cc61c: Dialog7cc61c_Open, 0x4bb800) against the original. Every call a recorder (their own benches have them; driver.cpp's kDialog7cc61cOpenEnv): Dialog_Create 10, Mem_Alloc 11, TextPart_Ctor 12,
# Window_SetImpl 13, Language_Font 14, Window_Set4c 15, Window_NotifyImpl100 16, Window_Notify3000 17, Window_NotifyC00 18, Dialog7cc61c_FitText 19, Window_SetHandler 20, RadioGroup_Select 21, Window_Notify40000 22, Language_WideText 23,
# HoverLabel_Attach 24, Window_SetFocus 25; the buttons' slot 9 a vtable recorder. Window_FindChild and Window_SetDepth run on the tree by hand. The window up, no owner or a failed allocation now and then.
# usage: dialog7cc61copen.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fail_mask, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x4bb800, 0x4bba7a), (0x65d7bc, 0x65d840), (0x65f16b, 0x65f246)],
          hooks={0x47ed80: _recorder(10, 3, cdecl=True), 0x67acc0: _recorder(11, 1, cdecl=True), 0x48f6a0: _recorder(12, 1), 0x65d1c8: _recorder(13, 1), 0x485a70: _recorder(14, 1, cdecl=True), 0x65c388: _recorder(15, 1),
                 0x65f5b7: _recorder(16, 1), 0x65c4b9: _recorder(17, 2), 0x65c428: _recorder(18, 2), 0x4bb460: _recorder(19, 0, cdecl=True), 0x65e526: _recorder(20, 1), 0x66a295: _recorder(21, 1), 0x65c5d5: _recorder(22, 4),
                 0x485b00: _recorder(23, 1, cdecl=True), 0x485d20: _recorder(24, 3, cdecl=True), 0x65e59b: _recorder(25, 1)})
DIALOG, PANEL, GROUP, FIRST, SECOND, VT = (RAM + 0x100 * k for k in range(6))


def window(r, ident, first=0, following=0):
    w = bytearray(r.bytes(0x100))
    put(w, 0, 'I', VT)
    put(w, 0x54, 'I', following)
    put(w, 0x5c, 'I', first)
    put(w, 0x8c, 'I', ident)
    put(w, 0x98, '2I', 0, 0)
    return w


def case(r):
    log = log_region()
    set_return(log, 10, DIALOG)
    fail_mask(log, 11, r.choice([7, 7, 7, 6, 5, 3, r.randrange(8)]))
    for k in (12, 14, 23):
        set_return(log, k, r.getrandbits(32))
    dialog = put(window(r, r.getrandbits(32), PANEL), 0xe0, 'h', r.choice([0, 1, 0x20, -4, -1, 0x7fff, r.randrange(-0x8000, 0x8000)]))
    regions = {LOG: log, DIALOG: dialog, PANEL: window(r, 0x5b7f, GROUP), GROUP: window(r, 0x5b81, FIRST), FIRST: window(r, 0x5b82, 0, SECOND), SECOND: window(r, 0x5b83), VT: vtable({9: 1}),
               0x7cc61c: struct.pack('<I', r.choice([0] * 9 + [r.getrandbits(32) | 1])), 0xf82884: struct.pack('<I', r.choice([1] * 9 + [0])), 0x7cc620: r.bytes(1), 0x758870: r.bytes(4)}
    return Call(0x4bb800, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

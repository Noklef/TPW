# Dialog7cc61c_FitText (src/game/ui/dialog7cc61c, 0x4bb460) against the original. Recorders (Dialog7cc61cTextEnv): Mem_Alloc 10, the holder's constructor 0x5b50b0 11 (agent B's), GuiText_Measure 12 (its box filled, the bounds hashed), Language_Font 13, Window_SetShown 14, Window705b20_SetRange 15; the font's slot 1, the bar's slot 7 and the
# panel's slot 9 vtable recorders. The windows by hand, their saved rectangles real; the holder [0xf7d88c] absent a quarter of the time; a line height of 0 (the divide faults in both) and a failed allocation (no holder: faults in both) now and then.
# usage: dialog7cc61ctext.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob, fill, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x4bb460, 0x4bb68f), (0x65d7bc, 0x65d840), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b), (0x65d4f6, 0x65d520)],
          hooks={0x67acc0: _recorder(10, 1, cdecl=True), 0x5b50b0: _recorder(11, 0), 0x4929f0: _recorder(12, 7, cdecl=True), 0x485a70: _recorder(13, 1, cdecl=True), 0x65d9dd: _recorder(14, 1), 0x66b47d: _recorder(15, 2)})
DIALOG, PANEL, BAR, HOLDER, FONT, VT = (RAM + 0x1000 * k for k in range(6))


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 12, 0b100)   # (the font: a byte)
    blob(log, 12, 1, 0b1111)
    strings(log, 12, 0, out=0b1000000)
    high = r.choice([0, 1, 16, 300, 600, 2000, r.randrange(-50, 3000)])
    scale, room = r.choice([480, 600, 768, 1, r.randrange(1, 0x7fff)]), None
    if r.random() < 0.3:   # (the text's height just the panel's, or one off)
        scale, room = 0x600, r.randrange(400)
        high = room + r.choice([0, 0, -1, 1])
    fill(log, 12, 6, struct.pack('<4i', r.getrandbits(8), r.getrandbits(8), r.getrandbits(8), high))
    set_return(log, 10, r.choice([HOLDER, HOLDER, HOLDER, 0x10000, 0]))
    set_return(log, 11, HOLDER)
    set_return(log, 13, FONT)
    set_return(log, 1, r.choice([12, 16, 20, 1, 0xffff0010, 0x10010, 0, r.getrandbits(16)]))
    set_return(log, 7, r.choice([0, 0, 0x10000, 1, r.getrandbits(32)]))
    dialog = put(bytearray(r.bytes(0x100)), 0x5c, 'I', PANEL)
    panel = bytearray(r.bytes(0x100))
    put(panel, 0, 'I', VT)
    put(panel, 0x54, 'I', 0)
    put(panel, 0x5c, 'I', BAR)
    put(panel, 0x8c, 'I', 0x5b7f)
    if room is not None:
        top = r.randrange(300)
        put(panel, 0x2c + 6, 'h', top)
        put(panel, 0x2c + 0xa, 'h', top + room)
    bar = bytearray(r.bytes(0x100))
    put(bar, 0, 'I', VT)
    put(bar, 0x54, 'I', 0)
    put(bar, 0x8c, 'I', 0x5b80)
    holder = bytearray(r.bytes(0x100))
    regions = {LOG: log, DIALOG: dialog, PANEL: panel, BAR: bar, HOLDER: holder, FONT: put(bytearray(16), 0, 'I', VT), VT: vtable({1: 0, 7: 1, 9: 1}), 0x7cc61c: struct.pack('<I', DIALOG), 0x7cc620: bytes([r.choice([0, 1, r.getrandbits(8)])]),
               0xf7d88c: struct.pack('<I', r.choice([HOLDER, HOLDER, HOLDER, 0])), 0x758870: r.bytes(4), 0xfaa5c0: struct.pack('<h', scale), 0xfaa5c4: struct.pack('<h', r.choice([640, 800, 1024, 0, r.randrange(-300, 0x7fff)]))}
    return Call(0x4bb460, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

# TextPanel7cc18c_Show (src/game/ui/textpanel7cc18c, 0x4a84c0) against the original. Every call a recorder (their own benches have them; driver.cpp's kTextPanel7cc18cEnv): Window_ImplSlot7 thiscall 10 (the two out pointers logged as 0;
# width and height filled), Panel_Reset thiscall 11, GuiText_DrawRgb 12 (nine arguments; a tenth pushed is never read), Panel_ShrinkInto thiscall 13, Window_ImplSlot6 thiscall 14. No image, no panel, the modes 2 and 3 and others.
# usage: textpanel7cc18c.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, set_return, strings  # noqa: E402

B = Bench([(0x4a84c0, 0x4a85f5), (0x5fa680, 0x5fa6c0), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b)],
          hooks={0x65d6db: _recorder(10, 2), 0x5d48d0: _recorder(11, 0), 0x492c40: _recorder(12, 9, cdecl=True), 0x5d4970: _recorder(13, 4), 0x65d745: _recorder(14, 0)})
WINDOW = RAM + 0x100
PANEL = RAM + 0x400


def case(r):
    log = log_region()
    strings(log, 10, 0, 0b11)
    fill(log, 10, 0, struct.pack('<i', r.choice([0, 1, 256, 640, -1, r.randrange(-1 << 31, 1 << 31)])), which=0)
    fill(log, 10, 1, struct.pack('<i', r.choice([0, 1, 256, 480, -2, r.randrange(-1 << 31, 1 << 31)])), which=1)
    set_return(log, 10, r.choice([0, RAM + 0x800, RAM + 0x800, RAM + 0x800, RAM + 0x800, r.getrandbits(32) | 1]))
    return Call(0x4a84c0, [WINDOW], {LOG: log, PANEL: r.bytes(0x100), 0x7cc18c: struct.pack('<I', r.choice([PANEL, PANEL, PANEL, PANEL, 0])),
                                     0x8bd554: struct.pack('<I', r.choice([2, 2, 2, 3, 3, 3, 0, 1, 4, 0x102, r.getrandbits(32)]))}, ret='v')


if __name__ == '__main__':
    B.main(case)

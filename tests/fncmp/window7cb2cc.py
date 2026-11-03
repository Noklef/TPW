# The window [0x7cb2cc]'s handler (src/game/ui/window7cb2cc: Window7cb2cc_Handler, 0x4a2680) against the original. Recorders (driver.cpp's kWindow7cb2ccEnv): KeyMap_FindKey 10 (words), 0x409350 11, Window_GetRect 12 (it fills the rectangle), Window_SetWordsE4 13 (words: the upper halves the registers'), Window_RunScript 14, Dialog_BaseHandler 15. GuiRect's run.
# usage: window7cb2cc.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, fill, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x4a2680, 0x4a2783), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b)],
          hooks={0x40c870: _recorder(10, 2), 0x409350: _recorder(11, 1), 0x65d42e: _recorder(12, 2), 0x65f345: _recorder(13, 2), 0x65e72d: _recorder(14, 2), 0x47ee90: _recorder(15, 4, cdecl=True)})
MAP = RAM


def case(r):
    log = log_region()
    for k, bits in ((10, 3), (13, 3)):
        struct.pack_into('<I', log, LOG_BYTES + 4 * k, bits << 16)
    set_return(log, 10, r.choice([0, 0x10000, 1, 0xffff, r.getrandbits(32)]))
    set_return(log, 15, r.getrandbits(32))
    strings(log, 12, 0, out=1)
    fill(log, 12, 0, struct.pack('<4h', *[r.choice([0, 1, -1, 100, -101, 0x7fff, -0x8000, r.randrange(-0x8000, 0x8000)]) for _ in range(4)]), offset=4)
    message = r.choice([0x14, 0x15, 0x15, 0x1000b, 0x1000b, 0x1000b, 0x13, 0x16, 0x1000a, 0x1000c, r.getrandbits(32)])
    m = bytearray(r.bytes(8))
    put(m, 6, 'H', r.choice([0, 0, 0, 1, r.getrandbits(16)]))
    regions = {LOG: log, MAP: m, 0x78718c: struct.pack('<I', MAP), 0x7cb2cc: r.bytes(4), 0x7a1a8c: r.bytes(4)}
    return Call(0x4a2680, [r.getrandbits(32), message, r.getrandbits(32), r.choice([0, 0x10000, 0xff0000, 0x800000, r.getrandbits(32), r.getrandbits(32) & 0xff00ffff])], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

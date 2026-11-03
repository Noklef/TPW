# Dialog7cc4b4_ShowTheme (src/game/ui/dialog7cc4b4update, 0x4b90f0) against the original. The world and recorders of dialog7cc4b4update.py (Dialog7cc4b4UpdateEnv), with the child 0x1e0ef, the stored count [0x7cc4b8] about the theme's field, and 0x5b12e0 recorder 24.
# usage: dialog7cc4b4theme.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7cc4b4update as base  # noqa: E402
from fncmp import RAM, Bench, _cdecl_log, _recorder, put  # noqa: E402

base.TOP_IDS = (0x1e0ea, 0x1e0ec, 0x1e0ef, 0x1e0f0, 0x1e0f1)
B = Bench([(0x4b90f0, 0x4b933e), (0x65d7bc, 0x65d840), (0x5f60b0, 0x5f6160), (0x5f76c0, 0x5f7720), (0x5f61e0, 0x5f6210), (0x550ca0, 0x550d80), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x66bff7, 0x66c01b), (0x5da3c0, 0x5da3c1)],
          hooks={0x67a7d0: _cdecl_log, 0x5aef50: _recorder(12, 0), 0x5b09c0: _recorder(13, 1), 0x5b11a0: _recorder(16, 0), 0x65d9dd: _recorder(17, 1), 0x65d3a3: _recorder(18, 1), 0x65d42e: _recorder(23, 2), 0x521e60: _recorder(19, 4, cdecl=True),
                 0x51feb0: _recorder(20, 1, cdecl=True), 0x5b12e0: _recorder(24, 2)})


def case(r):
    c = base.case(r)
    c.address = 0x4b90f0
    regions = c.regions
    regions[base.GAMEC] = put(bytearray(r.bytes(0x60)), 0x30, 'I', base.NAME)
    regions[base.GAME4] = put(bytearray(r.bytes(0x20)), 0xc, 'I', base.GAMEC)
    regions[base.GAME] = put(bytearray(r.bytes(0x20)), 4, 'I', base.GAME4)
    regions[0xf85614] = struct.pack('<I', base.GAME)
    dialog = struct.unpack_from('<I', regions[0x7cc4b4])[0]
    regions[0x7cc4b4] = struct.pack('<Ii', dialog, r.choice([0, 1, 2, 3, 7, 8, 9, -1, r.randrange(-1 << 31, 1 << 31)]))
    return c


if __name__ == '__main__':
    B.main(case)

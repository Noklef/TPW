# The game hidden behind a menu and brought back (src/game/ui/gamepause: 0x4a9180, 0x4a9280, 0x4a9380) against the original. Every callee a recorder as driver.cpp's kGamePauseEnv, the heap's
# too, the temp path's import in the table. 0x4a9180's 0x4a9280 real in both.
# usage: gamepause.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return, strings  # noqa: E402

TEMP, FREE, ALLOC = 5, 6, 7
PANELRESET, PLAY, MODE, ATEXIT, LEAVE, CHOOSE, SHOWN, SLOT3, FINISH, F5996, ENTER, SLOT2, CAMERA, STATE, TOGGLE, SELECT, OPEN, BFREE, BCTOR, F5D47, F5644, SHRINK, STORE = 8, *range(10, 32)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4a9180, 0x4a9276), (0x4a9280, 0x4a9341), (0x4a9380, 0x4a941f)],
          hooks={0x485aa0: C(PLAY, 1), 0x550ca0: _recorder(MODE, 0), 0x67a7d0: C(ATEXIT, 1), 0x4092a0: _recorder(LEAVE, 2), 0x4bcec0: C(CHOOSE, 0), 0x65d9dd: _recorder(SHOWN, 1), 0x6591e3: C(SLOT3, 0),
                 0x5eda70: C(FINISH, 0), 0x5996d0: C(F5996, 1), 0x409300: _recorder(ENTER, 0), 0x659201: C(SLOT2, 0), 0x48aff0: C(CAMERA, 0), 0x4a2a90: C(STATE, 0), 0x4a29d0: C(TOGGLE, 0),
                 0x4bcf50: C(SELECT, 0), 0x4a8c00: C(OPEN, 0), 0x67acc0: C(ALLOC, 1), 0x67acb0: C(FREE, 1), 0x5d46d0: _recorder(BFREE, 0), 0x5d45b0: _recorder(BCTOR, 0), 0x5d47a0: _recorder(F5D47, 0),
                 0x564430: _recorder(F5644, 1), 0x5d48a0: _recorder(SHRINK, 1), 0x5644c0: _recorder(STORE, 0), 0x5d48d0: _recorder(PANELRESET, 0)})
OLD, BLOCK, MADE, DRAW, ROOT = RAM + 0x1000, RAM + 0x1100, RAM + 0x1200, RAM + 0x1300, RAM + 0x1400


def case(r):
    log = log_region()
    table = bytearray(0x400)
    put(table, 0x6fd14c - IMPORT_TABLE, 'I', recorder(TEMP, 2, com=True))
    regions = {LOG: log, IMPORT_TABLE: table, OLD: r.bytes(0x10), BLOCK: r.bytes(0x10), MADE: r.bytes(0x10), DRAW: r.bytes(0x10), ROOT: r.bytes(0x10)}
    regions[0xfb34bf] = bytes([r.getrandbits(8)])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([1, 1, 0, 2, r.getrandbits(32)]))
    regions[0x786ba4] = r.bytes(4)
    regions[0x7cc1a0] = r.bytes(4)
    regions[0x7cc190] = r.bytes(4)
    regions[0x7cc150] = r.bytes(4)
    regions[0x7cc194] = bytes([r.choice([0, 0, 1, r.getrandbits(8)])])
    regions[0x7cc18c] = struct.pack('<I', r.choice([OLD, OLD, 0]))
    regions[0x879a24] = struct.pack('<I', DRAW)
    regions[0xfaa5d8] = struct.pack('<I', ROOT)
    set_return(log, ALLOC, r.choice([BLOCK, BLOCK, BLOCK, 0]))
    set_return(log, BCTOR, MADE)
    set_return(log, STATE, r.choice([0, 1, 0x100, r.getrandbits(32)]))
    strings(log, F5644, 0, out=1)
    strings(log, SHRINK, 0, out=1)
    return Call(r.choice([0x4a9180, 0x4a9280, 0x4a9380]), [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

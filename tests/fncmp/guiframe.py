# The GUI's work for one frame (src/game/ui/guiframe: 0x48a580) against the original. Every callee a recorder as driver.cpp's kGuiFrameEnv; the cursor's slot 6 a recorder.
# usage: guiframe.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

SLOT = 6
BEGIN, FIRE, COUNT, STEP, DONE, PUMP, WTIMERS, POST, AFTER, ACTIVE, CLOCK, ATEXIT, MOUSE, MOVE, UPDATE, PUMP2, IME = range(10, 27)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x48a580, 0x48a6d0)],
          hooks={0x47f620: C(BEGIN, 0), 0x6622d2: C(FIRE, 0), 0x669c30: _recorder(COUNT, 0), 0x4a9350: C(STEP, 0), 0x4a9180: C(DONE, 0), 0x658e42: C(PUMP, 1), 0x658f4b: C(WTIMERS, 0),
                 0x658ccc: C(POST, 4), 0x658f6c: C(AFTER, 0), 0x47eaa0: C(ACTIVE, 0), 0x65968e: C(CLOCK, 0), 0x67a7d0: C(ATEXIT, 1), 0x5f5640: C(MOUSE, 0), 0x6588ef: C(MOVE, 2),
                 0x5f5590: C(UPDATE, 4), 0x658f0b: C(PUMP2, 1), 0x481100: _recorder(IME, 0)})
CURSOR, KVTABLE, POINT = RAM + 0x1000, RAM + 0x1100, RAM + 0x1200


def case(r):
    log = log_region()
    regions = {LOG: log, KVTABLE: vtable({SLOT: 1})}
    regions[CURSOR] = struct.pack('<I', KVTABLE)
    block = bytearray(r.bytes(0x50))
    put(block, 0, 'h', r.choice([480, 600, 768, 1, -2, r.randrange(1, 0x7fff)]))
    put(block, 4, 'h', r.choice([640, 800, 1024, 1, -2, r.randrange(1, 0x7fff)]))
    put(block, 0x48, 'I', r.choice([CURSOR, CURSOR, 0]))
    regions[0xfaa5c0] = block
    old = (r.getrandbits(32), r.getrandbits(32))
    new = r.choice([old, old, (old[0], r.getrandbits(32)), (r.getrandbits(32), old[1]), (r.randrange(0, 2048), r.randrange(0, 1536)), (r.getrandbits(32), r.getrandbits(32))])
    regions[0x7b9d00] = struct.pack('<II', *old)
    regions[0x7c20e4] = bytes([r.getrandbits(8)])
    regions[0x7b9a4c] = r.bytes(4)
    regions[0x7b9bac] = r.bytes(4)
    regions[POINT] = struct.pack('<II', *new)
    set_return(log, COUNT, r.getrandbits(32))
    set_return(log, STEP, r.choice([0, 1, 0x100, r.getrandbits(32)]))
    set_return(log, ACTIVE, r.choice([0, 1, 1, 1, 0x100, r.getrandbits(32)]))
    set_return(log, CLOCK, r.getrandbits(32))
    set_return(log, MOUSE, POINT)
    return Call(0x48a580, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

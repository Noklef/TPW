# A surface's lock rate (src/game/gfx/texsurface: TexSurface_LockRate 0x58b470) against the original: sizes small, large and past the sign, the clock's two readings close, equal (a division by zero), backwards or far apart, the virtual base at several offsets.
# usage: texsurfacerate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, vtable  # noqa: E402

CLOCK = 8
FIRST, SECOND = 4, 5   # (the slots at +0x10 and +0x14)
B = Bench([(0x58b470, 0x58b4f0)], hooks={0x57cb90: _recorder(CLOCK, 0)})
SUB, OFFSETS, TABLE, READINGS = RAM + 0x100, RAM + 0x200, RAM + 0x280, RAM + 0x300


def size(r):
    return r.choice([0, 1, 2, 16, 64, 256, 640, 1024, 0x10000, r.randrange(1, 0x1000), r.getrandbits(32)])


def case(r):
    log = log_region()
    answers(log, CLOCK, READINGS)
    offset = r.choice([0x10, 0x40, 0x100])
    start = r.getrandbits(32)
    spent = r.choice([0, 1, 2, 100, r.randrange(1, 0x10000), r.randrange(1, 0x10000000), 0x10000000, 0xf0000000, r.getrandbits(32), 0xffffffff])
    base = struct.pack('<IIII', TABLE, r.getrandbits(32), size(r), size(r))
    regions = {LOG: log, OFFSETS: struct.pack('<II', r.getrandbits(32), offset), TABLE: vtable({FIRST: 0, SECOND: 0}), READINGS: struct.pack('<2I', start, (start + spent) & 0xffffffff) + bytes(38 * 4),
               SUB - 0x3c: struct.pack('<I', OFFSETS) + bytes(offset - 4) + base}
    return Call(0x58b470, [SUB], regions, conv='t', ret='d')


if __name__ == '__main__':
    B.main(case)

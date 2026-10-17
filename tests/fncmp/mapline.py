# A straight run of map cells (src/game/world/mapline) against the original: Map_ApplyLine (0x53aac0) with its cell operation 0x5370e0 a recorder (slot 20,
# three arguments: the emulator's hooks=, the driver's MapLine env), which logs the cell pointer (the world pointer at 0x7cf83c is never dereferenced: a
# random value) and the (arg, x step, y step) it gets, and answers a hash or, now and then, 0 (the run stops). Starts and ends are near the map (-8..135),
# sometimes the same, sometimes off by one or on a tie; the flags at 0x820ac8 / 0x820ad0 start random.
# usage: mapline.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

K = 20  # the recorder slot (driver.cpp's MapLine env)
B = Bench([(0x53aac0, 0x53ad1c), (0x4d8300, 0x4d8340)], hooks={0x5370e0: _recorder(K, 3)})
ENDS = RAM


def coord(r):
    return r.choice([r.randrange(-8, 136), r.randrange(0, 128), r.randrange(0, 128)])


def case(r):
    x, y = coord(r), coord(r)
    shape = r.choice(['any', 'any', 'h', 'v', 'tie', 'same', 'near'])
    if shape == 'h':
        ex, ey = coord(r), y + r.randrange(-3, 4)
    elif shape == 'v':
        ex, ey = x + r.randrange(-3, 4), coord(r)
    elif shape == 'tie':
        d = r.randrange(-40, 41)
        ex, ey = x + d, y + r.choice([d, -d])
    elif shape == 'same':
        ex, ey = x, y
    elif shape == 'near':
        ex, ey = x + r.randrange(-2, 3), y + r.randrange(-2, 3)
    else:
        ex, ey = coord(r), coord(r)
    log = log_region()
    if r.random() < 0.2:
        set_return(log, K, 0)
    world = r.getrandbits(32) | 0x80000000
    flags = struct.pack('<IIII', r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32))
    return Call(0x53aac0, [r.getrandbits(32), x & 0xffffffff, y & 0xffffffff, ENDS, ENDS + 4],
                {ENDS: bytearray(struct.pack('<ii', ex, ey)), LOG: log, 0x7cf83c: bytearray(struct.pack('<I', world)), 0x820ac8: bytearray(flags)},
                ret='a')


if __name__ == '__main__':
    B.main(case)

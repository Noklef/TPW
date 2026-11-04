# The advisor's comparison of two rings of the game's table (src/game/core/advisorscoring: 0x59eb80) against the original: the park's age a recorder's answer (FunnyTime_Elapsed: months either side of 3, negative too), the two rings (+0x1f7f0, +0x1fc90) wrapped or not, their cursor and size any small, the window +0x358 small (0 and negative too), the factor +0x354 any (NaN too), the values big enough to round in 24 bits. Reads past the rings' ends are mapped.
# usage: advisorringratio.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, echo, fill, log_region, put, strings  # noqa: E402

ELAPSED = 1
B = Bench([(0x59eb80, 0x59edc4), (0x519510, 0x519550), (0x5da3c0, 0x5da3c1), (0x67c320, 0x67c3e0)], hooks={0x4f8800: _recorder(ELAPSED, 1)})
SELF, POINTER, GAME1, GAME2, TABLE = RAM, RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0x1000
RINGS = 0x1f7f0, 0x1fc90
MONTH = 0x1792f8648000


def ring(r):
    data = bytearray(r.bytes(0x250))
    values = r.choice([lambda: r.randrange(-100, 100), lambda: r.getrandbits(32), lambda: r.getrandbits(26)])
    for i in range(0x90):
        put(data, 4 + 4 * i, 'I', values() & 0xffffffff)
    put(data, 0x244, '2i', r.randrange(0, 0x40), r.choice([0, 1, 4, 0x20, 0x40, r.randrange(0x40)]))
    data[0x24c] = r.choice([0, 0, 1, 0x80])
    return data


def case(r):
    log = log_region()
    self = bytearray(r.bytes(0x400))
    put(self, 0x358, 'i', r.choice([0, -1, 1, 2, 4, 8, 0x20, r.randrange(0x40)]))
    r.floats(self, [0x354], r.choice([1.0, 2.0, 0.5]))
    months = r.choice([3, 3, 3, 3, 4, 2, 0, -1, 100])
    age = (months * MONTH + r.choice([0, 1, MONTH - 1, -1])) & 0xffffffffffffffff
    strings(log, ELAPSED, 0, 1)
    echo(log, ELAPSED, 0)
    fill(log, ELAPSED, 0, struct.pack('<Q', age))
    regions = {LOG: log, SELF: self, 0x7cf6ec: struct.pack('<I', POINTER), POINTER: struct.pack('<I', GAME1), 0x7cf83c: struct.pack('<I', GAME2), GAME1: r.bytes(0x10), GAME2 + 0x1da720: bytes(4),
               0x7cfb90: struct.pack('<I', TABLE), TABLE + RINGS[0]: ring(r), TABLE + RINGS[1]: ring(r)}
    return Call(0x59eb80, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

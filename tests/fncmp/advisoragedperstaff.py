# The advisor's count per staff in an old enough park (src/game/core/advisorscoring: 0x59f9c0) against the original: the park's age and both counts recorders' answers, about the threshold, big enough to wrap, the table getter real.
# usage: advisoragedperstaff.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, echo, fill, log_region, put, set_return, strings  # noqa: E402

ELAPSED, COUNT, STAFF = 1, 27, 28
B = Bench([(0x59f9c0, 0x59fa52), (0x519510, 0x519550), (0x67c320, 0x67c3e0), (0x5da3c0, 0x5da3c1)], hooks={0x4f8800: _recorder(ELAPSED, 1), 0x4c9aa0: _recorder(COUNT, 0), 0x4c9a10: _recorder(STAFF, 0)})
SELF, POINTER, GAME, TABLE = RAM, RAM + 0x800, RAM + 0x900, RAM + 0xa00
MONTH = 0x1792f8648000


def case(r):
    log = log_region()
    self = bytearray(r.bytes(0x500))
    months = r.choice([-1, 0, 2, 3, 3, 3, 4, 40])
    age = (months * MONTH + r.choice([0, 1, MONTH - 1] if months >= 0 else [0, -1, 1 - MONTH])) & 0xffffffffffffffff
    strings(log, ELAPSED, 0, 1)
    echo(log, ELAPSED, 0)
    fill(log, ELAPSED, 0, struct.pack('<Q', age))
    count = r.choice([r.randrange(0, 200), r.getrandbits(32)])
    staff = r.choice([0, 1, r.randrange(1, 50), r.randrange(1, 50), r.getrandbits(32) | 1])
    set_return(log, COUNT, count)
    set_return(log, STAFF, staff)
    wrapped = (count * 100 + (1 << 31)) % (1 << 32) - (1 << 31)
    signed = (staff + (1 << 31)) % (1 << 32) - (1 << 31)
    ratio = abs(wrapped) // abs(signed or 1) * (1 if (wrapped < 0) == (signed < 0) else -1)
    put(self, 0x4b8, 'i', r.choice([ratio, ratio - 1, ratio + 1, r.randrange(-100, 300)]))
    regions = {LOG: log, SELF: self, 0x7cf6ec: struct.pack('<I', POINTER), POINTER: struct.pack('<I', GAME), GAME: r.bytes(0x10), GAME + 0x1da720: r.bytes(4), 0x7cfb90: struct.pack('<I', TABLE)}
    return Call(0x59f9c0, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

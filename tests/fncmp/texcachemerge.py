# The texture cache's item merge (src/game/gfx/texcache: 0x58eae0) against the original: two lists
# of up to 5 items each (positions from a few values so that ties occur; flag 0x40000000 and the
# driver filter, which depends on 0x8bd524, on or off), the owner's list empty or not.
# usage: texcachemerge.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x58eae0, 0x58ec95), (0x590fb0, 0x590fcb)])
A_AT, B_AT, OUT, OWNER = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000


def items(r, base, n):
    buf = bytearray(r.bytes(0x40 * 6))
    for i in range(n):
        o = 0x40 * i
        put(buf, o + 4, 'I', base + 0x40 * (i + 1) if i + 1 < n else 0)
        flags = r.getrandbits(32) & ~0x40000030
        flags |= r.choice([0, 0x40000000, 0x40000000]) | r.choice([0, 0x30, 0x10, 0x20])
        put(buf, o + 8, 'I', flags)
        put(buf, o + 0x10, 'hh', r.choice([-2, 0, 1, 5, 5, r.randrange(-300, 300)]), r.choice([-1, 0, 3, 3, r.randrange(-300, 300)]))
        put(buf, o + 0x14, 'hh', r.choice([0, 1, 16, 64, r.randrange(-300, 300)]), r.choice([0, 1, 16, 64, r.randrange(-300, 300)]))
    return buf


def case(r):
    na, nb = r.choice([0, 1, 2, 3, 5]), r.choice([0, 1, 2, 3, 5])
    owner = bytearray(r.bytes(0x10))
    put(owner, 4, 'I', A_AT if na else 0)
    regions = {A_AT: items(r, A_AT, na), B_AT: items(r, B_AT, nb), OUT: bytearray(0x100), OWNER: owner,
               0x8bd520: r.bytes(4) + bytes([r.choice([0, 1, 4, 0xff])]) + r.bytes(3)}
    return Call(0x58eae0, [OUT, B_AT if nb else 0, OWNER], regions, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)

# The challenges' orphan drop, current challenge and schema slot (src/game/world/challenges:
# 0x4d1c80, 0x4d1e60, 0x4d2840) against the originals: ids and prerequisites drawn from a few
# values (so that they match, chains drop in order, and some name themselves); current indexes
# about 0 and 20; and the guard's state checks (world/staff: 0x4d6330, 0x4d6350).
# usage: challenges.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4d1c80, 0x4d1cdc), (0x4d1e60, 0x4d1e74), (0x4d2840, 0x4d285d), (0x4d6330, 0x4d6348),
           (0x4d6350, 0x4d635f)])
C = RAM + 0x100


def case(r):
    mem = r.bytes(0x600)
    op = r.randrange(5)
    if op == 0:
        k = r.choice([3, 6, 25, 1 << 32])
        for i in range(0x14):
            put(mem, 0x100 + 0xc + i * 0x30, 'I', r.choice([0, r.randrange(k)]) if k < 1 << 32 else r.getrandbits(32))
            put(mem, 0x100 + 0xc + i * 0x30 + 0x18, 'I', r.choice([0, r.randrange(min(k, 30))]))
        return Call(0x4d1c80, [C], {RAM: mem}, conv='t')
    if op == 1:
        put(mem, 0x100 + 0x3cc, 'i', r.choice([-1, 0, 1, 19, 20, 21, r.randrange(-3, 23), r.getrandbits(32) - (1 << 31)]))
        return Call(0x4d2840, [C], {RAM: mem}, conv='t', ret='a')
    if op == 2:
        return Call(0x4d1e60, [C, r.choice([0, 1, r.randrange(30), r.getrandbits(32)])], {RAM: mem}, conv='t', ret='a')
    put(mem, 0x100 + 0x19c, 'I', r.choice([0, 1, 2, 0x12, 0x13, r.randrange(30), r.getrandbits(32), 0x100, 0x112]))
    return Call(r.choice([0x4d6330, 0x4d6350]), [C], {RAM: mem}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

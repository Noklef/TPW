# The IME support's constructor and its u32 vectors' copy and fill (src/game/core/ime: 0x478b60,
# 0x47c910, 0x47c940) against the originals: the constructor on random bytes at addresses with
# different top bytes (the allocator bytes copy it, sic); the helpers overlapping, to null, empty.
# usage: ime.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x478b60, 0x478bc0), (0x47c910, 0x47c93e), (0x47c940, 0x47c963)])


def case(r):
    op = r.randrange(3)
    if op == 0:
        at = r.choice([RAM, RAM + 0x1000 * r.randrange(16) + 4 * r.randrange(16), 0x7a0000 + 0x100 * r.randrange(8)])
        return Call(0x478b60, [at], {at: r.bytes(0x70)}, conv='t', ret='a')
    n = r.choice([0, 1, 2, r.randrange(3, 20)])
    src = RAM + 0x100 + 4 * r.randrange(0x10)
    dst = r.choice([src + 4 * r.randrange(-12, 13), RAM + 0x800, 0])
    if not dst:  # past a null first slot it would write at 4 on
        n = min(n, 1)
    if op == 1:
        return Call(0x47c910, [r.getrandbits(32), src, src + 4 * n, dst], {RAM: r.bytes(0x1000)}, conv='t', ret='a')
    value = r.choice([src, dst + 4 * r.randrange(n + 1) if dst else src])
    return Call(0x47c940, [r.getrandbits(32), dst, n, value], {RAM: r.bytes(0x1000)}, conv='t')


if __name__ == '__main__':
    B.main(case)

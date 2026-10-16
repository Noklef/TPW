# The IME support (src/game/core/ime: 0x478b60, 0x47a960, 0x47a980, 0x47ae40, 0x47b300, 0x47c700,
# 0x47c720, 0x47c910, 0x47c940, 0x47db00, and the host 0x480e70) and the wide string's set length (core/wtraits: 0x47ba30)
# against the originals: the constructor on random bytes at addresses with different top bytes
# (the allocator bytes copy it, sic); the vector helpers overlapping, to null, empty; the sizes
# of vectors null, empty, inverted.
# usage: ime.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x478b60, 0x478bc0), (0x47c910, 0x47c93e), (0x47c940, 0x47c963), (0x47db00, 0x47db11),
           (0x47c720, 0x47c733), (0x47c700, 0x47c713), (0x47ae40, 0x47ae53), (0x47b300, 0x47b313),
           (0x47a960, 0x47a975), (0x47a980, 0x47a99f), (0x47ba30, 0x47ba43), (0x480e70, 0x480eb8)])
SIZES = [0x47c720, 0x47c700, 0x47ae40, 0x47b300]


def case(r):
    op = r.choice(OPS)
    mem = r.bytes(0x1000)
    if op == 'Ctor':
        at = r.choice([RAM, RAM + 0x1000 * r.randrange(16) + 4 * r.randrange(16), 0x7a0000 + 0x100 * r.randrange(8)])
        return Call(0x478b60, [at], {at: r.bytes(0x70)}, conv='t', ret='a')
    if op == 'Host':
        at = r.choice([RAM, RAM + 0x1000 * r.randrange(16) + 4 * r.randrange(16), 0x7a0000 + 0x100 * r.randrange(8)])
        return Call(0x480e70, [at], {at: r.bytes(0x1a0)}, conv='t', ret='a')
    if op == 'Size':
        first = r.choice([0, RAM + 0x400])
        put(mem, 0x104, '3I', first, *((first + r.choice([0, 4, 16, 0x40, r.randrange(-0x40, 0x40)])) & 0xffffffff for _ in range(2)))
        return Call(r.choice(SIZES), [RAM + 0x100], {RAM: mem}, conv='t', ret='a')
    if op == 'Current':
        put(mem, 0x154, 'i', r.choice([-1, -1, -2, 0, 1, r.randrange(20), r.getrandbits(31), -r.getrandbits(31)]))
        return Call(0x47a960, [RAM + 0x100], {RAM: mem}, conv='t', ret='a')
    if op == 'Limit':
        return Call(0x47a980, [RAM + 0x100, r.choice([0, 11, 12, 13, r.getrandbits(32)])], {RAM: mem}, conv='t')
    if op == 'Construct':
        return Call(0x47db00, [r.choice([0, RAM + 0x100, RAM + 0x200]), RAM + r.choice([0x100, 0x200, 0x300])], {RAM: mem})
    if op == 'SetLength':
        n = r.choice([0, 1, r.randrange(0x100)])
        put(mem, 0x104, 'I', RAM + 0x200 + r.randrange(0x100))
        return Call(0x47ba30, [RAM + 0x100, n], {RAM: mem}, conv='t')
    n = r.choice([0, 1, 2, r.randrange(3, 20)])
    src = RAM + 0x100 + 4 * r.randrange(0x10)
    dst = r.choice([src + 4 * r.randrange(-12, 13), RAM + 0x800, 0])
    if not dst:  # past a null first slot it would write at 4 on
        n = min(n, 1)
    if op == 'Copy':
        return Call(0x47c910, [r.getrandbits(32), src, src + 4 * n, dst], {RAM: mem}, conv='t', ret='a')
    value = r.choice([src, dst + 4 * r.randrange(n + 1) if dst else src])
    return Call(0x47c940, [r.getrandbits(32), dst, n, value], {RAM: mem}, conv='t')


OPS_ALL = ['Ctor', 'Copy', 'Fill', 'Size', 'Current', 'Limit', 'Construct', 'SetLength', 'Host']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

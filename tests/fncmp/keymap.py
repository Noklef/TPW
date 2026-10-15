# The keyboard maps (src/game/core/keymap: 0x40c870-0x40cfa0) against the original: lookups by
# key and by action over entries mostly at their own index (some moved, repeated or missing),
# counts negative, zero or up to 0x40, and the enable / reset of one map or all five (entries
# apart from the map or over it).
# usage: keymap.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x40c870, 0x40c8b3), (0x40c8c0, 0x40c8f7), (0x40ca20, 0x40ca93), (0x40cb40, 0x40cb47),
           (0x40cb50, 0x40cb78), (0x40cf60, 0x40cf98), (0x40cfa0, 0x40d06a)])
GLOBALS = (0x78718c, 0x787a60, 0x787c14, 0x78733c, 0x78721c)
CALLS = ('Key', 'Action', 'Held', 'Enable', 'Reset', 'EnableAll', 'ResetAll')


def keymap(r, at, regs):
    """A map at `at` (0x40 bytes, in a region from at - 0x40) with its entries 0x400 on, or over
    the map (an entry's held flag over its count, its enabled flag or its pointer)."""
    n = r.choice([0, 1, r.randrange(2, 0x40), r.randrange(-3, 0)])
    m = bytearray(r.bytes(0x40))
    entries = at + 0x400
    if r.random() < 0.1:
        entries = at + r.choice([-2, -0x16, 0, 2, 4, 6, 8])
    put(m, 0, 'Ihh', entries, n, r.choice([0, 1, r.getrandbits(15)]))
    e = bytearray(r.bytes(0x40 * 0x14))
    for i in range(0x40):
        action = r.choice([i] * 6 + [r.randrange(8), r.randrange(0x40), -1, r.getrandbits(15)])
        put(e, i * 0x14, 'hhhh', action, r.randrange(8), r.randrange(4), r.choice([0, 1, r.getrandbits(15)]))
    regs[at - 0x40] = r.bytes(0x40) + m
    regs[at + 0x400] = e
    return n


def case(r):
    op = r.choice(OPS)
    regs = {}
    if op.endswith('All'):
        for k, g in enumerate(GLOBALS):
            keymap(r, RAM + k * 0x2000 + 0x40, regs)
            regs[g] = (RAM + k * 0x2000 + 0x40).to_bytes(4, 'little')
        return Call(0x40cf60 if op == 'EnableAll' else 0x40cfa0, [], regs)
    M = RAM + 0x40
    n = keymap(r, M, regs)
    small = lambda: r.choice([r.randrange(-2, max(n, 1) + 2), r.randrange(8), r.getrandbits(16)])  # noqa: E731
    if op == 'Key':
        return Call(0x40c870, [M, r.randrange(8) if r.random() < 0.7 else small(), r.randrange(4)], regs, conv='t', ret='a')
    if op == 'Action':
        return Call(0x40c8c0, [M, small()], regs, conv='t', ret='a')
    if op == 'Held':
        return Call(0x40ca20, [M, small()], regs, conv='t', ret='w')
    return Call(0x40cb40 if op == 'Enable' else 0x40cb50, [M], regs, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

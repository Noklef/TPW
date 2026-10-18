# The circular buffers' constructors, push, look back and sum (src/game/core/circbuf: 0x4cdb60,
# 0x4ce9e0, 0x4ce290, 0x495cf0, 0x495c20, 0x495d40, 0x51ade0-0x51ae90) against the originals: cursors before, at and past the size,
# sizes of either sign, wrapped or not, looks back of every sign about the cursor and size; sums of counts about the
# size and cursor over values small (exact sums) and large (sums rounded to 24 bits).
# usage: circbuf.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4cdb60, 0x4cdb7a), (0x4ce9e0, 0x4ce9f8), (0x4ce290, 0x4ce2c9), (0x495cf0, 0x495d3e), (0x495c20, 0x495ce4), (0x495d40, 0x495d56), (0x5da3c0, 0x5da3d0), (0x51ade0, 0x51ae01),
           (0x51ae30, 0x51aeab)])
BUF = RAM + 0x100


def case(r):
    mem = r.bytes(0x600)
    op = r.randrange(6)
    if op == 0:
        return Call(r.choice([0x4cdb60, 0x4ce9e0, 0x51ade0, 0x51ae30, 0x51ae60, 0x51ae90]), [BUF + 4 * r.randrange(8)], {RAM: mem}, conv='t', ret='a')
    if op == 1:
        size = r.choice([0, 1, 2, 0x90, r.randrange(0x90), r.randrange(-3, 3)])
        put(mem, 0x100 + 0x248, 'i', size)
        put(mem, 0x100 + 0x244, 'i', r.choice([size - 1, size - 2, size, -1, -2, r.randrange(-2, 0x92)]))
        if r.random() < 0.5:
            put(mem, 0x100 + 0x24c, 'B', 0)
        return Call(0x4ce290, [BUF, r.getrandbits(32)], {RAM: mem}, conv='t')
    size = r.choice([0, 1, 30, r.randrange(31), r.randrange(-3, 3)])
    cursor = r.choice([-1, 0, size - 1, size, r.randrange(-3, max(size, 0) + 1)])
    put(mem, 0x100 + 0x80, 'i', size)
    put(mem, 0x100 + 0x7c, 'i', cursor)
    put(mem, 0x100 + 0x84, 'I', r.choice([0, 1, r.getrandbits(8)]) | r.choice([0, r.getrandbits(24) << 8]))
    back = r.choice([0, 1, -1, cursor, cursor + 1, size, size - 1, r.randrange(-4, max(size, 0) + 4),
                     r.getrandbits(32) - (1 << 31)])
    if op == 5:   # the count
        return Call(0x495d40, [BUF], {RAM: mem}, conv='t', ret='a')
    if op == 4:   # the sum: values small, large or of any size
        kind = r.randrange(3)
        for i in range(30):
            put(mem, 0x104 + 4 * i, 'I', [r.randrange(0x100), r.getrandbits(32) | 0x1000000, r.getrandbits(r.randrange(1, 33))][kind])
        count = r.choice([0, 1, 30, size, size + 1, cursor, cursor + 1, cursor + 2, r.randrange(-3, 40), 0x7fffffff, -0x80000000])
        return Call(0x495c20, [BUF, RAM + 0x500, count & 0xffffffff], {RAM: mem}, conv='t')
    if back < 0 and cursor - back > 30:
        back = -1
    return Call(0x495cf0, [BUF, RAM + 0x500, back & 0xffffffff], {RAM: mem}, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)

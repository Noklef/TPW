# The park analyser's small methods (src/game/world/analyser: 0x4c5d10, 0x4c74b0, 0x4c74f0,
# 0x4c7b70, 0x4c94a0-0x4c94d0, 0x4cdb60) against the originals: rings with cursors before, at
# and past their sizes, wrapped or not; looks back of every sign about the cursor and size;
# positions 0 and beyond the grid with scales of either sign.
# usage: analyser.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4c5d10, 0x4c5d70), (0x4c74b0, 0x4c74e9), (0x4c74f0, 0x4c751b), (0x4c7b70, 0x4c7bab),
           (0x4c94a0, 0x4c952b), (0x4cdb60, 0x4cdb7a)])
A = RAM  # the analyser; its +0x20800 on is the region at HI
HI = RAM + 0x20800


def case(r):
    lo, hi = r.bytes(0x100), r.bytes(0x1500)
    regions = {RAM: lo, HI: hi}
    op = r.randrange(7)
    if op == 0:
        return Call(r.choice([0x4c5d10, 0x4cdb60]), [RAM + 0x1000 + 4 * r.randrange(8)], {RAM + 0x1000: r.bytes(0x100)},
                    conv='t', ret='a')
    if op == 1:
        ring = r.randrange(1, 18)
        off = 0x216fc + ring * 0x40 - 0x20800
        size = r.choice([0, 1, 2, r.randrange(0x34), r.randrange(-3, 3)])
        put(hi, off + 0x38, 'i', size)
        put(hi, off + 0x34, 'i', r.choice([size - 1, size - 2, size, -1, -2, r.randrange(-2, 0x36)]))
        if r.random() < 0.5:
            put(hi, off + 0x3c, 'B', 0)
        return Call(0x4c74b0, [A, ring, r.getrandbits(32)], regions, conv='t')
    if op == 2:
        return Call(0x4c74f0, [A, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t')
    if op == 3:
        return Call(0x4c94a0, [A, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op == 4:
        return Call(0x4c94c0, [A], regions, conv='t', ret='a')
    if op == 5:
        size = r.choice([0, 1, 0x90, r.randrange(0x91), r.randrange(-3, 3)])
        cursor = r.choice([-1, 0, size - 1, size, r.randrange(-3, max(size, 0) + 1)])
        put(hi, 0x20a68 - 0x20800, 'i', size)
        put(hi, 0x20a64 - 0x20800, 'i', cursor)
        put(hi, 0x20a6c - 0x20800, 'B', r.choice([0, 1, r.getrandbits(8)]))
        back = r.choice([0, 1, -1, cursor, cursor + 1, size, size - 1, r.randrange(-4, max(size, 0) + 4),
                         r.getrandbits(32) - (1 << 31)])
        if back < 0 and cursor - back > 0x90:
            back = -1
        return Call(0x4c94d0, [A, back & 0xffffffff], regions, conv='t', ret='a')
    scale = r.choice([1, 2, 4, 8, -1, -3, r.randrange(1, 200), -r.randrange(1, 200)])
    put(lo, 0xc, 'iI', scale, r.choice([32, 16, r.getrandbits(32)]))
    pos = r.choice([0, 1, 0x80, 0x81, 0xffff, r.randrange(0x10000)]) | r.choice([0, r.getrandbits(16) << 16])
    return Call(0x4c7b70, [A, pos], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

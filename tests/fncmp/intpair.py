# The int pair's copy, scale, 16.16 divide and multiply, add, subtract, negation, length and constructor, and the
# integer square root (src/game/math/intpair: 0x4d99c0, 0x4d97b0, 0x50e080, 0x50df70, 0x50ece0, 0x50cd80,
# 0x50f1a0-0x50f1f0, 0x50f850, 0x5104b0, 0x5104e0, 0x4d49a0), the truncation to a length and the normalization
# (0x50f600, 0x50f700), the quarter turns (0x4d9cc0) and the atexit entry 0x50ce20 (a bare RET) against the
# originals: apart, the same, and overlapping by one int either way; values about 0 and INT_MIN,
# scales of any value, divisors whose quotients fit; for the last two values in each of the length's three tiers and limits about the length.
# usage: intpair.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x4d99c0, 0x4d99d3), (0x4d97b0, 0x4d97d9), (0x50e080, 0x50e0d4), (0x50ece0, 0x50ed0a), (0x50f1a0, 0x50f202), (0x50f850, 0x50f86f),
           (0x5104b0, 0x510502), (0x50cd80, 0x50cdaa), (0x50df70, 0x50dfb6), (0x4d49a0, 0x4d49c2),
           (0x50f600, 0x50f6f9), (0x50f700, 0x50f84a), (0x4d9cc0, 0x4d9d6b), (0x5da3c0, 0x5da3c1), (0x50ce20, 0x50ce21)])


def case(r):
    pair = RAM + 0x100
    other = r.choice([RAM + 0x200, pair, pair + 4, pair - 4])
    mem = bytearray(r.bytes(0x400))
    x, y = (r.choice([0, 1, -1, 0x7fffffff, -0x80000000, r.randrange(-300, 300), r.getrandbits(32) - (1 << 31)])
            for _ in range(2))
    struct.pack_into('<ii', mem, 0x100, x, y)
    op = r.randrange(16)
    if op == 15:   # the atexit entry 0x50ce20 (a bare RET): the pair 0x7cf100 any
        return Call(0x50ce20, [], {0x7cf100: r.bytes(8)}, ret='v')
    if op == 14:   # angles: the quarter turns and their negatives, about them, multiples of 360 past them, any
        angle = r.choice([0, 90, 180, 270, -90, -180, -270, 360, -360, 450, -450, 1, 89, 91, -1, 359, r.randrange(-8, 8) * 90 + r.choice([0, 0, 0, 360 * r.randrange(-50, 50)]),
                          r.getrandbits(32) - (1 << 31), -0x80000000, 0x7fffffff])
        return Call(0x4d9cc0, [pair, other, angle & 0xffffffff], {RAM: mem}, conv='t', ret='a')
    if op in (12, 13):  # magnitudes in each tier: under 0x1000, under 0x400000, beyond
        x, y = (r.choice([1, -1]) * r.choice([0, 1, r.randrange(0x1000), r.randrange(0x400000), r.randrange(0x80000000), 0xfff, 0x1000, 0x3fffff, 0x400000])
                for _ in range(2))
        if r.random() < 0.1:
            x, y = r.choice([(-0x80000000, r.randrange(-5, 5)), (r.randrange(-0x1000, 0x1000), -0x80000000), (0, 0)])
        struct.pack_into('<ii', mem, 0x100, x, y)
        if op == 13:
            return Call(0x50f700, [other, pair], {RAM: mem}, ret='a')
        length = int(math.hypot(x, y))
        limit = r.choice([length, length + 1, length - 1, length // 2, length * 2, 0, 1, -1, r.randrange(1, 0x100000), r.getrandbits(31), -r.getrandbits(31)])
        if x == y == 0:  # a length of 0 at a limit of 0: the division by 0 (it faults, sic)
            limit = r.choice([0, 0, 1, -1])
        return Call(0x50f600, [other, pair, limit & 0xffffffff], {RAM: mem}, ret='a')
    if op == 9:
        return Call(0x50cd80, [pair, other, r.choice([RAM + 0x200, pair, pair + 4, other])], {RAM: mem}, conv='t', ret='a')
    if op == 10:
        scale = r.choice([0, 1, 0x10000, -0x10000, 0x8000, 0x18000, r.randrange(-0x20000, 0x20000), r.getrandbits(32) - (1 << 31)])
        return Call(0x50df70, [pair, other, scale & 0xffffffff], {RAM: mem}, conv='t', ret='a')
    if op == 11:
        k = r.randrange(0x10000)
        n = r.choice([0, 1, 2, 3, 0xffffffff, 0xfffe0001, k * k, k * k - 1, k * k + 2 * k, r.getrandbits(32), r.getrandbits(r.randrange(1, 33))])
        return Call(0x4d49a0, [n & 0xffffffff], {RAM: mem}, ret='a')
    if op == 7:
        return Call(0x50f850, [other, pair], {RAM: mem}, ret='a')
    if op == 8:
        return Call(r.choice([0x5104b0, 0x5104e0]), [other, r.getrandbits(32), r.getrandbits(32)], {RAM: mem}, ret='a')
    if op == 2:  # a divisor whose quotients fit (the original faults otherwise)
        while True:
            d = r.choice([1, -1, 2, 3, 0x10000, -0x10000, r.randrange(-1000, 1000), r.getrandbits(32) - (1 << 31)])
            if d and all(abs(v << 16) // abs(d) < (1 << 31) for v in (x, y)):
                break
        return Call(0x50e080, [pair, other, d & 0xffffffff], {RAM: mem}, conv='t', ret='a')
    if op == 3:
        return Call(0x50ece0, [pair, other, r.choice([RAM + 0x200, pair, pair + 4, other])], {RAM: mem}, conv='t', ret='a')
    if op == 4:
        return Call(0x50f1a0, [pair, other], {RAM: mem}, conv='t', ret='a')
    if op == 5:
        return Call(0x50f1c0, [pair], {RAM: mem}, conv='t', ret='a')
    if op == 6:
        return Call(0x50f1f0, [pair], {RAM: mem}, conv='t', ret='a')
    if r.random() < 0.5:
        return Call(0x4d99c0, [pair, other], {RAM: mem}, conv='t', ret='a')
    scale = r.choice([0, 1, 2, -1 & 0xffffffff, r.randrange(100), r.getrandbits(32)])
    return Call(0x4d97b0, [pair, other, scale], {RAM: mem}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

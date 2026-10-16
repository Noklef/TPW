# The int pair's copy, scale, 16.16 divide, add, negation, length and constructor
# (src/game/math/intpair: 0x4d99c0, 0x4d97b0, 0x50e080, 0x50ece0, 0x50f1a0-0x50f1f0) against the
# originals: apart, the same, and overlapping by one int either way; values about 0 and INT_MIN,
# scales of any value, divisors whose quotients fit.
# usage: intpair.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x4d99c0, 0x4d99d3), (0x4d97b0, 0x4d97d9), (0x50e080, 0x50e0d4), (0x50ece0, 0x50ed0a), (0x50f1a0, 0x50f202)])


def case(r):
    pair = RAM + 0x100
    other = r.choice([RAM + 0x200, pair, pair + 4, pair - 4])
    mem = bytearray(r.bytes(0x400))
    x, y = (r.choice([0, 1, -1, 0x7fffffff, -0x80000000, r.randrange(-300, 300), r.getrandbits(32) - (1 << 31)])
            for _ in range(2))
    struct.pack_into('<ii', mem, 0x100, x, y)
    op = r.randrange(7)
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

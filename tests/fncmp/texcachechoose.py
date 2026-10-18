# The texture cache's choice between two sizes (src/game/gfx/texcache: 0x58ea20) against the original: sizes 0-5 (a few the same),
# their counts 0-300 (a few 0: infinite and NaN shares, or negative), totals and used near each other (b's total 10 below a's give or
# take); the first entry's surface's slot 3 (a recorder) answering around limit * 4 / 3 (and past 2^31 once times 3). Some cases
# with 0 / 0 shares (NaN, unordered: a's among the next), and some with b's share of two 25-31 bit numbers and a's exactly twice its rounding (the
# x87's single rounding at 24 bits against the operands rounded first).
# usage: texcachechoose.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys
from fractions import Fraction

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x58ea20, 0x58ead5)])
CACHE, ENTRY, SURF, VT = RAM, RAM + 0x2000, RAM + 0x2100, RAM + 0x2200


def count(r):
    return r.choice([1, 2, 3, 7, 64, 300, r.randrange(1, 300)]) if r.random() > 0.15 else r.choice([0, 0, -1, -7])


def round24(x):
    e = 0
    while x >= 1 << 24:
        x, e = x / 2, e + 1
    while x < 1 << 23:
        x, e = x * 2, e - 1
    m, rest = divmod(x, 1)
    if rest > Fraction(1, 2) or rest == Fraction(1, 2) and m % 2:
        m += 1
    return Fraction(int(m)) * Fraction(2) ** e


def ties(r, cache, a, b):
    x, y = r.randrange(1 << 30, (1 << 31) - 200), r.randrange(1 << 25, 1 << 26)
    q = round24(Fraction(x, y))
    put(cache, 0x1800 + 4 * b, 'i', y)
    put(cache, 0x1818 + 4 * b, 'i', x)
    put(cache, 0x1830 + 4 * b, 'i', 0)
    put(cache, 0x1800 + 4 * a, 'i', 1 << 19)
    put(cache, 0x1818 + 4 * a, 'i', x + 100)
    put(cache, 0x1830 + 4 * a, 'i', x + 100 - int(2 * q * (1 << 19)))
    if r.random() < 0.3:   # (a's share 0 / 0)
        put(cache, 0x1800 + 4 * a, 'i', 0)
        put(cache, 0x1830 + 4 * a, 'i', x + 100)


def case(r):
    log = log_region()
    a, b = r.randrange(6), r.randrange(6)
    cache = bytearray(r.bytes(0x1900))
    if a != b and r.random() < 0.1:
        ties(r, cache, a, b)
        put(cache, 0x400 * a, 'I', ENTRY)
        set_return(log, 3, 0)
        regions = {CACHE: cache, ENTRY: struct.pack('<I', SURF) + r.bytes(0x1c), SURF: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({}, 8),
                   LOG: log}
        return Call(0x58ea20, [CACHE, a, b, 1], regions, conv='t', ret='a')
    totals = {}
    for k in (a, b):
        n = count(r)
        total = r.choice([r.randrange(0, 400), n * 2, n, r.getrandbits(32) - (1 << 31)])
        totals[k] = total
        put(cache, 0x1800 + 4 * k, 'i', n)
        put(cache, 0x1818 + 4 * k, 'i', total)
        put(cache, 0x1830 + 4 * k, 'i', r.choice([0, total // 2, total, total, r.randrange(0, 400), r.getrandbits(32) - (1 << 31)]))
    if a != b and r.random() < 0.6:
        put(cache, 0x1818 + 4 * b, 'i', max(-0x80000000, min(0x7fffffff, totals[a] - r.choice([9, 10, 11, 50]))))
    put(cache, 0x400 * a, 'I', ENTRY)
    limit = r.choice([0, 1, 3, 100, 300, r.randrange(1000), r.getrandbits(32) - (1 << 31)])
    used = r.choice([limit * 4 // 3, limit * 4 // 3 + 1, limit * 4 // 3 - 1, r.randrange(1000), r.getrandbits(32)])
    set_return(log, 3, used)
    regions = {CACHE: cache, ENTRY: struct.pack('<I', SURF) + r.bytes(0x1c), SURF: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({}, 8),
               LOG: log}
    return Call(0x58ea20, [CACHE, a, b, limit & 0xffffffff], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

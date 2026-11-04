# The draw buffer's three sorts of batch indices (src/game/render/drawbuffer: 0x582810 by the dword +8, 0x5828f0 up and
# 0x5829e0 down by the float +0x30) against the originals, their recursion real. Mostly valid orderings of 0-40 batches
# with repeated keys; some with NaNs that never become a pivot, as a NaN pivot recurses for ever in both.
# usage: drawbuffersort.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

BY8, UP, DOWN = 0x582810, 0x5828f0, 0x5829e0
B = Bench([(0x582810, 0x5828ec), (0x5828f0, 0x5829d4), (0x5829e0, 0x582ac4)])
SELF, ARRAY, BATCHES = RAM, RAM + 0x100, RAM + 0x1000
CHOSEN = [{'BY8': BY8, 'UP': UP, 'DOWN': DOWN}[k] for k in os.environ.get('OPS', 'BY8,UP,DOWN').split(',')]
TESTS = {BY8: (lambda v, p: v < p, lambda v, p: v > p), UP: (lambda v, p: not v >= p, lambda v, p: v > p),
         DOWN: (lambda v, p: v > p, lambda v, p: not v >= p)}


def ends(op, keys, order, lo, hi, depth=0):
    """Whether the sort of these ends: `order` sorted in place by the original's steps, up to a recursion that shows it would not."""
    before, after = TESTS[op]
    if depth > 60:
        return False
    while True:
        i, j = lo, hi
        pivot = keys[order[(lo + hi) >> 1]]
        while True:
            while i <= hi and before(keys[order[i]], pivot):
                i += 1
            while j > lo and after(keys[order[j]], pivot):
                j -= 1
            if i > j:
                break
            order[i], order[j] = order[j], order[i]
            i += 1
            j -= 1
            if i > j:
                break
        if lo < j and not ends(op, keys, order, lo, j, depth + 1):
            return False
        if hi <= i:
            return True
        lo = i


def case(r):
    op = r.choice(CHOSEN)
    count = r.choice([0, 1, 2, 3, r.randrange(4, 12), r.randrange(4, 41)])
    first = r.choice([0, 0, 0, r.randrange(-6, 7)])                         # (the range need not start at the array's start)
    length = r.choice([count, count, count, r.randrange(0, count + 1)])     # (nor be all of it)
    lo, hi = first, first + length - 1
    if op == BY8:
        pool = [r.getrandbits(32) for _ in range(5)] + [0, 1, 0x80000000, 0xffffffff]
        keys = [r.choice(pool) if r.random() < 0.7 else r.getrandbits(32) for _ in range(count)]
    else:
        pool = [r.uniform(-9, 9) for _ in range(4)] + [0.0, -0.0, 1.0, math.inf, -math.inf]
        keys = [struct.unpack('<f', struct.pack('<f', r.choice(pool) if r.random() < 0.7 else r.uniform(-1e6, 1e6)))[0] for _ in range(count)]
        if count and r.random() < 0.3:
            for _ in range(r.randrange(1, 4)):
                keys[r.randrange(count)] = math.nan
    order = list(range(count))
    r.shuffle(order)
    if count > 2 and r.random() < 0.1:
        order[r.randrange(count)] = order[r.randrange(count)]
    span = {lo + i: order[i] for i in range(length)}
    if any(k != k for k in keys) and length and not ends(op, keys, dict(span), lo, hi):
        keys = [0.5 if k != k else k for k in keys]
    batches = bytearray(r.bytes(0x38 * max(count, 1)))
    for i, key in enumerate(keys):
        struct.pack_into('<I' if op == BY8 else '<f', batches, i * 0x38 + (8 if op == BY8 else 0x30), key)
    self_ = bytearray(r.bytes(0x50))
    struct.pack_into('<I', self_, 0x44, BATCHES)
    regions = {SELF: bytes(self_), BATCHES: bytes(batches)}
    # (element `first` at ARRAY + 0x40, the pointer given `first` elements below it; 16 valid indices below that, as an empty range reads one for its pivot)
    below = [r.randrange(max(count, 1)) for _ in range(16)]
    regions[ARRAY] = struct.pack('<%dI' % (16 + count), *below, *order) + r.bytes(0x40)
    return Call(op, [SELF, (ARRAY + 0x40 - first * 4) & 0xffffffff, lo & 0xffffffff, hi & 0xffffffff], regions, conv='t')


if __name__ == '__main__':
    B.main(case)

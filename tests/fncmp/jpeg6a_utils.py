# libjpeg 6a's jutils.c (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a, its names jpeg6a_*) against the exe's
# 6a copies: jdiv_round_up, jround_up, and the row and block copies and jzero_far (the exe's inline REP MOVS / STOS, the library's
# memcpy / memset). The source is 6b's, so this is jpeg6b_utils.py at 6a's addresses, with jdiv_round_up (6b's is a hand port).
# Sample rows are arrays of row pointers (at IN_ROWS, OUT_ROWS, into IN and OUT; shuffled and repeated sometimes, never overlapping
# a source: libjpeg's never do).
# usage: jpeg6a_utils.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=div_round_up,round_up,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {
    'jdiv_round_up': (0x6503d0, 0x6503e3), 'jround_up': (0x6503e3, 0x650401), 'jcopy_sample_rows': (0x650401, 0x65047f),
    'jcopy_block_row': (0x65047f, 0x6504a2), 'jzero_far': (0x6504a2, 0x6504bf),
}
B = Bench(sorted(FUNCS.values()))

IN_ROWS, OUT_ROWS, IN, OUT = RAM, RAM + 0x1000, RAM + 0x10000, RAM + 0x20000
SIZE = 0x8000


def operands(r):
    b = r.choice([1, 2, 8, 16, r.randrange(1, 64), r.randrange(1, 1 << 16)])
    a = r.choice([0, 1, b - 1, b, b + 1, r.randrange(1 << 16), r.randrange(-b, b)])
    if r.wild:  # (a + b - 1 wraps, as both compute it)
        a = r.choice([a, r.randrange(-1 << 31, 1 << 31)])
        b = r.choice([b, r.randrange(1, 1 << 31)])
    return [a & 0xffffffff, b]


def div_round_up(r):
    return Call(0x6503d0, operands(r), {}, ret='a')


def round_up(r):
    return Call(0x6503e3, operands(r), {}, ret='a')


def rows(r, n, base, width):
    """n row pointers into base's region (rows width apart, in some order, a row twice sometimes)."""
    order = list(range(n))
    if r.random() < 0.3:
        r.shuffle(order)
    if n > 1 and r.random() < 0.1:
        order[r.randrange(n)] = order[r.randrange(n)]
    return struct.pack('<%dI' % n, *(base + i * width for i in order))


def copy_sample_rows(r):
    cols = r.choice([0, 1, 3, 4, 5, 8, r.randrange(1, 300)])
    num, src, dst = r.choice([0, 1, 2, r.randrange(1, 16)]), r.randrange(4), r.randrange(4)
    width = cols + r.randrange(4)
    total = num + 4
    regions = {IN_ROWS: rows(r, total, IN, width), OUT_ROWS: rows(r, total, OUT, width), IN: r.bytes(total * width + 1),
               OUT: r.bytes(total * width + 1)}
    return Call(0x650401, [IN_ROWS, src, OUT_ROWS, dst, num if r.random() < 0.95 else -num, cols], regions)


def copy_block_row(r):
    n = r.choice([0, 1, 2, r.randrange(1, 64)])
    return Call(0x65047f, [IN + r.choice([0, 2, 4]), OUT + r.choice([0, 2, 4]), n], {IN: r.bytes(n * 128 + 8), OUT: r.bytes(n * 128 + 8)})


def zero_far(r):
    n = r.choice([0, 1, 2, 3, 4, 5, r.randrange(64), r.randrange(SIZE - 8)])
    return Call(0x6504a2, [OUT + r.randrange(4), n], {OUT: r.bytes(n + 8)})


OPS = [f for f in (div_round_up, round_up, copy_sample_rows, copy_block_row, zero_far)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)

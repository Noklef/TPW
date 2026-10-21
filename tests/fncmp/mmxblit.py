# The software renderer's MMX picture copies (src/game/gfx/mmxblit: 0x678790 16 bytes a pass, 0x678c20 32 bytes a pass with the clear)
# against the originals. Copy16: width 4-0x40 (now and then below 4: a pass count of 0, which runs on until it faults in both), rows
# 1-4 (now and then 0, the same), pitches from the row's size to a little over. Copy32: width -9-0x30 (a width of 8 or less one pass),
# rows 1-4 (now and then 0), clear's low byte 0 half the time (its upper bytes random), the fill and the qword 0x77e580 random, the
# depth pitch 0x10-0x30; the globals 0xfab480-0xfab4bf and 0xfac518-0xfac51f random before.
# usage: mmxblit.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Copy16,Copy32)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'Copy16': 0x678790, 'Copy32': 0x678c20}
B = Bench([(0x678790, 0x6787e3), (0x678c20, 0x678d30)])
SOURCE, OUT, DEPTH = RAM, RAM + 0x2000, RAM + 0x4000


def rows(r):
    return r.choice([1, 1, 2, 3, 4]) if r.random() < 0.97 else 0


def case(r):
    op = r.deal(OPS)
    if op == 'Copy16':
        width = r.randrange(4, 0x41) if r.random() < 0.97 else r.randrange(4)
        n, row = rows(r), (width >> 2) * 16
        sp, op_ = row + r.choice([0, 0, 4, 0x10, 3]), row + r.choice([0, 0, 8, 0x20, 1])
        regions = {SOURCE: r.bytes(4 * sp + row + 0x10), OUT: r.bytes(4 * op_ + row + 0x10)}
        return Call(CALLS[op], [SOURCE, OUT, width, n, sp, op_], regions, ret='v')
    width = r.randrange(-9, 0x31)
    n, row = rows(r), max(1, (width + 7) // 8) * 0x20
    sp, op_, dp = row + r.choice([0, 0, 8, 0x20]), row + r.choice([0, 0, 4, 0x20, 1]), r.choice([0x10, 0x20, 0x30])
    clear = r.getrandbits(32) & (0xffffff00 if r.random() < 0.5 else 0xffffffff)
    regions = {SOURCE: r.bytes(4 * sp + row), OUT: r.bytes(4 * op_ + row), DEPTH: r.bytes(4 * dp + row),
               0xfab480: r.bytes(0x40), 0xfac518: r.bytes(8), 0x77e580: r.bytes(8)}
    return Call(CALLS[op], [SOURCE, OUT, width & 0xffffffff, n, sp, op_, DEPTH, dp, clear, r.getrandbits(32), r.getrandbits(32)], regions,
                ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

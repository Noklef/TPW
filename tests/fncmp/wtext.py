# UTF-16 text scanning (src/game/files/wtext: 0x413380, 0x4138b0) against the original: words
# of a small alphabet with every separator, alike or one character off, and texts of blank,
# empty and content lines with each kind of line end, from the middle of a line or its end.
# usage: wtext.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Words,Lines)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x413380, 0x41340b), (0x4138b0, 0x41392b)])
A, T = RAM, RAM + 0x1000
SEPS = [0x20, 9, 10, 13]
LETTERS = [0x41, 0x61, 0x5f, 0x20ac, 0x2041, 0xfeff]


def wstr(chars, r):
    return struct.pack('<%dH' % (len(chars) + 1), *chars, 0) + r.bytes(8)


def word(r):
    return [r.choice(LETTERS) for _ in range(r.choice([0, 1, 2, r.randrange(3, 9)]))]


def case(r):
    op = r.choice(OPS)
    if op == 'Words':
        w = word(r)
        tail = lambda: r.choice([[], [r.choice(SEPS)] + word(r)])  # noqa: E731
        a, b = w + tail(), w + tail()
        if b and r.random() < 0.4:  # one character off, or one longer
            b[r.randrange(len(b))] = r.choice(LETTERS + SEPS)
        if r.random() < 0.2:
            a.append(r.choice(LETTERS))
        regs = {A: wstr(a, r), T: wstr(b, r)}
        return Call(0x413380, [A, T], regs, ret='a')
    ends = [[10], [13], [13, 10], [10, 13], [10, 10], [13, 13]]
    chars = []
    for _ in range(r.randrange(0, 7)):
        kind = r.randrange(3)
        line = [] if kind == 0 else [r.choice([0x20, 9]) for _ in range(r.randrange(1, 4))] if kind == 1 else \
            [r.choice([0x20, 9]) for _ in range(r.randrange(0, 3))] + word(r) + [r.choice(LETTERS)]
        chars += line + r.choice(ends)
    chars += r.choice([[], word(r), [0x20, 9]])
    start = r.randrange(len(chars) + 1)
    return Call(0x4138b0, [T + 2 * start], {T: wstr(chars, r)}, ret='a')


OPS = [k for k in os.environ.get('OPS', 'Words,Lines').split(',') if k]


if __name__ == '__main__':
    B.main(case)

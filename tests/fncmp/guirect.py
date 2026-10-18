# The GUI library's rectangle (src/game/ui/guirect: 0x659d00-0x65a43e) against the originals: a rectangle of random bytes (now and then
# four words near each other, or edge values), the words given random, small or edge values (the upper halves random: only the low words
# count), and the centre's pointers given or 0.
# usage: guirect.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,CtorWith,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

CALLS = {'Ctor': 0x659d00, 'CtorWith': 0x659d43, 'Dtor': 0x659d90, 'Contains': 0x659e83, 'Centre': 0x659ee2, 'Offset': 0x65a3e9}
B = Bench([(0x659d00, 0x659dbc), (0x659e83, 0x659f4d), (0x65a3e9, 0x65a4a3), (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c01b)])
RECT, OUT = RAM, RAM + 0x100


def word(r):
    w = r.choice([0, 1, 2, 5, 100, 0x7fff, 0x8000, 0xffff, 0xfffe, r.randrange(-40, 40) & 0xffff, r.getrandbits(16)])
    return w | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0)


def case(r):
    op = r.deal(OPS)
    rect = bytearray(r.bytes(0x10))
    if r.random() < 0.6:
        x, y = r.randrange(-50, 50), r.randrange(-50, 50)
        struct.pack_into('<hhhh', rect, 4, x, y, x + r.randrange(-5, 40), y + r.randrange(-5, 40))
    regions = {RECT: rect, OUT: r.bytes(8)}
    if op in ('Ctor', 'Dtor'):
        return Call(CALLS[op], [RECT], regions, conv='t', ret='a')
    if op == 'CtorWith':
        return Call(CALLS[op], [RECT] + [word(r) for _ in range(4)], regions, conv='t', ret='a')
    if op == 'Contains':
        left, top, right, bottom = struct.unpack_from('<hhhh', rect, 4)
        x = r.choice([left, right, right - 1, left - 1, (left + right) // 2, r.randrange(-60, 90)]) & 0xffff
        y = r.choice([top, bottom, bottom - 1, top - 1, (top + bottom) // 2, r.randrange(-60, 90)]) & 0xffff
        if r.random() < 0.15:
            x, y = word(r), word(r)
        return Call(CALLS[op], [RECT, x | (r.getrandbits(16) << 16), y | (r.getrandbits(16) << 16)], regions, conv='t', ret='w')
    if op == 'Centre':
        return Call(CALLS[op], [RECT, r.choice([OUT, OUT, 0]), r.choice([OUT + 4, OUT + 4, 0])], regions, conv='t')
    if op == 'Offset':
        return Call(CALLS[op], [RECT, word(r), word(r)], regions, conv='t')
    return Call(CALLS[op], [RECT] + [word(r) for _ in range(4)], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

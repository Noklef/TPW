# The GUI library's rectangle (src/game/ui/guirect: 0x659d00-0x65a43e) against the originals: a rectangle of random bytes (now and then
# four words near each other, or edge values), the words given random, small or edge values (the upper halves random: only the low words
# count), and the centre's pointers given or 0. Overlap (0x659f3d): see overlap_case. Split (0x65a1c3): overlap_case's rectangle and
# shape (now and then none), the two out rectangles given or 0. Delete (0x665eb0, flags 0-3 or any): one rectangle, or with flag 2 an
# array of 0-4 (the count dword before them, now and then negative) destroyed last first (CRT_VectorDtorIterator) and its block freed.
# usage: guirect.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,CtorWith,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap  # noqa: E402

CALLS = {'Equals': 0x661700, 'GetSize': 0x659e3e, 'Ctor': 0x659d00, 'CtorWith': 0x659d43, 'Dtor': 0x659d90, 'Contains': 0x659e83, 'Centre': 0x659ee2, 'Offset': 0x65a3e9,
         'Adjust': 0x65a43e, 'Overlap': 0x659f3d, 'Split': 0x65a1c3, 'Delete': 0x665eb0}
B = Bench([(0x661700, 0x66176a), (0x659e3e, 0x659e83), (0x659d00, 0x659dbc), (0x659e83, 0x65a1c3), (0x65a3e9, 0x65a4a3), (0x65a1c3, 0x65a3e9), (0x659dbc, 0x659de3), (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c01b), (0x66c01a, 0x66c7dc),
           (0x67acb0, 0x67acb1), (0x665eb0, 0x665f0e), (0x67afa0, 0x67b0d0), (0x67b050, 0x67b0d0)])
RECT, OUT, SHAPE, POINTS, OUT2, ARRAY = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500
SHAPES = {'rect': 0x705920, 'circle': 0x705b50, 'ellipse': 0x705b68, 'polygon': 0x705b80}


def word(r):
    w = r.choice([0, 1, 2, 5, 100, 0x7fff, 0x8000, 0xffff, 0xfffe, r.randrange(-40, 40) & 0xffff, r.getrandbits(16)])
    return w | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0)


def overlap_case(r, rect, regions):
    """Overlap: the rectangle a GuiRect (its slot 0 Contains), the shape mostly another rectangle the same (now and then with the
    bottom at the top), inside, around, across, spanning it or apart (edges equal or off by one), else a circle, an ellipse (radii not 0) or a
    polygon of 0-6 points about it"""
    struct.pack_into('<I', rect, 0, SHAPES['rect'])
    left, top, right, bottom = struct.unpack_from('<hhhh', rect, 4)
    kind = r.choice(['rect'] * 7 + ['circle', 'ellipse', 'polygon'])
    shape = bytearray(r.bytes(0x10))
    struct.pack_into('<I', shape, 0, SHAPES[kind])
    if kind == 'rect':
        def edge(lo, hi):
            return r.choice([lo, lo, lo - 1, lo + 1, hi, hi - 1, hi + 1, (lo + hi) // 2, lo - 10, hi + 10, r.randrange(-60, 90)])
        sl, sr = edge(left, right), edge(left, right)
        st, sb = edge(top, bottom), edge(top, bottom)
        how = r.random()
        if how < 0.15:
            sl, st, sr, sb = left, top, right, r.choice([top, top, bottom])
        elif how < 0.25:
            sl, st, sr, sb = left + 1, top + 1, right - 1, bottom - 1
        elif how < 0.35:
            sl, sr = r.choice([left, left - 1, left - 5]), r.choice([right, right + 1, right + 5])
        elif how < 0.45:
            st, sb = r.choice([top, top - 1, top - 5]), r.choice([bottom, bottom + 1, bottom + 5])
        struct.pack_into('<HHHH', shape, 4, *[v & 0xffff for v in (sl, st, sr, sb)])
    elif kind == 'polygon':
        n = r.choice([0, 1, 3, 4, 6])
        struct.pack_into('<hxxI', shape, 4, n, POINTS)
        regions[POINTS] = b''.join(struct.pack('<HH', r.randrange(min(left, right) - 10, max(left, right) + 11) & 0xffff, r.randrange(min(top, bottom) - 10, max(top, bottom) + 11) & 0xffff) for _ in range(n)) + r.bytes(4)
    else:
        struct.pack_into('<HHh', shape, 4, r.randrange(min(left, right) - 10, max(left, right) + 11) & 0xffff, r.randrange(min(top, bottom) - 10, max(top, bottom) + 11) & 0xffff, r.choice([1, 3, 10, 30, -5]))
        if kind == 'ellipse':
            struct.pack_into('<h', shape, 0xa, r.choice([1, 4, 12, 40, -6]))
    regions[SHAPE] = shape
    return Call(CALLS['Overlap'], [RECT, SHAPE], regions, conv='t', ret='w')


def case(r):
    op = r.deal(OPS)
    rect = bytearray(r.bytes(0x10))
    if r.random() < 0.6:
        x, y = r.randrange(-50, 50), r.randrange(-50, 50)
        struct.pack_into('<hhhh', rect, 4, x, y, x + r.randrange(-5, 40), y + r.randrange(-5, 40))
    regions = {RECT: rect, OUT: r.bytes(8)}
    if op == 'Overlap':
        return overlap_case(r, rect, regions)
    if op == 'Split':
        overlap_case(r, rect, regions)
        regions[OUT], regions[OUT2] = r.bytes(0x10), r.bytes(0x10)
        args = [RECT, r.choice([SHAPE] * 15 + [0]), r.choice([OUT] * 4 + [0]), r.choice([OUT2] * 4 + [0])]
        return Call(CALLS[op], args, regions, conv='t', ret='w')
    if op == 'Delete':
        flags = r.choice([0, 1, 2, 3, 3, r.getrandbits(32)])
        count = r.choice([0, 1, 2, 3, 4, -1])
        block = struct.pack('<i', count) + b''.join(bytes(rect if i == 0 else r.bytes(0xc))[:0xc] for i in range(4))
        return Call(CALLS[op], [ARRAY + 4, flags], {ARRAY: block, HEAP: heap(0x100)}, conv='t', ret='a')
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
    if op == 'Equals':   # the other rectangle a copy of this one, with one word changed now and then (in its low or high byte)
        other = bytearray(rect)
        if r.random() < 0.6:
            k = r.choice([4, 5, 6, 7, 8, 9, 10, 11])
            other[k] ^= r.choice([1, 0x80, 0xff])
        regions[OUT] = bytes(other)
        return Call(CALLS[op], [RECT, OUT], regions, conv='t', ret='a')
    if op in ('Centre', 'GetSize'):
        return Call(CALLS[op], [RECT, r.choice([OUT, OUT, 0]), r.choice([OUT + 4, OUT + 4, 0])], regions, conv='t')
    if op == 'Offset':
        return Call(CALLS[op], [RECT, word(r), word(r)], regions, conv='t')
    return Call(CALLS[op], [RECT] + [word(r) for _ in range(4)], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

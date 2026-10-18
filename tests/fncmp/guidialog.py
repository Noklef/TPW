# The GUI library's dialog base (src/game/ui/guidialog: vtable 0x7058f8) against the originals: a dialog of random bytes (now and then its
# limit word +0xe small or an edge value), words with random upper halves (only the low word counts), the source of Apply random bytes or
# none with flags from its four bits, mixes and random ones; the scalar destructor's flags any (the heap region given: its free count and
# hash).
# usage: guidialog.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetA,SetC,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap  # noqa: E402

CALLS = {'SetA': 0x6599f2, 'SetC': 0x659a19, 'Apply': 0x6598f7, 'Ctor': 0x6597b0, 'Dtor': 0x659886, 'ScalarDtor': 0x659cb0}
B = Bench([(0x6597b0, 0x6598ed), (0x6598f7, 0x6599bc), (0x6599f2, 0x659a50), (0x659cb0, 0x659cf0), (0x659d00, 0x659d53), (0x659d90, 0x659dbc),
           (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c01b)])
DIALOG, SOURCE = RAM, RAM + 0x100


def word(r):
    w = r.choice([0, 1, 2, 5, 9, 10, 11, 0x7fff, 0x8000, 0xffff, r.randrange(-20, 20) & 0xffff, r.getrandbits(16)])
    return w | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0)


def case(r):
    op = r.deal(OPS)
    dialog = bytearray(r.bytes(0x40))
    if r.random() < 0.6:
        struct.pack_into('<H', dialog, 0xe, r.choice([10, 0, 0x7fff, 0x8000, 0xffff]))
    regions = {DIALOG: dialog}
    if op in ('SetA', 'SetC'):
        return Call(CALLS[op], [DIALOG, word(r)], regions, conv='t')
    if op == 'Apply':
        source = bytearray(r.bytes(0x10))
        for k in (4, 0xa):
            if r.random() < 0.5:
                struct.pack_into('<H', source, k, r.choice([9, 10, 11, 0, 0xffff]))
        regions[SOURCE] = source
        flags = r.choice([0, 1, 4, 0x40, 0x80000, 0x80045, r.getrandbits(32), r.getrandbits(32) & 0x80045])
        return Call(CALLS[op], [DIALOG, flags, r.choice([SOURCE] * 6 + [0])], regions, conv='t')
    if op == 'Ctor':
        return Call(CALLS[op], [DIALOG, r.getrandbits(32)], regions, conv='t', ret='a')
    if op == 'Dtor':
        return Call(CALLS[op], [DIALOG], regions, conv='t')
    regions[HEAP] = heap(0x20)
    return Call(CALLS[op], [DIALOG, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The GUI library's text pools (src/game/ui/textpool: 0x669440, 0x669586, 0x6695b9, 0x66969e) against the originals. Init: the four
# sizes small (now and then 0, 4 or less, equal or negative), the short row count below, equal to or above the long one; the heap
# failing the first or second allocation now and then. Free: the blocks set or 0. Take: pools of 1-8 rows of 5-20 characters (each
# row free or not), the blocks now and then 0, the length near either row length or anywhere. Release: a row or 0. The globals start
# random.
# usage: textpool.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Init,Free,Take,Release)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, heap  # noqa: E402

CALLS = {'Init': 0x669440, 'Free': 0x669586, 'Take': 0x6695b9, 'Release': 0x66969e}
B = Bench([(0x669440, 0x669586), (0x669586, 0x6695b9), (0x6695b9, 0x66969e), (0x66969e, 0x6696b4)])
LONG, SHORT = RAM, RAM + 0x1000
GLOBALS = 0xfaa63c   # short block, short length, long block, long length, long rows, short rows


def pool(r, rows, length):
    block = bytearray(r.bytes(rows * length * 2))
    for i in range(rows):
        if r.random() < 0.3:
            struct.pack_into('<i', block, i * length * 2, -1)
    return block


def case(r):
    op = r.deal(OPS)
    regions = {GLOBALS: r.bytes(0x18)}
    if op == 'Init':
        regions[HEAP] = heap(0x4000, r.choice([0] * 8 + [1, 2]))
        long_length = r.choice([0, 4, 5, 6] + [r.randrange(6, 40)] * 8)
        short_length = r.choice([0, 3, 4, 5, long_length, long_length + 1] + [r.randrange(5, max(6, long_length))] * 6)
        long_rows = r.choice([0, -1, 1] + [r.randrange(1, 20)] * 6)
        short_rows = r.choice([0, -2, 1, long_rows, long_rows + 1] + [r.randrange(1, 20)] * 5)
        args = [long_length, long_rows, short_length, short_rows]
        if r.random() < 0.05:
            args[r.randrange(4)] = r.getrandbits(32)
        return Call(CALLS[op], [a & 0xffffffff for a in args], regions, ret='a')
    if op == 'Free':
        regions[HEAP] = heap(0x40)
        regions[GLOBALS] = struct.pack('<6I', r.choice([HEAP + 0x10, 0]), r.getrandbits(32), r.choice([HEAP + 0x20, HEAP + 0x20, 0]),
                                       *[r.getrandbits(32) for _ in range(3)])
        return Call(CALLS[op], [], regions)
    long_rows, short_rows = r.randrange(1, 9), r.randrange(1, 9)
    short_length = r.randrange(5, 15)
    long_length = short_length + r.randrange(1, 8)
    regions[LONG], regions[SHORT] = pool(r, long_rows, long_length), pool(r, short_rows, short_length)
    if op == 'Release':
        row = r.choice([0, LONG + 2 * long_length * r.randrange(long_rows), SHORT + 2 * short_length * r.randrange(short_rows)])
        return Call(CALLS[op], [row], regions)
    regions[GLOBALS] = struct.pack('<6I', r.choice([SHORT] * 9 + [0]), short_length, r.choice([LONG] * 9 + [0]), long_length,
                                   long_rows, short_rows)
    length = r.choice([short_length - 1, short_length, long_length - 1, long_length, r.randrange(-3, 25), r.getrandbits(32)])
    return Call(CALLS[op], [length & 0xffffffff], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

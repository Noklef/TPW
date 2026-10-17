# The game string's token read (src/game/core/string: 0x5e3170) against the original: texts of 0 to
# 40 chars (NULs included) up to a delimiter (one of several, NUL included), the delimiter first, repeated or
# right after, the cursor anywhere in the text.
# usage: stringtoken.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x5e3170, 0x5e3208), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)])
OUT, CURSOR, TEXT = RAM, RAM + 0x40, RAM + 0x100
ALPHA = b'abcAZ09 ,;|\x80\xff\0'


def case(r):
    delim = r.choice([ord(','), ord(';'), ord('|'), 0, ord(' '), 0x80, r.getrandbits(8)])
    head = bytes(c for c in (r.choice(ALPHA) for _ in range(r.choice([0, 0, 1, 2, 7, 8, 9, 16, 17, r.randrange(41)]))) if c != delim)
    prefix = r.bytes(r.randrange(4))
    text = prefix + head + bytes([delim]) + r.bytes(r.randrange(8)) + bytes([delim])
    start = len(prefix) if r.random() < 0.8 else r.randrange(len(prefix) + len(head) + 1)
    regions = {OUT: r.bytes(0x20), CURSOR: put(bytearray(r.bytes(8)), 0, 'I', TEXT + start), TEXT: text + r.bytes(0x10), HEAP: heap(0x2000)}
    return Call(0x5e3170, [OUT, CURSOR, delim | r.getrandbits(24) << 8], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

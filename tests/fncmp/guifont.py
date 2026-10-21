# The GUI library's font (src/game/ui/guifont: 0x658334) against the originals. TextWidth: a font with glyphs (0x200 of 8 bytes, the
# widths any byte) or none, the spacing small or any; a text of 0-12 characters below 0x200 (or none); from / to about the length,
# negative or any (their upper halves random).
# Init (0x658243): a font of random bytes, the count 0 to 0x200 (upper half random), the allocation failing now and then (the heap),
# the bytes and words any (upper bits random).
# usage: guifont.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=TextWidth,Init)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, heap, put  # noqa: E402

CALLS = {'TextWidth': 0x658334, 'Init': 0x658243}
B = Bench([(0x658334, 0x6583fe), (0x67acd0, 0x67acee), (0x658243, 0x6582ec)])
FONT, TEXT, GLYPHS = RAM, RAM + 0x100, RAM + 0x1000


def index(r, n):
    return r.choice([0, 1, n - 1, n, n + 1, -1, r.randrange(-2, n + 3), 0x7fff, -0x8000, r.getrandbits(16)]) & 0xffff | r.getrandbits(16) << 16


def case(r):
    op = r.deal(OPS)
    if op == 'Init':
        count = r.choice([0, 1, 2, 3, 7, 0x80, 0x200, r.randrange(0x200)]) | r.choice([0, r.getrandbits(16)]) << 16
        regions = {FONT: r.bytes(0x10), HEAP: heap(0x2000, r.choice([0] * 6 + [1]))}
        return Call(CALLS[op], [FONT, count] + [r.getrandbits(32) for _ in range(4)], regions, conv='t', ret='a')
    n = r.choice([0, 1, 2, r.randrange(13)])
    font = bytearray(r.bytes(0x10))
    put(font, 0xc, 'I', GLYPHS if r.random() < 0.9 else 0)
    put(font, 6, 'H', r.choice([0, 1, 2, r.getrandbits(16)]))
    text = struct.pack('<%dH' % (n + 1), *([r.choice([0x20, 0x41, 0x61, r.randrange(1, 0x200)]) for _ in range(n)] + [0]))
    regions = {FONT: font, TEXT: text + r.bytes(4), GLYPHS: r.bytes(0x1000)}
    return Call(CALLS[op], [FONT, TEXT if r.random() < 0.95 else 0, index(r, n), index(r, n)], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

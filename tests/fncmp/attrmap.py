# The MAP chunk handler (src/game/terrain/attrmap: 0x519390, 0x4d8cf0; the vtable 0x700aa0's slots 2 and 3) against the originals.
# Match: the handler's tag +4 and the tag asked for equal or not. Read: a chunk (+0 width, +4 height: 0-20 by 0-140 now and then 0 or
# negative; its bytes from +0x1c) into the map cells' bytes +0x26 (the handler's +8 the cells, 0x44 bytes, 128 to a row; the first 16
# rows a region, the offsets 0x785584 / 0x785588 such that the rows written stay in them, now and then one past: a fault in both), the
# columns and rows past the map's 128 skipped.
# usage: attrmapchunk.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Match,Read)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

CALLS = {'Match': 0x519390, 'Read': 0x4d8cf0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x519390, 0x5193a3), (0x4d8cf0, 0x4d8d98)])
HANDLER, CHUNK, CELLS = RAM, RAM + 0x1000, RAM + 0x10000
ROWS = 16


def case(r):
    op = r.deal(OPS)
    handler = bytearray(r.bytes(0x10))
    put(handler, 8, 'I', CELLS + 0x44)   # (the cells' base: cell id 1 the first)
    regions = {HANDLER: handler}
    if op == 'Match':
        tag = r.choice([0x5041414d, 0x4d4150, 0, r.getrandbits(32)])
        put(handler, 4, 'I', tag)
        asked = r.choice([tag, tag, tag ^ 1, tag ^ 0x80000000, r.getrandbits(32)])
        return Call(CALLS['Match'], [HANDLER, asked], regions, conv='t', ret='a')
    w = r.choice([0, 1, 2, 5, 16, 20, r.randrange(21), -1])
    h = r.choice([0, 1, 3, 100, 128, 129, 140, r.randrange(141), -3])
    x0 = r.choice([0, 0, 1, 5, -3, 12, r.randrange(-10, 20)])   # (0x785584: a row's x is the row less it)
    y0 = r.choice([max(w, 0) - ROWS, 0, 2, max(w, 0) - 1, max(w, 0) + 2, r.randrange(max(w, 0) - ROWS, max(w, 0) + 4)])   # (0x785588)
    if r.random() < 0.03:
        y0 = max(w, 0) - ROWS - 1
    chunk = bytearray(r.bytes(0x1c + max(w, 0) * max(h, 0) + 4))
    put(chunk, 0, 'ii', w, h)
    regions.update({CHUNK: chunk, CELLS: r.bytes(0x44 * (128 * ROWS + 1)), 0x785584: struct.pack('<ii', x0, y0)})
    return Call(CALLS['Read'], [HANDLER, CHUNK, r.getrandbits(32)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

# The GUI library's kind 7 window (0x7059f0) and its item tree (src/game/ui/window7059f0) against the originals. ColumnBottom (0x6636f6): the
# column array +0x16c of 12 random rectangles, the column 0-11 (now and then -1 or -2: before the array), the upper half random. NextItem /
# NthItem (0x663a93, 0x663ba2): items(): a random forest of 1-12 items (each four dwords at an index 4k, or 4k + 1 now and then: the links
# then read across items, still in the array) whose missing links are -1 (now and then -2 or -5), the item given one of them, the level
# pointer given or 0; NthItem's n -1 to past the count +0x150 (itself 0 to past the items), +0x144 set or 0, the first shown +0x160 one of
# the items (or -1).
# usage: window7059f0.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ColumnBottom,NextItem,NthItem)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'ColumnBottom': 0x6636f6, 'NextItem': 0x663a93, 'NthItem': 0x663ba2}
B = Bench([(0x6636f6, 0x663718), (0x663a93, 0x663c11)])
WINDOW, COLUMNS, LINKS, OUT = RAM, RAM + 0x200, RAM + 0x400, RAM + 0x800


def none(r):
    return r.choice([-1] * 8 + [-2, -5])


def items(r, window, regions):
    """the forest: returns the item indices"""
    n = r.randint(1, 12)
    shift = 1 if r.random() < 0.1 else 0
    idx = [4 * k + shift for k in range(n)]
    links = [r.getrandbits(32) for _ in range(4 * n + 8)]
    parent = [None] + [r.choice([None, r.randrange(k)]) for k in range(1, n)]
    kids = {p: [k for k in range(n) if parent[k] == p] for p in [None] + list(range(n))}
    for k in range(n):
        links[idx[k] + 2] = idx[kids[k][0]] if kids[k] else none(r)
        links[idx[k] + 3] = idx[parent[k]] if parent[k] is not None else none(r)
        sibs = kids[parent[k]]
        at = sibs.index(k)
        links[idx[k]] = idx[sibs[at + 1]] if at + 1 < len(sibs) else none(r)
    regions[LINKS] = struct.pack('<%dI' % len(links), *[v & 0xffffffff for v in links])
    put(window, 0x144, 'I', LINKS)
    return idx


def case(r):
    op = r.deal(OPS)
    window = bytearray(r.bytes(0x180))
    regions = {WINDOW: window}
    if op == 'ColumnBottom':
        regions[COLUMNS - 0x20] = r.bytes(0x20 + 12 * 12)
        put(window, 0x16c, 'I', COLUMNS)
        column = r.choice(list(range(12)) + [-1, -2]) & 0xffff
        return Call(CALLS[op], [WINDOW, column | r.choice([0, r.getrandbits(16)]) << 16], regions, conv='t', ret='a')
    idx = items(r, window, regions)
    if op == 'NextItem':
        regions[OUT] = r.bytes(4)
        return Call(CALLS[op], [WINDOW, r.choice(idx), r.choice([OUT, OUT, 0])], regions, conv='t', ret='a')
    put(window, 0x150, 'i', r.choice([0, 1, len(idx), len(idx) + 3, r.randrange(len(idx) + 1), -1]))
    put(window, 0x160, 'i', r.choice(idx + [idx[0], -1]))
    if r.random() < 0.1:
        put(window, 0x144, 'I', 0)
    n = r.choice([-1, 0, 1, 2, 3, r.randrange(len(idx) + 2), len(idx) - 1, len(idx), 0x7fffffff, -0x80000000])
    return Call(CALLS[op], [WINDOW, n & 0xffffffff], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

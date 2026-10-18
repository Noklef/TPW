# The game's MSVC6 std::wstring members (src/game/core/wstdstring: assign 0x4f21d0, _Grow 0x4f39e0, _Copy 0x4f6090, _Split 0x4f6190) against
# the originals, as stdstring.py's narrow ones: a string under test and a second one, each null or a block (a byte, the count byte, up to
# 0x7c characters, usually a terminator) with counts about 0 and 0xff, at times sharing one block; Mem_Alloc / Mem_Free on the heap region.
# _Grow also on strings left unterminated (some empty), assign also for exactly the room s has.
# usage: wstdstring.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x4f21d0, 0x4f23e2), (0x4f23f0, 0x4f2437), (0x4f39e0, 0x4f3b06), (0x4f6080, 0x4f6086), (0x4f6090, 0x4f618e), (0x4f6190, 0x4f6266),
           (0x479150, 0x47919d), (0x47ba30, 0x47ba50), (0x67acd0, 0x67acee)])
STRS, BLOCKS = RAM, RAM + 0x1000
S, X = STRS, STRS + 0x10  # the string under test and a second one
SLOT = 0x100  # a block per string in BLOCKS: a byte, the count byte, then up to 0x7c characters + NUL


def string(r, blocks, k):
    """A string at slot k: null, or a block in BLOCKS. Returns (ptr, len, res)."""
    if r.random() < 0.15:
        return 0, 0, 0
    n = r.choice([0, 1, r.randrange(40), r.randrange(25, 0x70)])
    res = min(0x7b, r.choice([n, n + r.randrange(20), max(n, 31), 31]))
    at = k * SLOT
    blocks[at + 1] = r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    for i in range(n):
        put(blocks, at + 2 + 2 * i, 'H', r.randrange(1, 0x10000))
    if r.random() < 0.9:
        put(blocks, at + 2 + 2 * n, 'H', 0)
    if r.random() < 0.05 and n:
        put(blocks, at + 2 + 2 * r.randrange(n), 'H', 0)  # wcslen stops early in _Split
    return BLOCKS + at + 2, n, res


def case(r):
    op = r.deal(OPS)
    blocks = bytearray(r.bytes(4 * SLOT))
    strs = bytearray(r.bytes(0x20))
    s = string(r, blocks, 0)
    together = s[0] and r.random() < 0.2  # x shares s's block
    x = s if together else string(r, blocks, 1)
    if together:
        blocks[s[0] - BLOCKS - 1] = r.choice([1, 2, 0xfd, 0xfe])
    put(strs, 4, '3I', *s)
    put(strs, 0x14, '3I', *x)
    regions = {STRS: strs, BLOCKS: blocks, HEAP: heap()}
    small = r.choice([0, 1, r.randrange(40), r.randrange(30, 0x90), s[1], s[2], s[2] + 1])
    if op == 'Grow':
        if s[0] and r.random() < 0.3:  # no terminator where the length ends (an emptied string's is written)
            put(blocks, s[0] - BLOCKS + 2 * s[1], 'H', r.randrange(1, 0x10000))
        if s[0] and r.random() < 0.2:  # an unterminated empty string of its own, emptied again
            put(strs, 8, 'I', 0)
            blocks[s[0] - BLOCKS - 1] = r.choice([0, 0xff])
            put(blocks, s[0] - BLOCKS, 'H', r.randrange(1, 0x10000))
            small = r.choice([0, small])
        return Call(0x4f39e0, [S, small, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='b')
    if op == 'Copy':
        huge = r.choice([0x7ffffffe, 0x7fffffe0, 0xfffffff0, 0xffffffe1])  # the block size wraps negative
        return Call(0x4f6090, [S, r.choice([small] * 3 + [huge])], regions, conv='t')
    if op == 'Split':
        return Call(0x4f6190, [S], regions, conv='t')
    # AssignSub: from x, or from s itself
    src, (_, n, _) = r.choice([(X, x), (X, x), (S, s)])
    pos = r.choice([0, n, r.randrange(n + 1)])
    count = r.choice([0xffffffff, r.randrange(50), 0, n - pos, max(0, n - pos - 1), s[2]])  # (s's room: a block kept exactly full)
    return Call(0x4f21d0, [S, src, pos, count], regions, conv='t', ret='a')


KINDS = ['Grow', 'Copy', 'Split', 'AssignSub', 'AssignSub']
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

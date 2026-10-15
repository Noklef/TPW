# The game's MSVC6 std::string members (0x45b340-0x45b998) against the originals.
# usage: stdstring.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x45b340, 0x45b9a0)])
STRS, BLOCKS, CHARS = RAM, RAM + 0x1000, RAM + 0x2000
S, X = STRS, STRS + 0x10  # the string under test and a second one
SLOT = 0x80  # a block per string in BLOCKS: count byte, then up to 0x7e characters + NUL


def string(r, blocks, k):
    """A string at slot k: null, or a block in BLOCKS with a count byte, characters and usually a
    terminator. Returns (ptr, len, res)."""
    if r.random() < 0.15:
        return 0, 0, 0
    n = r.choice([0, 1, r.randrange(40), r.randrange(25, 100)])
    res = min(0x7d, r.choice([n, n + r.randrange(20), max(n, 31), 31]))
    at = k * SLOT
    blocks[at] = r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    blocks[at + 1:at + 1 + n] = bytes(r.randrange(1, 256) for _ in range(n))
    if r.random() < 0.9:
        blocks[at + 1 + n] = 0
    if r.random() < 0.05 and n:
        blocks[at + 1 + r.randrange(n)] = 0  # strlen stops early in _Split
    return BLOCKS + at + 1, n, res


def case(r):
    op = r.choice(OPS)
    blocks = bytearray(r.bytes(4 * SLOT))
    strs = bytearray(r.bytes(0x20))
    s = string(r, blocks, 0)
    together = s[0] and r.random() < 0.2  # x shares s's block
    x = s if together else string(r, blocks, 1)
    if together:
        blocks[s[0] - BLOCKS - 1] = r.choice([1, 2, 0xfd, 0xfe])
    put(strs, 4, '3I', *s)
    put(strs, 0x14, '3I', *x)
    regions = {STRS: strs, BLOCKS: blocks, CHARS: r.bytes(0x100), HEAP: heap()}
    small = r.choice([0, 1, r.randrange(40), r.randrange(30, 0x90), s[1], s[2], s[2] + 1])
    if op == 'Dtor':
        return Call(0x45b340, [S], regions, conv='t')
    if op == 'Tidy':
        return Call(0x45b590, [S, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t')
    if op == 'AssignChars':
        src = r.choice([CHARS + r.randrange(0x40), CHARS, s[0] or CHARS])
        return Call(0x45b5e0, [S, src, small], regions, conv='t', ret='a')
    if op == 'Eos':
        if not s[0]:
            return Call(0x45b7c0, [S], regions, conv='t', ret='a')
        return Call(0x45b680, [S, r.randrange(s[2] + 1)], regions, conv='t')
    if op == 'Grow':
        return Call(0x45b6a0, [S, small, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='b')
    if op == 'MaxSize':
        return Call(0x45b7c0, [S], regions, conv='t', ret='a')
    if op == 'Copy':
        huge = r.choice([0x7ffffffe, 0x7fffffe0, 0xfffffff0, 0xffffffe1])  # the block size wraps negative
        return Call(0x45b7d0, [S, r.choice([small] * 3 + [huge])], regions, conv='t')
    if op == 'Split':
        return Call(0x45b8d0, [S], regions, conv='t')
    # AssignSub: from x, or from s itself
    src, (_, n, _) = r.choice([(X, x), (X, x), (S, s)])
    pos = r.choice([0, n, r.randrange(n + 1)])
    count = r.choice([0xffffffff, r.randrange(50), 0, n - pos, max(0, n - pos - 1)])
    return Call(0x45b380, [S, src, pos, count], regions, conv='t', ret='a')


KINDS = ['Dtor', 'Tidy', 'AssignChars', 'Eos', 'Grow', 'MaxSize', 'Copy', 'Split', 'AssignSub', 'AssignSub']
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

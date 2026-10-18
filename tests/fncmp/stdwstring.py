# The game's MSVC6 std::wstring members (0x405280, 0x47b750-0x47bcd5) against the originals.
# usage: stdwstring.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# (with the wide string's helpers: the traits' move, the counted buffer reset, max_size and wcslen)
B = Bench([(0x405280, 0x405390), (0x479150, 0x4791b0), (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0),
           (0x67acd0, 0x67ad00)])
STRS, BLOCKS = RAM, RAM + 0x1000
S, X = STRS, STRS + 0x10  # the string under test and a second one
SLOT = 0x100  # a block per string in BLOCKS: a pad byte, the count byte, then up to 0x7e characters + NUL


def chars(r, n):
    return b''.join(r.choice([r.randrange(1, 0x100), r.randrange(1, 0x10000)]).to_bytes(2, 'little') for _ in range(n))


def string(r, blocks, k):
    """A string at slot k: null, or a block in BLOCKS with a count byte, characters and usually a
    terminator. Returns (ptr, len, res)."""
    if r.random() < 0.15:
        return 0, 0, 0
    n = r.choice([0, 1, r.randrange(40), r.randrange(25, 0x7e)])
    res = min(0x7d, r.choice([n, n + r.randrange(20), max(n, 31), 31]))
    at = k * SLOT
    blocks[at + 1] = r.choice([0, 0, 1, 2, 0xfd, 0xfe, 0xff])
    blocks[at + 2:at + 2 + 2 * n] = chars(r, n)
    if r.random() < 0.9:
        blocks[at + 2 + 2 * n:at + 4 + 2 * n] = bytes(2)
    if r.random() < 0.05 and n:
        i = r.randrange(n)
        blocks[at + 2 + 2 * i:at + 4 + 2 * i] = bytes(2)  # wcslen stops early in _Split
    return BLOCKS + at + 2, n, res


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
    regions = {STRS: strs, BLOCKS: blocks, HEAP: heap()}
    small = r.choice([0, 1, r.randrange(40), r.randrange(30, 0x90), s[1], s[2], s[2] + 1])
    if op == 'Grow':
        return Call(0x47bad0, [S, small, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='b')
    if op == 'Copy':
        huge = r.choice([0x7ffffffe, 0x7fffffe0, 0xfffffff0, 0xffffffe1, 0x3ffffff0])  # the block size wraps
        return Call(0x405280, [S, r.choice([small] * 3 + [huge])], regions, conv='t')
    if op == 'Split':
        return Call(0x47bc00, [S], regions, conv='t')
    if op == 'Freeze':
        return Call(0x47ba50, [S], regions, conv='t')
    if op == 'Fill':
        n = small  # (not npos or past max_size: the throws end the case)
        return Call(0x47b970, [S, n, r.choice([0, r.getrandbits(16), r.getrandbits(32)])], regions, conv='t', ret='a')
    # AssignSub: from x, or from s itself
    src, (_, n, _) = r.choice([(X, x), (X, x), (S, s)])
    pos = r.choice([0, n, r.randrange(n + 1)])  # (not past the end: _Xran ends the case)
    count = r.choice([0xffffffff, r.randrange(50), 0, n - pos, max(0, n - pos - 1)]) & 0xffffffff
    return Call(0x47b750, [S, src, pos, count], regions, conv='t', ret='a')


KINDS = ['Grow', 'Copy', 'Split', 'Freeze', 'Fill', 'AssignSub', 'AssignSub', 'AssignSub']
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

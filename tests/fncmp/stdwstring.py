# The game's MSVC6 std::wstring members (0x405280, 0x47b750-0x47bcd5, the constructor from characters 0x4f0170) against the originals.
# usage: stdwstring.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# (with the wide string's helpers: the traits' move, the counted buffer reset, max_size and wcslen)
B = Bench([(0x405280, 0x405390), (0x479150, 0x4791b0), (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0),
           (0x67acd0, 0x67ad00), (0x4ea740, 0x4ea7e0), (0x47ae60, 0x47b060), (0x47b600, 0x47b750), (0x479230, 0x479256),
           (0x47cc10, 0x47cd00), (0x47b460, 0x47b5b0), (0x47d9d0, 0x47db00), (0x47d620, 0x47d780), (0x47b320, 0x47b460),
           (0x4f0170, 0x4f0237)])
STRS, BLOCKS = RAM, RAM + 0x1000
S, X = STRS, STRS + 0x10  # the string under test and a second one
DEST = RAM + 0x800  # (UninitCopy's)
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
    if r.random() < (0.5 if n == 0 else 0.9):  # (an empty string left unterminated: _Grow(0, false) ends it)
        blocks[at + 2 + 2 * n:at + 4 + 2 * n] = bytes(2)
    if r.random() < 0.05 and n:
        i = r.randrange(n)
        blocks[at + 2 + 2 * i:at + 4 + 2 * i] = bytes(2)  # wcslen stops early in _Split
    return BLOCKS + at + 2, n, res


def case(r):
    op = r.choice(OPS)
    blocks = bytearray(r.bytes(8 * SLOT))   # (slots 4-7: CopyRange's destinations)
    strs = bytearray(r.bytes(0x50))
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
    if op == 'Entry':  # 0x4ea740: an entry at +0x20 built from s passed by value (its allocator dword random)
        return Call(0x4ea740, [STRS + 0x20, r.getrandbits(32), *s], regions, conv='t', ret='a')
    if op == 'Split':
        return Call(0x47bc00, [S], regions, conv='t')
    if op == 'Freeze':
        return Call(0x47ba50, [S], regions, conv='t')
    if op == 'Fill':
        n = small  # (not npos or past max_size: the throws end the case)
        return Call(0x47b970, [S, n, r.choice([0, r.getrandbits(16), r.getrandbits(32)])], regions, conv='t', ret='a')
    if op == 'CtorCopy':   # (s's fields are overwritten: x, or s itself; x's length past 0x7ffffff, only its block shared, now and then)
        if x[0] and r.random() < 0.15:
            blocks[x[0] - BLOCKS - 1] = r.choice([0, 1, 0xfd])
            put(strs, 0x18, 'I', r.choice([0x8000000, 0x7ffffffd, 0xfffffff0]))
        return Call(0x47ae60, [S, r.choice([X, X, X, S])], regions, conv='t', ret='a')
    if op == 'CtorChars':   # 0x4f0170: s's fields overwritten by a copy of 0-0x7e characters at the end of x's slot (or empty); the allocator's byte from anywhere
        n = r.choice([0, 0, 1, r.randrange(31), 31, 32, r.randrange(0x7e)])
        put(blocks, 2 * SLOT, '%dH' % (n + 1), *[r.randrange(1, 0x10000) for _ in range(n)], 0)
        return Call(0x4f0170, [S, BLOCKS + 2 * SLOT, r.choice([X, STRS + 0x31])], regions, conv='t', ret='a')
    if op == 'CtorFill':   # (the allocator's byte from anywhere)
        return Call(0x47af80, [S, small, r.choice([0, r.getrandbits(16), r.getrandbits(32)]), r.choice([X, STRS + 0x31])], regions, conv='t', ret='a')
    if op == 'UninitCopy':   # 0-3 strings from x on copied to DEST (or onto themselves); a null dest only for none or one (skipped)
        k = r.choice([0, 1, 1, 2, 3, 3])
        for j in (2, 3):
            put(strs, 0x10 * j + 4, '3I', *string(r, blocks, j))
        dest = r.choice([DEST, DEST, DEST, X] + ([0, 0] if k < 2 else []))
        regions[DEST] = r.bytes(0x40)
        return Call(0x47b600, [r.getrandbits(32), X, X + 0x10 * k, dest], regions, conv='t', ret='a')
    if op == 'Construct':   # (a null p does nothing)
        return Call(0x47d9d0, [r.choice([S, S, S, 0]), r.choice([X, X, S])], regions, ret='v')
    if op in ('CopyRange', 'VecErase'):   # four strings from s on; CopyRange: some of x's on to DEST, down over s's or onto themselves
        for j in (2, 3):
            put(strs, 0x10 * j + 4, '3I', *string(r, blocks, j))
        regions[DEST] = r.bytes(0x40)
        if op == 'CopyRange':
            k = r.choice([0, 1, 2, 3, 3])
            regions[DEST] = b''.join(struct.pack('<4I', r.getrandbits(32), *string(r, blocks, 4 + j)) for j in range(4))
            return Call(0x47d620, [X, X + 0x10 * k, r.choice([DEST, DEST, S, X])], regions, ret='a')
        # VecErase: a vector of the strings from s up to 0-4 of them (its end past that or not); [first, last) within them
        m = r.choice([0, 1, 2, 3, 4, 4])
        a = r.randrange(m + 1)
        b = r.choice([a, r.randrange(a, m + 1), m])
        regions[DEST] = put(bytearray(r.bytes(0x10)), 4, '3I', S, S + 0x10 * m, S + 0x10 * r.choice([m, 4]))
        return Call(0x47b320, [DEST, S + 0x10 * a, S + 0x10 * b], regions, conv='t', ret='a')
    if op == 'Erase':   # (pos not past the end: _Xran ends the case)
        pos = r.choice([0, s[1], r.randrange(s[1] + 1)])
        count = r.choice([0xffffffff, r.randrange(50), 0, 1, s[1] - pos, max(0, s[1] - pos - 1)]) & 0xffffffff
        return Call(0x47b460, [S, pos, count], regions, conv='t', ret='a')
    # AssignSub / Append: from x, or from s itself
    src, (_, n, _) = r.choice([(X, x), (X, x), (S, s)])
    pos = r.choice([0, n, r.randrange(n + 1)])  # (not past the end: _Xran ends the case)
    count = r.choice([0xffffffff, r.randrange(50), 0, n - pos, max(0, n - pos - 1)]) & 0xffffffff
    return Call(0x47cc10 if op == 'Append' else 0x47b750, [S, src, pos, count], regions, conv='t', ret='a')


KINDS = ['Grow', 'Copy', 'Split', 'Freeze', 'Fill', 'AssignSub', 'AssignSub', 'AssignSub', 'Entry', 'CtorCopy', 'CtorFill', 'UninitCopy',
         'Append', 'Append', 'Erase', 'Construct', 'CopyRange', 'VecErase', 'CtorChars']
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The game's MSVC6 std::string members (0x45b340-0x45b998) and their users (the online
# servers' settings 0x45aee0) against the originals; the copy constructor 0x4a9420 from x or s itself (npos 0x6fe5ac now and then small).
# usage: stdstring.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x45b340, 0x45b9a0), (0x45aee0, 0x45b1e4), (0x47b1c0, 0x47b2f6), (0x4a9420, 0x4a953c)])
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


def online(r):
    """Online_ApplyBalance: gBalanceOnline (ports, then seven C strings in CHARS or in a server
    string's block) into the seven strings at 0x7a14f0 (blocks in BLOCKS, some shared)."""
    blocks = bytearray(r.bytes(8 * SLOT))
    servers = bytearray(r.bytes(0x7c))
    made = []
    for k, at in enumerate((0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x6c)):
        x = string(r, blocks, k)
        if made and r.random() < 0.15:
            x = r.choice(made)
            if x[0]:
                blocks[x[0] - BLOCKS - 1] = r.choice([1, 2, 0xfd, 0xfe])
        made.append(x)
        put(servers, at + 4, '3I', *x)
    chars = bytearray(r.bytes(0x100))
    names = []
    for k in range(7):
        n = r.choice([0, 1, r.randrange(20)])
        chars[0x20 * k:0x20 * k + n] = bytes(r.randrange(1, 256) for _ in range(n))
        chars[0x20 * k + n] = 0
        own = [x for x in made if x[0] and 0 in blocks[x[0] - BLOCKS:x[0] - BLOCKS + 0x7f]]
        names.append(r.choice(own)[0] if own and r.random() < 0.1 else CHARS + 0x20 * k)
    bal = bytearray(r.bytes(0x40))
    put(bal, 0xc, '13I', *([r.getrandbits(32) for _ in range(6)] + names))
    return Call(0x45aee0, [], {BLOCKS: blocks, CHARS: chars, HEAP: heap(), 0x785828: bal, 0x7a14f0: servers})


def case(r):
    op = r.choice(OPS)
    if op == 'Online':
        return online(r)
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
    if op == 'Fill':   # (not npos or past max_size: the throws end the case)
        return Call(0x47b1c0, [S, small, r.choice([0, r.getrandbits(8), r.getrandbits(32)])], regions, conv='t', ret='a')
    if op == 'Freeze':
        return Call(0x47b280, [S], regions, conv='t')
    if op == 'CopyCtor':
        if r.random() < 0.2:   # npos (0x6fe5ac) small: the copy cut short
            regions[0x6fe5ac] = put(bytearray(4), 0, 'I', r.choice([0, 1, 5, 0x20]))
        return Call(0x4a9420, [S, r.choice([X, X, X, S])], regions, conv='t', ret='a')
    if op == 'Split':
        return Call(0x45b8d0, [S], regions, conv='t')
    # AssignSub: from x, or from s itself
    src, (_, n, _) = r.choice([(X, x), (X, x), (S, s)])
    pos = r.choice([0, n, r.randrange(n + 1)])
    count = r.choice([0xffffffff, r.randrange(50), 0, n - pos, max(0, n - pos - 1)])
    return Call(0x45b380, [S, src, pos, count], regions, conv='t', ret='a')


KINDS = ['Dtor', 'Tidy', 'AssignChars', 'Eos', 'Grow', 'MaxSize', 'Copy', 'Split', 'AssignSub', 'AssignSub', 'Online', 'Fill', 'Freeze', 'CopyCtor']
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

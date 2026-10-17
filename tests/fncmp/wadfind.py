# The WAD directory's finder (src/game/files/wadfind) against the originals: trees of folders and files named as WadDir_Load leaves them (lower case, '\'
# separated; now and then an upper-case, empty or high-byte name), patterns over them ('\' or '/', a leading or trailing separator, upper case, the folder part missing
# or wrong, the last part exact, empty or with '*' / '?' anywhere, ".*" endings), the files' name hashes now and then the pattern's, so that an exact pattern
# finds them; token lists for the matcher built by hand too (tokens the compiler never makes among them), finders mid-walk for Next / Done / Fill. toupper is
# the CRT's in the C locale (0xfb1440 0; driver.cpp's WadFind env has an ASCII stand-in). The arena and list builders are waddir.py's.
# usage: wadfind.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Start,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, Bench, Call, RAM, heap  # noqa: E402
from waddir import ARENA, LOCALE, Arena, plist  # noqa: E402

B = Bench([(0x637460, 0x638840), (0x641500, 0x641900), (0x67a400, 0x690000), (0x5f5f00, 0x5f9000), (0x405000, 0x406000), (0x5e4000, 0x5e5000)])
THIS, RESULT, ENTRY = RAM, RAM + 0x100, RAM + 0x200
STRING_VTABLE = 0x703090   # (the driver points its slots at the replacements)
PARTS = [b'data', b'rides', b'coaster', b'x.wad', b'car.bin', b'track.bin', b'a', b'ab', b'readme.txt', b'abc.d', b'sfx', b'q', b'', b'Rides',
         b'c.bin.bin', b'aa.a', b'caf\xe9', b'\xe9t\xe9', b'n\xffa.b']
WILD = [b'*', b'*.*', b'*.bin', b'c?r.bin', b'r*', b'*?x', b'**', b'?', b'', b'a*b*c', b'*a', b'?*.d', b'*.', b'.*', b'x*.*', b'a?', b'*??', b'track*', b'?*?', b'a*b*?c', b'?*?.bin', b'*a*?*b']
# name: (address, this: d a WadDir, f a folder, n a finder, s a token list, x space for one, - none; stack arguments; conv; returns)
OPS = {
    'NodeCtor': (0x5f8a50, 'x', 'V', 't', 'a'), 'PopHead': (0x641820, 's', 'O', 't', 'a'), 'Compile': (0x641500, 's', 'RP', 't', 'a'),
    'Match': (0x641760, 's', 'T', 't', 'a'), 'MatchFrom': (0x641780, '-', 'TL', 's', 'a'), 'FolderFind': (0x637e10, 'f', 'QO', 't', 'a'),
    'FinderCtor': (0x638050, 'x', 'GSr', 't', 'a'), 'Start': (0x6380f0, 'n', 'GSr', 't', 'v'), 'Next': (0x638430, 'N', '', 't', 'v'),
    'Done': (0x638550, 'N', '', 't', 'a'), 'Fill': (0x638570, 'N', 'E', 't', 'v'), 'FindFirst': (0x637a50, 'd', 'RES', 't', 'a'),
    'FindNext': (0x637aa0, 'D', 'RE', 't', 'a'),
}


def toupper(c):   # the CRT's in the C locale, on a sign-extended char
    c = c - 0x100 if c >= 0x80 else c
    return c - 0x20 if 0x61 <= c <= 0x7a else c


def name_hash(s):
    h, n = 0, len(s)
    for g in range(n >> 2):
        h += sum(toupper(s[4 * g + i]) << i for i in range(4))
    for c in s[n & ~3:]:
        h += toupper(c) & 0xff
    return h & 0xffffffff


def part(r):
    s = r.choice(PARTS)
    return s.upper() if r.random() < 0.05 else s


def string(a, r, text, at=None):
    """A String (vtable, length, data, capacity, grow) holding text; its block in the arena (Mem_Free only records it). The game's String vtable: Start
    assigns through it, and the originals call their own slots."""
    at = at or a.alloc(0x14)
    a.put(at, '5I', STRING_VTABLE, len(text), a.text(text), len(text) + r.choice([0, 0, 8]), r.choice([0, 8]))
    return at


def tree(a, r, depth, hashes):
    """A folder (name, owned, folders, files) of 0-3 subfolders and 0-4 files; each file's hash random or one of hashes."""
    at = a.alloc(0x28)
    a.put(at, 'IB', a.text(part(r)), r.choice([0, 1]))
    subs = [tree(a, r, depth - 1, hashes) for _ in range(r.choice([0, 1, 2, 3]))] if depth > 0 else []
    files = []
    for _ in range(r.choice([0, 1, 2, 4])):
        f = a.alloc(0x14)
        a.put(f, 'IBxxxIII', a.text(part(r)), r.choice([0, 1]), r.getrandbits(32), r.getrandbits(32),
              r.choice(hashes) if hashes and r.random() < 0.6 else r.getrandbits(32))
        files.append(f)
    plist(a, r, at + 8, subs)
    plist(a, r, at + 0x18, files)
    return at


def pattern(r):
    """Pattern text: 0-3 folder parts and a last part, separated by '\\' or '/', perhaps a separator either end, perhaps upper case."""
    sep = lambda: r.choice([b'\\', b'\\', b'/'])
    s = b''.join(part(r) + sep() for _ in range(r.choice([0, 0, 1, 1, 2, 3])))
    s += r.choice([part(r), r.choice(WILD), r.choice(WILD)])
    if r.random() < 0.15:
        s = sep() + s
    if r.random() < 0.15:
        s += sep()
    return s.upper() if r.random() < 0.1 else s


def normalized(s):
    """WadFinder_Start's search text: one leading and one trailing separator dropped, '/' to '\\', lower case."""
    if s[-1:] in (b'\\', b'/'):
        s = s[:-1]
    if s[:1] in (b'\\', b'/'):
        s = s[1:]
    return s.replace(b'/', b'\\').lower()


def tokens(a, r, at, toks):
    """A StringList (vtable, head, tail, count) of Strings with the token texts."""
    ns = [a.alloc(0x1c) for _ in toks]
    for i, (n, t) in enumerate(zip(ns, toks)):
        string(a, r, t, n)
        a.put(n + 0x14, 'II', ns[i + 1] if i + 1 < len(ns) else 0, ns[i - 1] if i else 0)
    a.put(at, '4I', r.getrandbits(32), ns[0] if ns else 0, ns[-1] if ns else 0, len(ns) if r.random() < 0.9 else r.choice([0, 1, 7]))
    return ns[0] if ns else 0


def compiled(s):
    """Wildcard_Compile's tokens for s (the text before the first wildcard, then each wildcard with the text after it; wildcards after a '*' dropped)."""
    toks, i = [], 0
    while i < len(s) and s[i] not in b'*?':
        i += 1
    if i:
        toks.append(s[:i])
    if i < len(s):
        tok, i = s[i:i + 1], i + 1
        while i < len(s):
            if s[i] not in b'*?':
                tok, i = tok + s[i:i + 1], i + 1
                continue
            toks.append(tok)
            tok = s[i:i + 1]
            if s[i] == 0x2a:
                while i + 1 < len(s) and s[i + 1] in b'*?':
                    i += 1
            i += 1
        toks.append(tok)
    return toks


def some_tokens(r):
    if r.random() < 0.6:
        return compiled(r.choice(WILD + [part(r)]))
    return [r.choice([b'*', b'?', b'*a', b'?b', b'ab', b'', b'*.bin', b'car', b'.d', b'?']) for _ in range(r.choice([0, 1, 2, 3, 5]))]


def text(r):
    return r.choice([part(r), part(r) + part(r), r.choice(WILD), b''])


def finder(a, r, hashes, at=None):
    """A finder mid-walk: pattern, folder, cursor, current, path, folders, exact, hash, the two token lists, dotStar."""
    at = at or a.alloc(0x60)
    root = tree(a, r, 1, hashes) if r.random() < 0.9 else 0
    pat = r.choice(WILD + [part(r)])
    string(a, r, pat, at)
    nodes = []
    if root:
        for off in (8, 0x18):
            head = struct.unpack_from('<I', a.data, root + off + 4 - ARENA)[0]
            while head:
                nodes.append(head)
                head = struct.unpack_from('<I', a.data, head + 4 - ARENA)[0]
    pick = lambda: r.choice(nodes + [0]) if nodes and r.random() < 0.8 else r.choice([0, r.getrandbits(32) & ~0xff | 0x80000000, 0x80000001])
    a.put(at + 0x14, '3I', root, pick() if root else 0, pick() if root else 0)
    string(a, r, r.choice([b'', b'data\\', b'rides\\coaster\\', b'a\\b\\c\\']), at + 0x20)
    exact = r.choice([0, 0, 1, 0x80])
    a.put(at + 0x34, 'BBxxI', r.choice([0, 1, 1, 0x80]), exact, name_hash(pat) if r.random() < 0.5 else r.choice(hashes or [0]))
    tokens(a, r, at + 0x3c, compiled(pat) if r.random() < 0.8 else some_tokens(r))
    tokens(a, r, at + 0x4c, compiled(pat[:-2]) if r.random() < 0.8 else some_tokens(r))
    a.put(at + 0x5c, 'B', r.choice([0, 0, 1, 0x40]))
    return at


def case(r):
    op = r.choice(PICK)
    address, kind, args, conv, ret = OPS[op]
    a = Arena(r)
    pat = pattern(r)
    hashes = [name_hash(normalized(pat)), name_hash(pat)]
    regions = {HEAP: heap(0x10000), LOCALE: bytes(4), RESULT: r.bytes(0x100), ENTRY: r.bytes(0x200)}
    this = 0
    if kind in 'dD':   # root, names, finder
        this = a.alloc(0x1c)
        a.put(this, '3I', tree(a, r, 2, hashes) if r.random() < 0.9 else 0, 0,
              finder(a, r, hashes) if kind == 'D' or r.random() < 0.95 else 0)
    elif kind == 'f':
        this = tree(a, r, 3, hashes)
    elif kind == 'n':
        this = finder(a, r, hashes) if r.random() < 0.5 else a.alloc(0x60)
    elif kind == 'N':
        this = finder(a, r, hashes)
    elif kind == 's':
        this = a.alloc(0x10)
        tokens(a, r, this, some_tokens(r))
    elif kind == 'x':
        this = a.alloc(0x60)
    stack = []
    for x in args:
        if x == 'V':   # a String by value
            s = string(a, r, text(r))
            stack += list(struct.unpack_from('<5I', a.data, s - ARENA))
        elif x == 'O':   # an out String: for Find, the path so far
            stack.append(string(a, r, r.choice([b'', b'data\\', b'x'])) if op == 'FolderFind' else RESULT)
        elif x == 'R':
            stack.append(RESULT)
        elif x == 'P':
            stack.append(a.text(pat if r.random() < 0.5 else r.choice(WILD)))
        elif x == 'T':   # a text to match
            stack.append(a.text(text(r) + r.choice([b'', b'.bin', b'x', b'a']) + r.choice([b'', b'\0'])))   # (\0: the byte past the end 0 too)
        elif x == 'L':   # a token list's first node
            stack.append(tokens(a, r, a.alloc(0x10), some_tokens(r)))
        elif x == 'Q':   # a folder path: parts of the tree's names
            stack.append(string(a, r, b'\\'.join(part(r) for _ in range(r.choice([1, 1, 2, 3]))) + r.choice([b'', b'', b'\\', b'/'])))
        elif x == 'G':   # &root
            g = a.alloc(4)
            a.put(g, 'I', tree(a, r, 2, hashes) if r.random() < 0.9 else 0)
            stack.append(g)
        elif x == 'S':
            stack.append(string(a, r, pat))
        elif x == 'r':
            stack.append(r.choice([0, RESULT + 0xf0]))
        elif x == 'E':
            stack.append(ENTRY)
    regions[ARENA] = a.data
    return Call(address, ([this] if kind != '-' else []) + stack, regions, conv=conv, ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

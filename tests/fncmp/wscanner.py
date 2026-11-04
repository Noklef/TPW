# The wide text scanner's text setter (src/game/core/wscanner: 0x5e6b30) against the original: its wstring at +4 and the text passed by
# value made as stdwstring.py makes strings (null, empty, short and long, count bytes 0-2, 0xfd-0xff, sometimes the same block), any
# flag bytes and context; the copies and frees from the heap region. Pair (0x5e73f0): the keyword pair's constructor on random bytes, the
# key made the same way, the value random. Token (0x5e57c0, 0x5e6070, 0x5e6540): the scanner random bytes, its vector [+0x50] of CountedPairs
# as countedpair.py builds one for an insert (0-4 pairs, room for up to 4 more or none, no block at all; now and then the new block not got).
# Span (0x5e6fc0): the scanner's set at +0x14 a valid red-black tree of stdtree.py (1-11 word keys of 0-99), its text as stdwstring.py makes
# one with characters mostly in the set (else 0, 7, 99, 100, 0xffff, any), the position at 0, inside, at or past the length.
# Next (0x5e70a0), Rest (0x5e7200): Span's scanner with a second set at +0x24 and a string to assign to (a null count now and then); Substr (0x5e7520): any part of a string (a position past its length throws, not run), from an out that may be the string itself.
# Base (0x5e6820): random bytes, the node nil made or not, the map counter any; the five allocator bytes (the stack's garbage) left out.
# Compare (0x5e6750): two strings of letters, the second made from the first (same, other case, cut, longer, one changed), towupper the original's C-locale branch against the
# driver's. Char (0x5e7320): Span's scanner. Find (0x5e6e50): a list of 0-6 keywords, the text one of them (other case, cut, longer) or none. Read (0x5e6c90): Rest's scanner, its
# words the keys, the handlers recorders answering mostly 1. Parse (0x5e45d0): Rest's scanner with its vector of tokens [+0x50] (0-2 random-byte tokens, room for 4 more) and its out string [+0x54], the list inside it, a text with or without its '/' and words.
# usage: wscanner.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetText,Pair,Token,Span,Next,Rest,Substr,Base,Destruct,Compare,Char,Find,Read,Parse)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, recorder, set_return  # noqa: E402
import countedpair  # noqa: E402
import stdtree  # noqa: E402
import stdwstring  # noqa: E402

# (with stdwstring.py's helpers: _Grow, erase, the counted buffer reset, the traits' copy, the length setter)
B = Bench([(0x4051d0, 0x405390), (0x45b380, 0x45b5f0), (0x45b680, 0x45b9b0), (0x479150, 0x4791b0), (0x479230, 0x479270), (0x47ae60, 0x47b060),
           (0x47b320, 0x47b5b0), (0x47b600, 0x47bcf0), (0x47cc10, 0x47cd00), (0x47d620, 0x47d780), (0x47d9d0, 0x47db00), (0x4e51a0, 0x4e5220),
           (0x4e6410, 0x4e6570), (0x4ea740, 0x4ea7f0), (0x4f0150, 0x4f0180), (0x4f23f0, 0x4f2450), (0x4f34a0, 0x4f3730), (0x4f5560, 0x4f59a0),
           (0x4f6080, 0x4f60a0), (0x4f67b0, 0x4f6830), (0x4f6ef0, 0x4f7180), (0x4f7410, 0x4f7440), (0x4f7870, 0x4f7970), (0x5b1650, 0x5b1680), (0x5b1930, 0x5b1b80), (0x5b2530, 0x5b2f40), (0x5e6a00, 0x5e6b2b), (0x5b30b0, 0x5b31f0), (0x5b3210, 0x5b3230), (0x5e45d0, 0x5e47f0), (0x5e4b20, 0x5e6730), (0x5e6750, 0x5e6a00),
           (0x5e6b30, 0x5e7518), (0x5e7520, 0x5e7740), (0x67c8d0, 0x67c9f0), (0x67ed00, 0x67ed90), (0x67acd0, 0x67ad00), (0x6918f0, 0x691a30),
           (0x691ba0, 0x691ce0)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})   # (_Lockit's constructor and destructor)
SCANNER, FLAGS = RAM + 0x800, RAM + 0x900
OPS = [k for k in os.environ.get('OPS', 'SetText,Pair,Token,Span,Next,Rest,Substr,Base,Destruct,Compare,Char,Find,Read,Parse').split(',') if k]
TOKENS = [0x5e57c0, 0x5e6070, 0x5e6540, 0x5e6630]
OUT, HEAD2, NODES2 = RAM + 0x980, RAM + 0xa00, RAM + 0x2000
VECTOR, TOKEN_ARRAY = RAM + 0xb00, RAM + 0xc00
CTX, LIST, KEYS, STR_A, STR_B = RAM + 0x3000, RAM + 0x3100, RAM + 0x3400, RAM + 0x4000, RAM + 0x4400
HANDLER = 16   # (recorders 16-19)
LETTERS = [0x61, 0x41, 0x62, 0x42, 0x7a, 0x5a, 0x60, 0x7b, 0x40, 0x5b, 0xe9, 0xc9, 0x1234]


def base(r):
    nil = r.choice([0, RAM + 0xd00])
    regions = {SCANNER: r.bytes(0x40), RAM + 0xd00: put(put(bytearray(r.bytes(0x14)), 0, 'I', 0), 8, 'I', 0), 0xf7c96c: struct.pack('<II', r.choice([0, 1, r.getrandbits(32)]), nil), HEAP: heap()}
    put(regions[RAM + 0xd00], 0x10, 'I', 1)
    return Call(0x5e6820, [SCANNER], regions, conv='t', ret='a', ignore=[(SCANNER + 4, 1), (SCANNER + 0x14, 2), (SCANNER + 0x24, 2)])


def destruct(r):
    """Base's destructor (0x5e6a00): two sets of word keys, their counts now and then 0 (erased node by node then), the map counter 1 (the nil freed) or more, the string null or with any count byte."""
    _keys, head, blob = tree(r, stdtree.HEAD, stdtree.NODES)
    _keys2, head2, blob2 = tree(r, HEAD2, NODES2)
    blocks = bytearray(r.bytes(4 * stdwstring.SLOT))
    ptr_, n, res = stdwstring.string(r, blocks, 0)
    scanner = bytearray(r.bytes(0x50))
    put(scanner, 8, '3I', ptr_, n, res)
    for at, address in ((0x14, stdtree.HEAD), (0x24, HEAD2)):
        put(scanner, at, '2I', r.getrandbits(32), address)
        put(scanner, at + 0xc, 'I', r.choice([0, 1, 5, 5, 5, 5]))
    regions = {SCANNER: scanner, stdwstring.BLOCKS: blocks, HEAP: heap(), stdtree.HEAD: head, HEAD2: head2, stdtree.NODES: blob, NODES2: blob2,
               stdtree.NIL: put(bytearray(0x40), 0x10, 'I', stdtree.BLACK), 0xf7c96c: struct.pack('<2I', r.choice([1, 2, 2, 5]), stdtree.NIL)}
    return Call(0x5e6a00, [SCANNER], regions, conv='t', ret='v')


def wide(chars):
    return b''.join(struct.pack('<H', c) for c in chars)


def variant(r, chars):
    """Chars the same, in the other case, cut, longer or with one changed."""
    kind = r.choice(['same', 'case', 'case', 'cut', 'long', 'one', 'any'])
    out = list(chars)
    if kind == 'case':
        out = [c ^ 0x20 if 0x41 <= c <= 0x5a or 0x61 <= c <= 0x7a else c for c in out]
    elif kind == 'cut':
        out = out[:r.randrange(len(out) + 1)]
    elif kind == 'long':
        out = out + [r.choice(LETTERS) for _ in range(r.choice([1, 3]))]
    elif kind == 'one' and out:
        out[r.randrange(len(out))] = r.choice(LETTERS)
    elif kind == 'any':
        out = [r.choice(LETTERS) for _ in range(r.randrange(6))]
    return out


def compare(r):
    a = [r.choice(LETTERS) for _ in range(r.choice([0, 1, 3, 8]))]
    b = variant(r, a)
    na, nb = 0 if r.random() < 0.05 else STR_A + 0x10, 0 if r.random() < 0.05 else STR_B + 0x10
    regions = {STR_A: struct.pack('<4I', r.getrandbits(32), na, len(a), r.getrandbits(32)) + wide(a) + b'\xff\xff' * 4,
               STR_B: struct.pack('<4I', r.getrandbits(32), nb, len(b), r.getrandbits(32)) + wide(b) + b'\xff\xff' * 4, 0xfb1440: bytes(4)}
    return Call(0x5e6750, [STR_A, STR_B, r.choice([0, 0, 1, 1, 0x100, r.getrandbits(32)])], regions, ret='a')


def keyword_list(r, regions, words):
    """A circular list of 0-6 keywords at LIST: the keys variants of the words, the handlers recorders answering mostly 1."""
    n = r.choice([0, 1, 2, 4, 6])
    log = log_region()
    for k in range(4):
        set_return(log, HANDLER + k, r.choice([1, 1, 1, 0x101, 0, 0xffffff01]))
    nodes = bytearray(r.bytes(0x28 * (n + 1)))
    text = bytearray()
    put(nodes, 0, '2I', LIST + 0x28 if n else LIST, LIST + 0x28 * n)
    for i in range(1, n + 1):
        key = variant(r, list(r.choice(words))) if words and r.random() < 0.8 else [r.choice(LETTERS) for _ in range(r.randrange(5))]
        at = LIST + 0x28 * i
        put(nodes, 0x28 * i, '2I', at + 0x28 if i < n else LIST, at - 0x28)
        put(nodes, 0x28 * i + 8, '4I', r.getrandbits(32), KEYS + len(text) + 2, len(key), len(key))
        put(nodes, 0x28 * i + 0x18, '2I', recorder(HANDLER + i % 4, 0), r.choice([0, 0, 4, 0x20]))
        text += b'\xff\xff' + wide(key) + bytes(2)
    regions.update({LIST: nodes, KEYS: bytes(text) or bytes(2), LOG: log, 0xfb1440: bytes(4)})


def find(r):
    blocks = bytearray(r.bytes(4 * stdwstring.SLOT))
    chars = [r.choice(LETTERS + [0, 0]) for _ in range(r.choice([0, 1, 3, 8]))]   # (a NUL: _wcsnicmp's compare stops there, the other does not)
    ptr = stdwstring.BLOCKS + 2
    blocks[1] = r.choice([0, 1, 2, 0xfd, 0xfe, 0xff])
    blocks[2:4 + 2 * len(chars)] = wide(chars) + bytes(2)
    x = (0, 0, 0) if r.random() < 0.1 else (ptr, len(chars), len(chars))
    regions = {SCANNER: put(bytearray(r.bytes(0x50)), 0x40, 'I', CTX), CTX: struct.pack('<II', r.getrandbits(32), LIST), stdwstring.BLOCKS: blocks, HEAP: heap()}
    put(regions[SCANNER], 0x3d, 'B', r.choice([0, 0, 1, 0xff]))
    put(regions[SCANNER], 0x3f, 'B', r.choice([0, 0, 1, 0xff]))
    keyword_list(r, regions, [chars])
    return Call(0x5e6e50, [SCANNER, r.getrandbits(32), *x], regions, conv='t', ret='a')


def words_of(c, stops):
    """The runs of characters between those in stops, in Rest's scanner's text."""
    blocks = c.regions[stdwstring.BLOCKS]
    ptr, n = struct.unpack_from('<II', c.regions[SCANNER], 8)
    chars = struct.unpack_from('<%dH' % n, blocks, ptr - stdwstring.BLOCKS) if ptr else ()
    words, word = [], []
    for ch in chars:
        if ch in stops:
            words, word = words + [word] if word else words, []
        else:
            word.append(ch)
    return words + [word] if word else words


def read(r):
    stops = []
    c = span(r, 'Rest', stops)
    s = c.regions[SCANNER]
    put(s, 0x3d, 'B', r.choice([0, 0, 1]))
    put(s, 0x3e, 'B', r.choice([0, 0, 0, 1]))
    put(s, 0x3f, 'B', r.choice([0, 0, 1]))
    put(s, 0x40, 'I', CTX)
    c.regions[CTX] = struct.pack('<II', r.getrandbits(32), LIST)
    keyword_list(r, c.regions, words_of(c, stops))
    return Call(0x5e6c90, [SCANNER], c.regions, conv='t', ret='a')


def char(r):
    c = span(r, 'Span')
    c.regions[OUT] = r.bytes(4)
    return Call(0x5e7320, [SCANNER, OUT], c.regions, conv='t', ret='b')


def parse(r):
    stops = []
    c = span(r, 'Rest', stops)
    s = c.regions[SCANNER]
    put(s, 0x38, 'I', r.getrandbits(32))
    head = [r.choice(stops) for _ in range(r.choice([0, 0, 1, 3]))]
    words = [[r.choice(LETTERS) for _ in range(r.choice([1, 2, 4]))] for _ in range(r.choice([0, 1, 2, 3]))]
    body = []
    for w in words:
        body += w + [r.choice(stops)]
    chars = head + [0x2f] * (r.random() < 0.8) + body[:0x70]
    blocks = c.regions[stdwstring.BLOCKS]
    at = stdwstring.SLOT
    blocks[at + 1] = r.choice([0, 1, 2, 0xfd, 0xfe, 0xff])
    blocks[at + 2:at + 4 + 2 * len(chars)] = wide(chars) + bytes(2)
    held = r.choice([0, 0, 1, 2])
    last = TOKEN_ARRAY + 0x24 * held
    s += struct.pack('<II', VECTOR, OUT)
    c.regions[VECTOR] = struct.pack('<IIII', r.getrandbits(32), TOKEN_ARRAY, last, last + 0x24 * 4)
    c.regions[TOKEN_ARRAY] = r.bytes(0x24 * (held + 4))
    regions = c.regions
    put(s, 0x48, 'I', LIST)
    keyword_list(r, regions, words)
    ignore = [(last + 0x24 * i + k, 1) for i in range(3) for k in (0, 0x10)]
    return Call(0x5e45d0, [SCANNER, r.getrandbits(32), stdwstring.BLOCKS + at + 2, len(chars), max(len(chars), r.choice([0, 31, 0x7d]))], regions, conv='t', ret='v', ignore=ignore)


def pair(r):
    blocks = bytearray(r.bytes(4 * stdwstring.SLOT))
    x = stdwstring.string(r, blocks, 1)
    pair = bytearray(r.bytes(0x20))
    if r.random() < 0.5:   # (an old capacity of 31 or under: _Grow keeps a null string's capacity unless it is cleared first)
        put(pair, 0xc, 'I', r.choice([0, 1, 31, r.randrange(32)]))
    regions = {SCANNER: pair, stdwstring.BLOCKS: blocks, HEAP: heap()}
    return Call(0x5e73f0, [SCANNER, r.getrandbits(32), *x, *(r.getrandbits(32) for _ in range(4))], regions, conv='t', ret='a')


def token(r):
    """The pair goes at the vector's end (or at the same index of its new block, the heap's first); its strings' allocator bytes, the low
    byte of the stack pair's address (CountedPair_CopyCtor, sic), are left out."""
    regions = countedpair.insert_case(r).regions
    regions[SCANNER] = put(bytearray(r.bytes(0x60)), 0x50, 'I', countedpair.V)
    first, last, end = struct.unpack_from('<3I', regions[countedpair.V], 4)
    at = last if first and last < end else HEAP + 0x10 + (last - first)
    return Call(TOKENS[r.deal(list(range(len(TOKENS))))], [SCANNER], regions, conv='t', ret='a', ignore=[(at, 1), (at + 0x10, 1)])


def tree(r, head_at, nodes_at):
    """A set of word keys of stdtree.py at nodes_at, its head at head_at: (keys, head's bytes, nodes' bytes)."""
    nodes, root = stdtree.build(r, r.choice([1, 2, 3, 5, 8, 11]))
    keys = sorted(nodes)
    addr = {k: nodes_at + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: stdtree.NIL if k is None else addr[k]  # noqa: E731
    blob = bytearray(0x40 * len(keys))
    for k in keys:
        base = addr[k] - nodes_at
        struct.pack_into('<III', blob, base, ptr(nodes[k][0]), head_at if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        struct.pack_into('<II', blob, base + 0xc, k | r.getrandbits(16) << 16, nodes[k][3])
    head = struct.pack('<III', addr[keys[0]], addr[root], addr[keys[-1]]) + bytes(0x20)
    return keys, head, blob


def span(r, op='Span', stops=None):
    keys, head, blob = tree(r, stdtree.HEAD, stdtree.NODES)
    while op == 'Rest' and 0 in keys:   # (the skip must stop at the end: past it the rest's substr throws, _Xran)
        keys, head, blob = tree(r, stdtree.HEAD, stdtree.NODES)
    if stops is not None:
        stops.extend(keys)
    nil = put(bytearray(0x40), 0x10, 'I', stdtree.BLACK)
    blocks = bytearray(r.bytes(4 * stdwstring.SLOT))
    ptr_, n, res = stdwstring.string(r, blocks, 0)
    inside = r.choice([0.5, 0.9, 1.0])
    for i in range(n):
        if r.random() < 0.97 and int.from_bytes(blocks[2 + 2 * i:4 + 2 * i], 'little'):
            c = r.choice(keys) if r.random() < inside else r.choice([0, 7, 99, 100, 0xffff, r.getrandbits(16)])
            blocks[2 + 2 * i:4 + 2 * i] = struct.pack('<H', c)
    if op == 'Rest' and ptr_:
        blocks[2 + 2 * n:4 + 2 * n] = b'\0\0'
    scanner = put(bytearray(r.bytes(0x50)), 8, '3I', ptr_, n, res)
    put(scanner, 0x14, '2I', r.getrandbits(32), stdtree.HEAD)
    # (past the length only for Span: the others' substr throws, _Xran)
    put(scanner, 0x38, 'I', r.choice([0, 0, r.randrange(n + 1), r.randrange(n + 1), n] + ([n + 1, n + 9] if op == 'Span' else [])))
    regions = {SCANNER: scanner, stdwstring.BLOCKS: blocks, HEAP: heap(), stdtree.HEAD: head, stdtree.NIL: nil, stdtree.NODES: blob,
               0xf7c970: struct.pack('<I', stdtree.NIL)}
    if op == 'Span':
        return Call(0x5e6fc0, [SCANNER], regions, conv='t', ret='a')
    stop, head2, blob2 = tree(r, HEAD2, NODES2)
    regions[HEAD2], regions[NODES2] = head2, blob2
    put(scanner, 0x24, '2I', r.getrandbits(32), HEAD2)
    for i in range(n):
        if r.random() < 0.15 and int.from_bytes(blocks[2 + 2 * i:4 + 2 * i], 'little'):
            blocks[2 + 2 * i:4 + 2 * i] = struct.pack('<H', r.choice(stop))
    out = stdwstring.string(r, blocks, 1)
    regions[OUT] = put(bytearray(r.bytes(0x18)), 4, '3I', *out)
    if op == 'Next':
        return Call(0x5e70a0, [SCANNER, OUT], regions, conv='t', ret='b')
    return Call(0x5e7200, [SCANNER, OUT, r.choice([OUT + 0x10, OUT + 0x10, 0])], regions, conv='t', ret='b')


def substr(r):
    blocks = bytearray(r.bytes(4 * stdwstring.SLOT))
    x = stdwstring.string(r, blocks, 0)
    regions = {SCANNER: put(bytearray(r.bytes(0x10)), 4, '3I', *x), OUT: r.bytes(0x10), stdwstring.BLOCKS: blocks, HEAP: heap()}
    pos, n = r.choice([0, 0, r.randrange(x[1] + 1), x[1]]), r.choice([0, 1, r.randrange(x[1] + 1), x[1], 0xffffffff])
    out = r.choice([OUT, OUT, OUT, SCANNER])   # (the string its own out: emptied first, so only position 0 does not throw)
    return Call(0x5e7520, [SCANNER, out, pos if out == OUT else 0, n], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op in ('Span', 'Next', 'Rest'):
        return span(r, op)
    if op == 'Substr':
        return substr(r)
    if op == 'Pair':
        return pair(r)
    if op == 'Base':
        return base(r)
    if op == 'Destruct':
        return destruct(r)
    if op == 'Compare':
        return compare(r)
    if op == 'Char':
        return char(r)
    if op == 'Find':
        return find(r)
    if op == 'Read':
        return read(r)
    if op == 'Parse':
        return parse(r)
    if op == 'Token':
        return token(r)
    blocks = bytearray(r.bytes(4 * stdwstring.SLOT))
    s = stdwstring.string(r, blocks, 0)
    together = s[0] and r.random() < 0.2
    x = s if together else stdwstring.string(r, blocks, 1)
    if together:
        blocks[s[0] - stdwstring.BLOCKS - 1] = r.choice([1, 2, 0xfd, 0xfe])
    scanner = put(bytearray(r.bytes(0x50)), 8, '3I', *s)
    regions = {SCANNER: scanner, FLAGS: r.bytes(4), stdwstring.BLOCKS: blocks, HEAP: heap()}
    return Call(0x5e6b30, [SCANNER, r.getrandbits(32), *x, r.getrandbits(32), FLAGS], regions, conv='t')


if __name__ == '__main__':
    B.main(case)

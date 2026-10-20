# The wide text scanner's text setter (src/game/core/wscanner: 0x5e6b30) against the original: its wstring at +4 and the text passed by
# value made as stdwstring.py makes strings (null, empty, short and long, count bytes 0-2, 0xfd-0xff, sometimes the same block), any
# flag bytes and context; the copies and frees from the heap region. Pair (0x5e73f0): the keyword pair's constructor on random bytes, the
# key made the same way, the value random. Token (0x5e57c0, 0x5e6070, 0x5e6540): the scanner random bytes, its vector [+0x50] of CountedPairs
# as countedpair.py builds one for an insert (0-4 pairs, room for up to 4 more or none, no block at all; now and then the new block not got).
# Span (0x5e6fc0): the scanner's set at +0x14 a valid red-black tree of stdtree.py (1-11 word keys of 0-99), its text as stdwstring.py makes
# one with characters mostly in the set (else 0, 7, 99, 100, 0xffff, any), the position at 0, inside, at or past the length.
# usage: wscanner.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetText,Pair,Token,Span)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402
import countedpair  # noqa: E402
import stdtree  # noqa: E402
import stdwstring  # noqa: E402

# (with stdwstring.py's helpers: _Grow, erase, the counted buffer reset, the traits' copy, the length setter)
B = Bench([(0x405280, 0x405390), (0x479150, 0x4791b0), (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0),
           (0x67acd0, 0x67ad00), (0x4ea740, 0x4ea7e0), (0x47ae60, 0x47b060), (0x47b600, 0x47b750), (0x479230, 0x479256),
           (0x47cc10, 0x47cd00), (0x47b460, 0x47b5b0), (0x47d9d0, 0x47db00), (0x47d620, 0x47d780), (0x47b320, 0x47b460),
           (0x5e6b30, 0x5e6c87), (0x5e73f0, 0x5e7518), (0x5e57c0, 0x5e58a6), (0x5e6070, 0x5e6156), (0x5e6540, 0x5e6626), (0x5e7310, 0x5e7319), (0x5e6fc0, 0x5e7093), (0x5b30b0, 0x5b3110),
           (0x45b380, 0x45b5f0), (0x45b680, 0x45b9b0), (0x4e6410, 0x4e6560), (0x4f5860, 0x4f5988), (0x4f5560, 0x4f5852), (0x4e51a0, 0x4e520e),
           (0x4f67b0, 0x4f67e1), (0x4f67f0, 0x4f6819), (0x4f7410, 0x4f7423)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})   # (_Lockit's constructor and destructor)
SCANNER, FLAGS = RAM + 0x800, RAM + 0x900
OPS = [k for k in os.environ.get('OPS', 'SetText,Pair,Token,Span').split(',') if k]
TOKENS = [0x5e57c0, 0x5e6070, 0x5e6540]


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


def span(r):
    nodes, root = stdtree.build(r, r.choice([1, 2, 3, 5, 8, 11]))
    keys = sorted(nodes)
    addr = {k: stdtree.NODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: stdtree.NIL if k is None else addr[k]  # noqa: E731
    blob = bytearray(0x40 * len(keys))
    for k in keys:
        base = addr[k] - stdtree.NODES
        struct.pack_into('<III', blob, base, ptr(nodes[k][0]), stdtree.HEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        struct.pack_into('<II', blob, base + 0xc, k | r.getrandbits(16) << 16, nodes[k][3])
    nil = put(bytearray(0x40), 0x10, 'I', stdtree.BLACK)
    head = struct.pack('<III', addr[keys[0]], addr[root], addr[keys[-1]]) + bytes(0x20)
    blocks = bytearray(r.bytes(4 * stdwstring.SLOT))
    ptr_, n, res = stdwstring.string(r, blocks, 0)
    inside = r.choice([0.5, 0.9, 1.0])
    for i in range(n):
        if r.random() < 0.97 and int.from_bytes(blocks[2 + 2 * i:4 + 2 * i], 'little'):
            c = r.choice(keys) if r.random() < inside else r.choice([0, 7, 99, 100, 0xffff, r.getrandbits(16)])
            blocks[2 + 2 * i:4 + 2 * i] = struct.pack('<H', c)
    scanner = put(bytearray(r.bytes(0x50)), 8, '3I', ptr_, n, res)
    put(scanner, 0x14, '2I', r.getrandbits(32), stdtree.HEAD)
    put(scanner, 0x38, 'I', r.choice([0, 0, r.randrange(n + 1), r.randrange(n + 1), n, n + 1, n + 9]))
    regions = {SCANNER: scanner, stdwstring.BLOCKS: blocks, HEAP: heap(), stdtree.HEAD: head, stdtree.NIL: nil, stdtree.NODES: blob,
               0xf7c970: struct.pack('<I', stdtree.NIL)}
    return Call(0x5e6fc0, [SCANNER], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Span':
        return span(r)
    if op == 'Pair':
        return pair(r)
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

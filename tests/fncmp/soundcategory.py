# A sound category's SFX.map loading (src/game/core/soundcategory) against the originals: the image fixups (0x6c0510 and below: a valid image of
# 1 or 2 categories, 0-3 sounds each, 0-3 variations, 0-3 samples and 0-3 links, built in the order the fixup walks it; the samples' banks 0 or
# within deque C, the weights random, delta-coded or not) over a category object whose deques A (the category records, each with a PtrList of
# 0-3 banks) and C (the bank indices) are small stretches of one or two blocks (the block boundary crossed now and then); a variation's sample
# count (0x6bd7a0) and the simple sound mark (0x6c0550: each of its conditions met or missed).
# usage: soundcategory.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, heap  # noqa: E402

CALLS = {'count': 0x6bd7a0, 'simple': 0x6c0550, 'variation': 0x6c06e0, 'sound': 0x6c0610, 'record': 0x6c05b0, 'fixup': 0x6c0510}
B = Bench([(0x6bd7a0, 0x6bd7ac), (0x6c0310, 0x6c035f), (0x6c0510, 0x6c0844), (0x6b9b30, 0x6b9b94), (0x5e42d0, 0x5e431c), (0x67a400, 0x690000)])
CAT, OUT, RECS, NODES = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400
MAPA, MAPC = RAM + 0x600, RAM + 0x640
BLOCKA, BLOCKC = RAM + 0x10000, RAM + 0x20000   # (two blocks each, 0x1000 apart, only the used stretch mapped)
IMAGE = RAM + 0x3000
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def deque(r, at, mapat, values, regions):
    """A deque's begin iterator (16 bytes) over values starting at a random place in its first block at `at` (into the next, at + 0x1000 or + 0x3000,
    when they run past it); maps the stretch of each block they use and the map's two slots."""
    n = len(values)
    start = max(0, min(0x3ff, r.choice([0, 0x3ff - n // 2, 0x400 - n, r.randrange(0x400 - n + 1)])))
    blocks = [at, at + r.choice([0x1000, 0x3000])]
    regions[mapat] = struct.pack('<2I', *blocks)
    cells = {}
    for i, v in enumerate(values):
        q = start + i
        cells[blocks[q // 0x400] + 4 * (q % 0x400)] = v
    for b in blocks:
        mine = sorted(a for a in cells if b <= a < b + 0x1000)
        if mine:
            buf = bytearray(r.bytes(mine[-1] + 4 - mine[0]))
            for a in mine:
                struct.pack_into('<I', buf, a - mine[0], cells[a])
            regions[mine[0]] = buf
    return struct.pack('<4I', at, at + 0x1000, at + 4 * start, mapat)


def category(r, banks=4):
    """The category object with deques A (records with bank lists) and C (bank indices); returns regions."""
    regions = {HEAP: heap(0x2000)}
    nrec = r.randrange(1, 4)
    recs = bytearray(r.bytes(0x40 * nrec))
    nodes = bytearray(r.bytes(0x10 * 9))
    for i in range(nrec):
        n = r.choice([0, 0, 1, 2, 3])
        vals = r.sample(range(1, 7), n)
        addr = [NODES + 0x10 * (3 * i + j) for j in range(n)]
        for j in range(n):
            struct.pack_into('<3I', nodes, addr[j] - NODES, vals[j], addr[j + 1] if j + 1 < n else 0, addr[j - 1] if j else 0)
        struct.pack_into('<4I', recs, 0x40 * i + 0x24, r.getrandbits(32), addr[0] if n else 0, addr[-1] if n else 0, n)
    regions[RECS] = recs
    regions[NODES] = nodes
    cat = bytearray(r.bytes(0xd0))
    cat[0x3c:0x4c] = deque(r, BLOCKA, MAPA, [RECS + 0x40 * i for i in range(nrec)], regions)
    cat[0x9c:0xac] = deque(r, BLOCKC, MAPC, [r.choice([r.randrange(1, 7), r.getrandbits(32)]) for _ in range(banks)], regions)
    struct.pack_into('<I', cat, 0xc8, r.randrange(nrec))
    cat[0xcc] = r.choice([0, 1, 1, 0x80])
    regions[CAT] = cat
    return regions


def image(r, banks, levels, top):
    """An SFX.map image from its records down, in the fixup's order; levels: which record kinds come first ('c' categories, 's' sounds, 'v' variations).
    Returns (bytes, the count of top records)."""
    def link(nvars):
        return struct.pack('<I', r.choice([r.randrange(1, nvars + 1), r.randrange(1, nvars + 1), r.getrandbits(32)])) + r.bytes(4)

    def sample():
        s = bytearray(r.bytes(0x10))
        struct.pack_into('<H', s, 0xc, r.choice([0, r.randrange(1, banks + 1), r.randrange(1, banks + 1)]))
        return bytes(s)

    def variation_head(nsamp, nlinks):
        v = bytearray(r.bytes(0x2a))
        struct.pack_into('<HHI', v, 0, nsamp, r.getrandbits(16), nlinks)
        if r.random() < 0.3:
            struct.pack_into('<2H', v, 0x10, 0, 0)
        return v

    def variations(n):   # the heads, then each one's samples and links
        heads, tails = [], []
        for _ in range(n):
            ns, nl = r.choice([0, 1, 1, 2, 3]), r.choice([0, 0, 1, 2, 3])
            heads.append(variation_head(ns, nl))
            tails.append(b''.join(sample() for _ in range(ns)) + b''.join(link(n) for _ in range(nl)))
        return b''.join(heads) + b''.join(tails)

    def sounds(n):
        heads, tails = [], []
        for _ in range(n):
            nv = r.choice([0, 1, 1, 2, 3])
            s = bytearray(r.bytes(0x14))
            struct.pack_into('<II', s, 0, r.getrandbits(16), nv)
            struct.pack_into('<H', s, 0x10, r.choice([6, 0xe, 2, 4, r.getrandbits(16)]))
            heads.append(s)
            tails.append(variations(nv))
        return b''.join(heads) + b''.join(tails)

    def categories(n):
        heads, tails = [], []
        for _ in range(n):
            ns = r.choice([0, 1, 2, 3])
            c = bytearray(r.bytes(0x18))
            struct.pack_into('<I', c, 0, ns)
            heads.append(c)
            tails.append(sounds(ns))
        return b''.join(heads) + b''.join(tails)

    body = {'c': categories, 's': sounds, 'v': variations}[levels](top)
    return bytearray(body) + bytearray(r.bytes(0x10)), top


def case(r):
    name = r.deal(OPS)
    if name == 'count':
        regions = {IMAGE: r.bytes(8)}
        return Call(CALLS[name], [IMAGE], regions, conv='c', ret='a')
    if name == 'simple':   # a sound and its variation, each condition met mostly
        s = bytearray(r.bytes(0x14))
        v = bytearray(r.bytes(0x2a))
        struct.pack_into('<I', s, 4, 1 if r.random() < 0.8 else r.choice([0, 2, r.getrandbits(32)]))
        struct.pack_into('<I', s, 8, IMAGE + 0x40)
        struct.pack_into('<H', s, 0x10, r.choice([6, 6, 0xe, 0xfffe, 2, 4, r.getrandbits(16)]))
        struct.pack_into('<I', v, 0, (r.getrandbits(16) << 16) | (1 if r.random() < 0.8 else r.choice([0, 2])))
        struct.pack_into('<I', v, 0x26, 0 if r.random() < 0.8 else r.getrandbits(32))
        struct.pack_into('<2H', v, 0x10, *(0, 0) if r.random() < 0.7 else r.choice([(1, 0), (0, 1), (r.getrandbits(16), r.getrandbits(16))]))
        regions = {CAT: r.bytes(0xd0), IMAGE: s, IMAGE + 0x40: v}
        return Call(CALLS[name], [CAT, IMAGE], regions, conv='t', ret='v')
    banks = r.randrange(1, 7)
    regions = category(r, banks)
    if name == 'fixup':
        top = r.choice([1, 1, 2])
        body, top = image(r, banks, 'c', top)
        regions[IMAGE] = body
        regions[OUT] = r.bytes(4)
        return Call(CALLS[name], [CAT, IMAGE, OUT, top if r.random() < 0.9 else 0], regions, conv='t', ret='a')
    level = {'record': 'c', 'sound': 's', 'variation': 'v'}[name]
    body, _top = image(r, banks, level, 1)
    size = {'c': 0x18, 's': 0x14, 'v': 0x2a}[level]
    regions[IMAGE] = body[size:]
    regions[OUT] = body[:size] + r.bytes(0x10)   # the record itself apart from the data that follows it
    return Call(CALLS[name], [CAT, IMAGE, OUT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

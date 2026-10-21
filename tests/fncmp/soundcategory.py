# A sound category's SFX.map loading (src/game/core/soundcategory) against the originals: the image fixups (0x6c0510 and below: a valid image of
# 1 or 2 categories, 0-3 sounds each, 0-3 variations, 0-3 samples and 0-3 links, built in the order the fixup walks it; the samples' banks 0 or
# within deque C, the weights random, delta-coded or not) over a category object whose deques A (the category records, each with a PtrList of
# 0-3 banks) and C (the bank indices) are small stretches of one or two blocks (the block boundary crossed now and then); a variation's sample
# count (0x6bd7a0) and the simple sound mark (0x6c0550: each of its conditions met or missed). The free slots in deques A and B (0x6bf150 with
# holes in A, 0x6bf1c0 over bank objects and gaps, long runs crossing into the second block); the BANK.map entries read (0x6bf9e0) through a stream recorder whose reads hand out the file's bytes in turn (seq_read).
# usage: soundcategory.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, echo, fill, heap, import_table, log_region, seq_read, set_return, strings, vtable  # noqa: E402

CALLS = {'count': 0x6bd7a0, 'simple': 0x6c0550, 'variation': 0x6c06e0, 'sound': 0x6c0610, 'record': 0x6c05b0, 'fixup': 0x6c0510, 'readimage': 0x6c0360,
         'readmap': 0x6c0450, 'freecat': 0x6bf150, 'freebank': 0x6bf1c0, 'entries': 0x6bf9e0}
B = Bench([(0x6bd7a0, 0x6bd7ac), (0x6bf150, 0x6bf253), (0x6bf9e0, 0x6bfa5c), (0x6bfe10, 0x6bfe34), (0x6c0310, 0x6c035f), (0x6c0360, 0x6c0844), (0x5f7180, 0x5f72c0), (0x4036a0, 0x4036e0), (0x639100, 0x639120), (0x618d30, 0x618d80), (0x6b9b30, 0x6b9b94), (0x5e42d0, 0x5e431c), (0x67a400, 0x690000)])
CAT, OUT, RECS, NODES = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400
MAPA, MAPC, MAPB = RAM + 0x600, RAM + 0x640, RAM + 0x680
BLOCKA, BLOCKC, BLOCKB = RAM + 0x10000, RAM + 0x20000, RAM + 0x40000   # (two blocks each, 0x1000 apart, only the used stretch mapped)
IMAGE = RAM + 0x3000
STREAM, SRC, VT_STREAM, VT_SRC, VT_HANDLE, FILE = RAM + 0x700, RAM + 0x740, RAM + 0x780, RAM + 0x800, RAM + 0x880, RAM + 0x5000
SFX_GUID, BANK_GUID = bytes.fromhex('002c61e9d031d211b40900b0c993f203'), bytes.fromhex('012c61e9d031d211b40900a0c993f203')
STREAM_VTABLE = 0x703100
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def deque(r, at, mapat, values, regions):
    """A deque's begin iterator (16 bytes) over values starting at a random place in its first block at `at` (into the next, at + 0x1000 or + 0x3000,
    when they run past it); maps the stretch of each block they use and the map's two slots."""
    n = len(values)
    start = max(0, min(0x3ff, r.choice([0, 0x3ff - n // 2, 0x400 - n, r.randrange(max(1, 0x400 - n + 1))])))
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


def deque_full(r, at, mapat, values, regions):
    """deque() and the rest of the deque after its begin iterator: the end iterator (at the end of its block when the values fill it, as the
    deque's push_back leaves it), the map (2 slots) and the count (0x2c bytes, for the deque + 4)."""
    it = deque(r, at, mapat, values, regions)
    first, _last, cur, _node = struct.unpack('<4I', it)
    blocks = struct.unpack('<2I', regions[mapat])
    q = (cur - first) // 4 + len(values)
    b = q // 0x400 - 1 if q and q % 0x400 == 0 else q // 0x400
    return it + struct.pack('<4I', blocks[b], blocks[b] + 0x1000, blocks[b] + 4 * (q - 0x400 * b), mapat + 4 * b) + struct.pack('<3I', mapat, 2, len(values))


def category(r, banks=4, holes=False, bank_objects=None):
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
    cat[0x3c:0x68] = deque_full(r, BLOCKA, MAPA, [0 if holes and r.random() < 0.3 else RECS + 0x40 * i for i in range(nrec)], regions)
    cat[0x9c:0xc8] = deque_full(r, BLOCKC, MAPC, [r.choice([r.randrange(1, 7), r.getrandbits(32)]) for _ in range(banks)], regions)
    if bank_objects is not None:
        cat[0x6c:0x98] = deque_full(r, BLOCKB, MAPB, bank_objects, regions)
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
    if name in ('readimage', 'readmap'):   # SFX.map's bytes, read in turn (a sequenced read)
        top = r.choice([0, 1, 1, 2])
        body, top = image(r, banks, 'c', top)
        guid = r.choice([SFX_GUID, SFX_GUID, SFX_GUID, BANK_GUID, bytes(16)])
        data = guid + struct.pack('<4I', r.choice([0, 0, 1]), 0, top, 0)[:12] + bytes(body)
        regions[FILE] = data
        log = log_region()
        size, pos = r.choice([(len(data), 0x14), (len(data), 0x14), (len(data), 0x14), (len(data) - 8, 0x14), (len(data) + 0x20, 0x14), (0x1c, 0x14)])
        regions[LOG] = log
        regions[HEAP] = heap(0x3000)
        if name == 'readimage':
            seq_read(log, 2, FILE, len(data), buffer=0, count=1)
            strings(log, 2, 0, out=1)
            set_return(log, 5, size)
            set_return(log, 6, pos)
            regions[STREAM] = struct.pack('<2I', VT_STREAM, r.getrandbits(32))
            regions[VT_STREAM] = vtable({2: 2})
            regions[OUT] = r.bytes(8)
            return Call(CALLS[name], [CAT, OUT, STREAM, OUT + 4], regions, conv='t', ret='a')
        # the category's Handle (+0x30) opens the stream: its slot 3 answers &status, which it fills, and gives the stream SRC (a recorder source)
        status = r.choice([0, 0, 0, 1, -1, -0x80000000])
        struct.pack_into('<2I', regions[CAT], 0x30, VT_HANDLE, r.getrandbits(32))
        regions[VT_HANDLE] = vtable({3: 5})
        echo(log, 3, 0)
        fill(log, 3, 0, struct.pack('<i', status), 0)
        fill(log, 3, 1, struct.pack('<2I', STREAM_VTABLE, SRC), 1)
        strings(log, 3, 4, out=3)
        seq_read(log, 1, FILE, len(data), buffer=0, count=1)
        strings(log, 1, 0, out=1)
        set_return(log, 4, size)
        set_return(log, 5, pos)
        regions[VT_SRC] = vtable({0: 1, 1: 2})
        regions[SRC] = struct.pack('<3I', VT_SRC, 0, r.choice([1, 2]))
        regions[IMPORT_TABLE] = import_table(log, {'InterlockedDecrement': (51, 0x6fd12c, 1, 0, 0)})
        set_return(log, 51, r.choice([0, 1]))
        regions[OUT] = r.bytes(8)
        regions[STREAM] = b'cat_x' + r.choice([b'SFX.map', b'']) + bytes(1)
        return Call(CALLS[name], [CAT, OUT, STREAM, OUT + 4], regions, conv='t', ret='a')
    if name == 'freecat':
        regions = category(r, banks, holes=True)
        nrec = struct.unpack_from('<I', regions[CAT], 0x64)[0]
        struct.pack_into('<I', regions[CAT], 0x64, r.choice([nrec, nrec, max(0, nrec - 1), 0]))
        return Call(CALLS[name], [CAT], regions, conv='t', ret='a')
    if name == 'freebank':   # deque B of bank objects (any non-zero) and gaps; a long run crosses into the second block
        n = r.choice([0, 1, 3, 6, r.randrange(0x3f0, 0x401)])
        objs = [r.choice([0, RAM + 0x100, r.getrandbits(32) | 1, r.getrandbits(32) | 1]) if r.random() < 0.3 else RAM + 4 * i + 4 for i in range(n)]
        regions = category(r, banks, bank_objects=objs)
        return Call(CALLS[name], [CAT], regions, conv='t', ret='a')
    if name == 'entries':   # the rest of BANK.map: the 11-byte entries, then each path (a u32 length and the chars)
        count = r.choice([0, 1, 2, 3, 5])
        paths = [bytes(r.choice(b'ab\/.') for _ in range(r.randrange(1, 12))) for _ in range(count)]
        data = r.bytes(11 * count) + b''.join(struct.pack('<I', len(p)) + p for p in paths) + r.bytes(r.choice([0, 0, 3])) + bytes(4)
        regions = {HEAP: heap(0x2000), CAT: r.bytes(0xd0), FILE: data}
        log = log_region()
        regions[LOG] = log
        pos = r.choice([0x1c, 0x1c, 0, 0x100])
        seq_read(log, 2, FILE, len(data), buffer=0, count=1)
        strings(log, 2, 0, out=1)
        set_return(log, 5, len(data) + pos)
        set_return(log, 6, pos)
        regions[STREAM] = struct.pack('<2I', VT_STREAM, r.getrandbits(32))
        regions[VT_STREAM] = vtable({2: 2})
        regions[OUT] = struct.pack('<2I', r.getrandbits(32), count if r.random() < 0.9 else max(0, count - 1))
        return Call(CALLS[name], [CAT, STREAM, OUT], regions, conv='t', ret='v')
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

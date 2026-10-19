# zlib 1.1.3 (src/game/lib/zlib: the library built from third_party/zlib-1.1.3) against the exe's copies of it. A deflate_state at S
# (0x16b8 bytes, random except the fields a function reads as sizes or pointers), its pending buffer at PEND, input bytes at BUF.
# usage: zlib.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=adler32,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import EXE, Bench, Call, RAM  # noqa: E402
from pe_read import PE  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {
    'adler32': (0x63ea00, 0x63eb30), 'init_block': (0x63eba0, 0x63ec10), 'set_data_type': (0x640460, 0x6404d0),
    'bi_flush': (0x6404f0, 0x640580), 'bi_windup': (0x640580, 0x6405f0), 'copy_block': (0x6405f0, 0x640690),
    '_tr_stored_block': (0x63ec10, 0x63ecb0), '_tr_align': (0x63ecb0, 0x63ef00), '_tr_flush_block': (0x63ef00, 0x63f100),
    'build_tree': (0x63f100, 0x63f340), 'pqdownheap': (0x63f340, 0x63f400), 'gen_bitlen': (0x63f400, 0x63f630), 'gen_codes': (0x63f630, 0x63f6b0),
    'build_bl_tree': (0x63f6b0, 0x63f730), 'scan_tree': (0x63f730, 0x63f820), 'send_all_trees': (0x63f820, 0x63fa90), 'send_tree': (0x63fa90, 0x640010),
    'compress_block': (0x640010, 0x640460), 'bi_reverse': (0x6404d0, 0x6404f0),
}
B = Bench(sorted(FUNCS.values()))
S, PEND, BUF, LBUF, DBUF, BIG = RAM, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x4800, RAM + 0x10000
LIT_MAX = 300  # (symbols in a block: at most 6 bytes each in the pending buffer)
STATE = 0x16b8
# deflate_state offsets (deflate.h; src/game/lib/zlib.cpp checks them)
PENDING_BUF, PENDING, DATA_TYPE, DYN_LTREE, LAST_EOB_LEN, BI_BUF, BI_VALID = 0x8, 0x14, 0x1c, 0x8c, 0x16ac, 0x16b0, 0x16b4
PEND_SIZE = 0x1000


def state(r, pending_max=0x100):
    """A random state with a pending buffer (PEND) filled up to `pending` and bits waiting in bi_buf."""
    s = r.bytes(STATE)
    struct.pack_into('<I', s, PENDING_BUF, PEND)
    struct.pack_into('<i', s, PENDING, r.randrange(pending_max))
    valid = r.choice([0, 1, 7, 8, 9, 15, 16, r.randrange(17)])
    struct.pack_into('<HxxI', s, BI_BUF, r.getrandbits(16) & ((1 << valid) - 1) if r.random() < 0.8 else r.getrandbits(16), valid)
    return s


def regions(r, s):
    return {S: s, PEND: r.bytes(PEND_SIZE)}


def adler32(r):
    n = r.choice([0, 1, 15, 16, 17, r.randrange(64), r.randrange(0x1000), 5551, 5552, 5553, r.randrange(0x15b0, 0x4000)])
    data = bytes([0xff] * n) if r.random() < 0.2 else r.bytes(n)  # (all 0xff: the sums at their largest before the modulo)
    adler = r.choice([1, 0, 0xfff00000 | r.getrandbits(16), r.getrandbits(32), 0xfff0fff0])
    if data and data[0] == 0xff and r.random() < 0.7:
        adler = 0xfff0fff0  # (both sums at BASE - 1: NMAX is the most bytes that cannot overflow from there)
    if n == 0 and r.random() < 0.5:
        return Call(0x63ea00, [adler, 0, r.randrange(100)], {BIG: b'\0' * 4}, ret='a')  # a null buffer: 1
    return Call(0x63ea00, [adler, BIG, n], {BIG: data or b'\0' * 4}, ret='a')


def init_block(r):
    s = state(r)
    return Call(0x63eba0, [S], regions(r, s))


def set_data_type(r):
    s = state(r)
    f = [r.randrange(50) if 32 <= i < 128 else r.choice([0, 0, 0, r.randrange(30), r.randrange(400)]) for i in range(256)]
    if r.random() < 0.3:  # binary exactly when bin_freq > ascii_freq / 4: put bin_freq at the edge
        ascii = sum(f[7:128])
        f[0] = max(0, (ascii >> 2) - sum(f[1:7]) - sum(f[128:256]) + r.choice([-1, 0, 0, 1]))
    for i, v in enumerate(f):  # frequencies: mostly text, sometimes binary
        struct.pack_into('<H', s, DYN_LTREE + 4 * i, v & 0xffff)
    return Call(0x640460, [S], regions(r, s))


def bi_flush(r):
    return Call(0x6404f0, [S], regions(r, state(r)))


def bi_windup(r):
    return Call(0x640580, [S], regions(r, state(r)))


def copy_block(r):
    s = state(r)
    n = r.choice([0, 1, r.randrange(0x40), r.randrange(0x400)])
    return Call(0x6405f0, [S, BUF, n, r.choice([0, 1])], {**regions(r, s), BUF: r.bytes(max(n, 4))})



# --- trees.c: the Huffman trees of a block
L_CODES, D_CODES, BL_CODES, HEAP_SIZE, END_BLOCK = 286, 30, 19, 573, 256
DYN_DTREE, BL_TREE, L_DESC, D_DESC, BL_DESC, BL_COUNT, HEAP, HEAP_LEN, HEAP_MAX, DEPTH = 0x980, 0xa74, 0xb10, 0xb1c, 0xb28, 0xb34, 0xb54, 0x1448, 0x144c, 0x1450
LEVEL, L_BUF, LIT_BUFSIZE, LAST_LIT, D_BUF, OPT_LEN, STATIC_LEN, MATCHES = 0x7c, 0x1690, 0x1694, 0x1698, 0x169c, 0x16a0, 0x16a4, 0x16a8
# the exe's static_l_desc, static_d_desc, static_bl_desc (what _tr_init stores) and static_ltree / static_dtree
STATIC_DESC = {L_DESC: 0x779ba8, D_DESC: 0x779bc0, BL_DESC: 0x779bd8}
STATIC_LTREE, STATIC_DTREE = 0x704158, 0x7045d8
# desc -> (tree offset, elems, max_length)
TREES = {L_DESC: (DYN_LTREE, L_CODES, 15), D_DESC: (DYN_DTREE, D_CODES, 15), BL_DESC: (BL_TREE, BL_CODES, 7)}
_pe = PE(EXE)
LENGTH_CODE, DIST_CODE = _pe.bytes(0x704850, 256), _pe.bytes(0x704650, 512)


def descs(s, max_codes=None):
    """The three tree descriptors as _tr_init sets them (the exe's static descriptors), with max_code given or random."""
    for d, (tree, _, _) in TREES.items():
        struct.pack_into('<IiI', s, d, S + tree, max_codes[d] if max_codes else -1, STATIC_DESC[d])


def freqs(r, s, desc, n=None):
    """Random frequencies for desc's tree: sparse, dense, few, or a Fibonacci run (lengths past max_length)."""
    tree, elems, _ = TREES[desc]
    kind = r.choice(['sparse', 'dense', 'few', 'fib'])
    f = [0] * elems
    if kind == 'fib':
        a, b = 1, 1
        for i in r.sample(range(elems), min(elems, r.randrange(2, 26))):
            f[i] = a
            a, b = b, min(a + b, 0xffff)
    else:
        for i in range(elems):
            if kind == 'dense' or r.random() < (0.25 if kind == 'sparse' else 0.03):
                f[i] = r.choice([1, 1, 2, r.randrange(1, 40), r.randrange(1, 3000)])
    for i, v in enumerate(f):
        struct.pack_into('<H', s, tree + 4 * i, v)
    return f


def lens(r, s, tree, count, maxlen=15):
    """Code lengths with runs (what scan_tree / send_tree code as repeats), and random codes."""
    i = 0
    while i < count:
        run, v = r.choice([1, 1, 2, 3, 4, 6, 7, 11, r.randrange(1, 140)]), r.choice([0, 0, r.randrange(1, maxlen + 1)])
        for _ in range(min(run, count - i)):
            struct.pack_into('<HH', s, tree + 4 * i, r.getrandbits(16), v)
            i += 1


def bl_codes(r, s):
    """The bit length tree's codes (send_tree / send_all_trees send with them)."""
    for i in range(BL_CODES):
        n = r.randrange(8)
        struct.pack_into('<HH', s, BL_TREE + 4 * i, r.getrandbits(n), n)


def build_tree(r):
    s = state(r)
    desc = r.choice(list(TREES))
    descs(s)
    freqs(r, s, desc)
    return Call(0x63f100, [S, S + desc], regions(r, s))


def pqdownheap(r):
    s = state(r)
    tree = r.choice([DYN_LTREE, BL_TREE])
    nodes = HEAP_SIZE if tree == DYN_LTREE else 2 * BL_CODES + 1
    n = r.choice([1, 2, 3, r.randrange(1, 40), r.randrange(1, HEAP_SIZE)])
    struct.pack_into('<i', s, HEAP_LEN, n)
    for i in range(1, n + 2):
        struct.pack_into('<i', s, HEAP + 4 * i, r.randrange(nodes))
    for i in range(nodes):  # few distinct frequencies and depths, so the ties are taken
        struct.pack_into('<H', s, tree + 4 * i, r.randrange(6))
        s[DEPTH + i] = r.randrange(4)
    return Call(0x63f340, [S, S + tree, r.randrange(1, n + 1)], regions(r, s))


def gen_bitlen(r):
    """A random full binary tree over some leaves, laid out as build_tree leaves it (root at heap[heap_max], parents first)."""
    s = state(r)
    desc = r.choice(list(TREES))
    tree, elems, _ = TREES[desc]
    leaves = sorted(r.sample(range(elems), r.randrange(2, min(elems, 40) + 1)))
    max_code = leaves[-1] if r.random() < 0.9 else r.randrange(leaves[-1] + 1)
    descs(s, {L_DESC: max_code, D_DESC: max_code, BL_DESC: max_code})
    for n in leaves:
        struct.pack_into('<H', s, tree + 4 * n, r.randrange(1, 300))
    heap_max, node, pool = HEAP_SIZE, elems, list(leaves)
    while len(pool) > 1:
        a = pool.pop(r.randrange(len(pool)))
        b = pool.pop(r.randrange(len(pool)))
        for c in (a, b):
            heap_max -= 1
            struct.pack_into('<i', s, HEAP + 4 * heap_max, c)
            struct.pack_into('<H', s, tree + 4 * c + 2, node)  # Dad
        pool.append(node)
        node += 1
    heap_max -= 1
    struct.pack_into('<i', s, HEAP + 4 * heap_max, pool[0])
    struct.pack_into('<i', s, HEAP_MAX, heap_max)
    return Call(0x63f400, [S, S + desc], regions(r, s))


def gen_codes(r):
    s = state(r)
    max_code = r.randrange(-1, L_CODES)
    lens(r, s, DYN_LTREE, max_code + 1)
    for b in range(16):
        struct.pack_into('<H', s, BL_COUNT + 2 * b, r.choice([0, 0, 1, 2, r.randrange(20)]))
    return Call(0x63f630, [S + DYN_LTREE, max_code, S + BL_COUNT], regions(r, s))


def scan_tree(r):
    s = state(r)
    tree, elems = r.choice([(DYN_LTREE, L_CODES), (DYN_DTREE, D_CODES)])
    max_code = r.randrange(-1, elems)
    lens(r, s, tree, max_code + 1)
    return Call(0x63f730, [S, S + tree, max_code], regions(r, s))


def send_tree(r):
    s = state(r)
    tree, elems = r.choice([(DYN_LTREE, L_CODES), (DYN_DTREE, D_CODES)])
    max_code = r.randrange(-1, elems)
    lens(r, s, tree, max_code + 2)
    bl_codes(r, s)
    return Call(0x63fa90, [S, S + tree, max_code], regions(r, s))


def build_bl_tree(r):
    s = state(r)
    lmax, dmax = r.randrange(256, L_CODES), r.randrange(-1, D_CODES)
    descs(s, {L_DESC: lmax, D_DESC: dmax, BL_DESC: -1})
    lens(r, s, DYN_LTREE, lmax + 1)
    lens(r, s, DYN_DTREE, dmax + 1)
    for i in range(BL_CODES):
        struct.pack_into('<H', s, BL_TREE + 4 * i, r.choice([0, 0, 0, r.randrange(5)]))
    return Call(0x63f6b0, [S], regions(r, s), ret='a')


def send_all_trees(r):
    s = state(r)
    lcodes, dcodes, blcodes = r.randrange(257, L_CODES + 1), r.randrange(1, D_CODES + 1), r.randrange(4, BL_CODES + 1)
    lens(r, s, DYN_LTREE, lcodes + 1)
    lens(r, s, DYN_DTREE, dcodes + 1)
    bl_codes(r, s)
    return Call(0x63f820, [S, lcodes, dcodes, blcodes], regions(r, s))


def symbols(r, s, n):
    """n literals and matches in l_buf / d_buf (as _tr_tally leaves them), their frequencies counted into the trees after init_block's."""
    lf, df = [0] * L_CODES, [0] * D_CODES
    lf[END_BLOCK] = 1
    lb, db = bytearray(), bytearray()
    for _ in range(n):
        if r.random() < 0.6:
            c = r.choice([r.randrange(256), r.randrange(97, 123)])
            lb.append(c)
            db += struct.pack('<H', 0)
            lf[c] += 1
        else:
            length = r.choice([0, 1, r.randrange(256)])  # match length - 3
            dist = r.choice([1, 2, r.randrange(1, 256), r.randrange(1, 32768)])
            lb.append(length)
            db += struct.pack('<H', dist)
            lf[LENGTH_CODE[length] + 257] += 1
            d = dist - 1
            df[DIST_CODE[d] if d < 256 else DIST_CODE[256 + (d >> 7)]] += 1
    for i, v in enumerate(lf):
        struct.pack_into('<H', s, DYN_LTREE + 4 * i, v)
    for i, v in enumerate(df):
        struct.pack_into('<H', s, DYN_DTREE + 4 * i, v)
    for i in range(BL_CODES):
        struct.pack_into('<H', s, BL_TREE + 4 * i, 0)
    struct.pack_into('<IIII', s, L_BUF, LBUF, r.randrange(n + 1, 0x8000), n, DBUF)
    struct.pack_into('<IIII', s, OPT_LEN, 0, 0, 0, r.choice([7, 8, 9, 10, r.randrange(20)]))
    return {LBUF: lb + b'\0' * 4, DBUF: db + b'\0' * 4}


def compress_block(r):
    s = state(r)
    syms = symbols(r, s, r.choice([0, 1, r.randrange(30), r.randrange(LIT_MAX)]))
    if r.random() < 0.5:
        trees = [STATIC_LTREE, STATIC_DTREE]
    else:
        trees = [S + DYN_LTREE, S + DYN_DTREE]
        lens(r, s, DYN_LTREE, L_CODES)
        lens(r, s, DYN_DTREE, D_CODES)
    return Call(0x640010, [S] + trees, {**regions(r, s), **syms})


def tr_stored_block(r):
    s = state(r)
    n = r.choice([0, 1, r.randrange(0x40), r.randrange(0x400)])
    return Call(0x63ec10, [S, BUF, n, r.choice([0, 1])], {**regions(r, s), BUF: r.bytes(max(n, 4))})


def tr_align(r):
    s = state(r)
    struct.pack_into('<i', s, LAST_EOB_LEN, r.choice([7, 8, 9, 10, r.randrange(16)]))
    return Call(0x63ecb0, [S], regions(r, s))


def tr_flush_block(r):
    s = state(r)
    descs(s)
    syms = symbols(r, s, r.choice([0, 1, r.randrange(30), r.randrange(LIT_MAX)]))
    struct.pack_into('<i', s, LEVEL, r.choice([0, 1, 6, 9]))
    s[DATA_TYPE] = r.choice([0, 1, 2, 2])
    n = r.choice([0, r.randrange(16), r.randrange(0x400)])
    buf = r.choice([BUF, BUF, 0])
    return Call(0x63ef00, [S, buf, n, r.choice([0, 1])], {**regions(r, s), **syms, BUF: r.bytes(max(n, 4))})

OPS = [f for f in (adler32, init_block, set_data_type, bi_flush, bi_windup, copy_block, build_tree, pqdownheap, gen_bitlen, gen_codes, scan_tree,
                   send_tree, build_bl_tree, send_all_trees, compress_block, tr_stored_block, tr_align, tr_flush_block)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)

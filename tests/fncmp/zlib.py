# zlib 1.1.3 (src/game/lib/zlib: the library built from third_party/zlib-1.1.3) against the exe's copies of it. A deflate_state at S
# (0x16b8 bytes, random except the fields a function reads as sizes or pointers), its pending buffer at PEND, input bytes at BUF; for
# the deflate.c functions the whole state as deflateInit2_ lays it out at small sizes (deflating()), its stream at STRM.
# usage: zlib.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=adler32,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CPU, EXE, RAM, Bench, Call, cpu_region  # noqa: E402
from pe_read import PE  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {
    'adler32': (0x63ea00, 0x63eb30), 'init_block': (0x63eba0, 0x63ec10), 'set_data_type': (0x640460, 0x6404d0),
    'bi_flush': (0x6404f0, 0x640580), 'bi_windup': (0x640580, 0x6405f0), 'copy_block': (0x6405f0, 0x640690),
    '_tr_stored_block': (0x63ec10, 0x63ecb0), '_tr_align': (0x63ecb0, 0x63ef00), '_tr_flush_block': (0x63ef00, 0x63f100),
    'build_tree': (0x63f100, 0x63f340), 'pqdownheap': (0x63f340, 0x63f400), 'gen_bitlen': (0x63f400, 0x63f630), 'gen_codes': (0x63f630, 0x63f6b0),
    'build_bl_tree': (0x63f6b0, 0x63f730), 'scan_tree': (0x63f730, 0x63f820), 'send_all_trees': (0x63f820, 0x63fa90), 'send_tree': (0x63fa90, 0x640010),
    'compress_block': (0x640010, 0x640460), 'bi_reverse': (0x6404d0, 0x6404f0),
    'putShortMSB': (0x635710, 0x635740), 'flush_pending': (0x635740, 0x6357c0), 'lm_init': (0x635870, 0x635910), 'deflate_stored': (0x635910, 0x635a70),
    'fill_window': (0x635a70, 0x635ba0), 'read_buf': (0x635ba0, 0x635c20), 'deflate_fast': (0x635c20, 0x635f80), 'longest_match': (0x635f80, 0x636100),
    'deflate_slow': (0x636100, 0x636560),
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
    if r.random() < 0.25:  # (past NMAX bytes from there: the first NMAX would overflow s2 without the modulo between)
        n, data, adler = r.choice([5553, 5568, r.randrange(5553, 0x4000)]), bytes([0xff] * 0x4000), 0xfff0fff0
        data = data[:n]
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
EXTRA_LBITS, EXTRA_DBITS = struct.unpack('<29i', _pe.bytes(0x704000, 29 * 4)), struct.unpack('<30i', _pe.bytes(0x704078, 30 * 4))


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


def static_bits(s):
    """The block's length with the static trees (what build_tree leaves in static_len, from the frequencies in the state)."""
    n = 0
    for i in range(L_CODES):
        f = struct.unpack_from('<H', s, DYN_LTREE + 4 * i)[0]
        n += f * ((8 if i < 144 else 9) if i < 256 else (7 if i < 280 else 8) + EXTRA_LBITS[i - 257])
    for i in range(D_CODES):
        n += struct.unpack_from('<H', s, DYN_DTREE + 4 * i)[0] * (5 + EXTRA_DBITS[i])
    return n


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
    eob = r.choice([7, 8, 9, 10, r.randrange(16)])
    if r.random() < 0.4:  # (at the second block's threshold: 1 + last_eob_len + 10 - bi_valid around 9, bi_valid as the first block leaves it)
        eob = max(0, (struct.unpack_from('<I', s, BI_VALID)[0] + 2) % 8 - 2 + r.choice([-1, 0, 0, 1]))
    struct.pack_into('<i', s, LAST_EOB_LEN, eob)
    return Call(0x63ecb0, [S], regions(r, s))


def tr_flush_block(r):
    s = state(r)
    descs(s)
    syms = symbols(r, s, r.choice([0, 1, r.randrange(30), r.randrange(LIT_MAX)]))
    struct.pack_into('<i', s, LEVEL, r.choice([0, 1, 6, 9]))
    s[DATA_TYPE] = r.choice([0, 1, 2, 2])
    n = r.choice([0, r.randrange(16), r.randrange(0x400)])
    if r.random() < 0.4:  # (stored_len + 4 at the static length in bytes: stored when the dynamic trees cost no less)
        n = max(0, ((static_bits(s) + 3 + 7) >> 3) - 4 + r.choice([-1, 0, 0, 1]))
    buf = r.choice([BUF, BUF, 0])
    return Call(0x63ef00, [S, buf, n, r.choice([0, 1])], {**regions(r, s), **syms, BUF: r.bytes(max(n, 4))})


# --- deflate.c: the window, the hash chains and the stream
STRM, OUT, WIN, PREV, HEAD = RAM + 0x5000, RAM + 0x5100, RAM + 0x6100, RAM + 0x6b00, RAM + 0x7400
WIN_BIG, PREV_BIG = RAM + 0x20000, RAM + 0x28000  # (windowBits 13: the window and prev of deflating(big=True))
OUT_SIZE, WIN_SLACK = 0x1000, 0x110  # (longest_match reads up to MAX_MATCH past a string; zlib's window has that room, the bench gives it)
W_SIZE, W_BITS, W_MASK, WINDOW, WINDOW_SIZE, PREV_, HEAD_, INS_H, HASH_SIZE, HASH_BITS, HASH_MASK, HASH_SHIFT = (
    0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c, 0x40, 0x44, 0x48, 0x4c, 0x50)
BLOCK_START, MATCH_LENGTH, PREV_MATCH, MATCH_AVAILABLE, STRSTART, MATCH_START, LOOKAHEAD, PREV_LENGTH = 0x54, 0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c, 0x70
MAX_CHAIN, MAX_LAZY, STRATEGY, GOOD_MATCH, NICE_MATCH, NOHEADER, PENDING_OUT, PENDING_BUF_SIZE = 0x74, 0x78, 0x80, 0x84, 0x88, 0x18, 0x10, 0xc
MIN_MATCH, MAX_MATCH, MIN_LOOKAHEAD, TOO_FAR = 3, 258, 262, 4096
# the z_stream: next_in, avail_in, total_in, next_out, avail_out, total_out, msg, state, zalloc, zfree, opaque, data_type, adler, reserved


def text(r, n):
    """Bytes that repeat (a small alphabet, copied runs, or a short period with sparse changes), so strings match, at many lengths."""
    if r.random() < 0.3:
        period, every = r.randrange(1, 12), r.choice([4, 8, 16, 32, 64])
        unit = bytes(r.randrange(0x61, 0x65) for _ in range(period))
        return bytearray(r.randrange(0x61, 0x65) if r.randrange(every) == 0 else unit[i % period] for i in range(n))
    out = bytearray()
    alpha = r.choice([2, 4, 26, 256])
    while len(out) < n:
        if out and r.random() < 0.4:
            at = r.randrange(len(out))
            out += out[at:at + r.choice([3, 4, 8, r.randrange(3, 300)])]
        else:
            out += bytes((r.randrange(alpha) + 0x61) % 256 for _ in range(r.randrange(1, 12)))
    return out[:n]


def deflating(r, fresh=False, big=False):
    """A deflate_state as deflateInit2_ lays it out (window, prev, head; pending_buf overlaid by d_buf and l_buf) at memLevel 1..3 and
    windowBits 9 or 10 (big: 13, past TOO_FAR, the window and prev at WIN_BIG / PREV_BIG), with its stream at STRM. fresh: as lm_init
    leaves it; else part way through (random positions and chains)."""
    s = state(r)
    w_bits, mem = 13 if big else r.choice([9, 10]), r.choice([1, 2, 3])
    win, prv = (WIN_BIG, PREV_BIG) if big else (WIN, PREV)
    w_size, hash_bits, lit = 1 << w_bits, mem + 7, 1 << (mem + 6)
    struct.pack_into('<IiI', s, 0, STRM, r.choice([42, 113]), PEND)
    struct.pack_into('<IIiiBBxxiIII', s, PENDING_BUF_SIZE, lit * 4, PEND, 0, r.choice([0, 0, 1]), 2, 8, 0, w_size, w_bits, w_size - 1)
    struct.pack_into('<IIIIIIIII', s, WINDOW, win, 2 * w_size, prv, HEAD, 0, 1 << hash_bits, hash_bits, (1 << hash_bits) - 1, (hash_bits + 2) // 3)
    level = r.choice([1, 2, 3, 4, 6, 9])
    good, lazy, nice, chain = [(4, 4, 8, 4), (4, 5, 16, 8), (4, 6, 32, 32), (4, 4, 16, 16), (8, 16, 128, 128), (32, 258, 258, 4096)][[1, 2, 3, 4, 6, 9].index(level)]
    struct.pack_into('<IIiiII', s, MAX_CHAIN, chain, lazy, level, r.choice([0, 0, 1, 2]), good, nice)
    struct.pack_into('<IIII', s, L_BUF, PEND + 3 * lit, lit, 0, PEND + lit)
    struct.pack_into('<IIII', s, OPT_LEN, 0, 0, 0, 8)
    descs(s)  # (the trees as _tr_init and init_block leave them)
    for tree, elems in ((DYN_LTREE, L_CODES), (DYN_DTREE, D_CODES), (BL_TREE, BL_CODES)):
        for i in range(elems):
            struct.pack_into('<H', s, tree + 4 * i, 1 if tree == DYN_LTREE and i == END_BLOCK else 0)
    window = text(r, 2 * w_size + WIN_SLACK)
    prev = bytearray(2 * w_size)
    head = bytearray(2 << hash_bits)
    if fresh:
        struct.pack_into('<iIIiIIII', s, BLOCK_START, 0, 2, 0, 0, 0, 0, 0, 2)
        struct.pack_into('<I', s, INS_H, 0)
    else:
        strstart = r.randrange(1, 2 * w_size - MIN_LOOKAHEAD)
        lookahead = r.randrange(0, 2 * w_size - strstart + 1)
        for i in range(w_size):
            struct.pack_into('<H', prev, 2 * i, r.choice([0, r.randrange(max(1, strstart))]))
        for i in range(1 << hash_bits):
            struct.pack_into('<H', head, 2 * i, r.choice([0, r.randrange(max(1, strstart))]))
        struct.pack_into('<iIIiIIII', s, BLOCK_START, r.randrange(-w_size, strstart + 1), r.randrange(2, 258), r.randrange(strstart + 1), r.choice([0, 1]),
                         strstart, r.randrange(strstart + 1), lookahead, r.randrange(2, 258))
        struct.pack_into('<I', s, INS_H, r.getrandbits(hash_bits))
    struct.pack_into('<HxxI', s, BI_BUF, 0, 0)
    n = r.choice([0, 1, 3, r.randrange(300), r.randrange(4 * min(w_size, 1024))])  # (BUF has 0x1000 bytes before LBUF)
    strm = struct.pack('<6I', BUF, n, r.getrandbits(16), OUT, r.choice([OUT_SIZE, r.randrange(1, 64), 0]), r.getrandbits(16)) + \
        struct.pack('<IIIIIIII', 0, S, 0, 0, 0, 2, r.getrandbits(32), 0)
    return s, {S: s, PEND: r.bytes(lit * 4), STRM: strm, OUT: bytes(OUT_SIZE), win: window, prv: prev, HEAD: head, BUF: text(r, max(n, 4))}


def put_short_msb(r):
    s = state(r)
    return Call(0x635710, [S, r.getrandbits(32)], regions(r, s))


def flush_pending(r):
    s, regs = deflating(r)
    pending = r.choice([0, 1, 2, 3, r.randrange(0x100), r.randrange(0x100)])
    struct.pack_into('<II', s, PENDING_OUT, PEND + r.randrange(0x40), pending)
    if r.random() < 0.6:  # (avail_out at or just under pending: all of it, or all but one or two bytes)
        regs[STRM] = regs[STRM][:16] + struct.pack('<I', max(0, pending - r.choice([0, 1, 1, 2]))) + regs[STRM][20:]
    return Call(0x635740, [STRM], regs)


def read_buf(r):
    s, regs = deflating(r)
    return Call(0x635ba0, [STRM, WIN + r.randrange(0x100), r.choice([0, 1, r.randrange(0x400)])], regs, ret='a')


def lm_init(r):
    s, regs = deflating(r)
    struct.pack_into('<i', s, LEVEL, r.randrange(10))
    return Call(0x635870, [S], regs)


def longest_match(r):
    s, regs = deflating(r)
    strstart = struct.unpack_from('<I', s, STRSTART)[0]
    # (configuration_table's chains, at least 4: one under 4 shifted right by good_match's >>= 2 is 0, which --chain_length wraps; 4096
    # links of long matches pass the emulator's step limit)
    struct.pack_into('<I', s, MAX_CHAIN, r.choice([4, 8, 16, 32, 128, 512]))
    if r.random() < 0.5:  # (a nice_match that matches reach, so one stops the chain at exactly it)
        struct.pack_into('<I', s, NICE_MATCH, r.choice([3, 4, 5, 8, 16, r.randrange(3, 40)]))
    w_size = struct.unpack_from('<I', s, W_SIZE)[0]
    if strstart >= 64 and r.random() < 0.3:
        return nice_chain(r, s, regs, strstart, w_size)
    return Call(0x635f80, [S, r.randrange(strstart) if strstart else 0], regs, ret='a')


def nice_chain(r, s, regs, strstart, w_size):
    """A chain of two: the first match exactly nice_match long (zlib stops there), the second longer."""
    nice, more = r.randrange(3, 21), r.randrange(1, 10)
    window, prev = regs[WIN], regs[PREV]
    lo = max(1, strstart - (w_size - MIN_LOOKAHEAD) + 1)  # (the chain goes on while above strstart - MAX_DIST)
    first, second = r.sample(range(lo, strstart - nice - more - 2), 2)
    for at, n in ((second, nice + more), (first, nice)):  # (the second first: the first's bytes win where the two overlap)
        window[at:at + n] = window[strstart:strstart + n]
        window[at + n] = window[strstart + n] ^ 1
    struct.pack_into('<H', prev, 2 * (first & (w_size - 1)), second)
    struct.pack_into('<H', prev, 2 * (second & (w_size - 1)), 0)
    struct.pack_into('<I', s, NICE_MATCH, nice)
    struct.pack_into('<I', s, LOOKAHEAD, r.randrange(MIN_LOOKAHEAD, 2 * w_size - strstart + 1))
    struct.pack_into('<I', s, PREV_LENGTH, MIN_MATCH - 1)
    return Call(0x635f80, [S, first], regs, ret='a')


def fill_window(r):
    s, regs = deflating(r, fresh=r.random() < 0.3)
    if r.random() < 0.3:  # (strstart at the slide's threshold, wsize + MAX_DIST)
        w_size = struct.unpack_from('<I', s, W_SIZE)[0]
        strstart = 2 * w_size - MIN_LOOKAHEAD + r.choice([-1, 0, 0, 1])
        struct.pack_into('<I', s, STRSTART, strstart)
        struct.pack_into('<I', s, LOOKAHEAD, r.randrange(2 * w_size - strstart + 1))
    return Call(0x635a70, [S], regs)


def deflate_block(address):
    """A new stream or one part way through (sometimes with fewer than MIN_MATCH bytes to start on). The exe's deflate_fast and
    deflate_slow execute CPUID (leaf 0) in their prologue, inside PUSHAD / POPAD, its EAX into bflush's slot, which is written before it
    is read: dead, but the emulator needs the fake CPU to run it (docs/porting.md, "Library functions")."""
    def op(r):
        if address == 0x636100 and r.random() < 0.25:
            return too_far(r)
        fresh = r.random() < 0.5
        s, regs = deflating(r, fresh)
        if r.random() < 0.2:
            struct.pack_into('<I', s, LOOKAHEAD, r.randrange(3))
            regs[STRM] = regs[STRM][:4] + struct.pack('<I', r.randrange(3)) + regs[STRM][8:]
        if not fresh:  # (between calls no match is pending, except deflate_slow's lazy one: up to lookahead + 1 long, within MAX_DIST)
            strstart, lookahead, w_size = (struct.unpack_from('<I', s, o)[0] for o in (STRSTART, LOOKAHEAD, W_SIZE))
            if address == 0x636100 and strstart >= 2 and lookahead >= 2 and r.random() < 0.5:
                struct.pack_into('<IIII', s, MATCH_LENGTH, r.randrange(3, min(MAX_MATCH, lookahead + 1) + 1), 0, 1, strstart)
                struct.pack_into('<I', s, MATCH_START, r.randrange(max(0, strstart - 1 - (w_size - MIN_LOOKAHEAD)), strstart - 1))
            else:
                struct.pack_into('<I', s, MATCH_LENGTH, MIN_MATCH - 1)
            if address == 0x635910 and lookahead >= 2 and r.random() < 0.4:  # (the block at MAX_DIST: deflate_stored flushes there)
                lookahead = r.choice([2, 2, lookahead])  # (2: the least it moves on without refilling first)
                struct.pack_into('<I', s, LOOKAHEAD, lookahead)
                struct.pack_into('<i', s, BLOCK_START, strstart + lookahead - (w_size - MIN_LOOKAHEAD) + r.choice([-1, 0, 0, 1]))
            if address == 0x635c20 and r.random() < 0.5:  # (levels 1-3's max_insert_length, so matches of exactly it are common)
                struct.pack_into('<I', s, MAX_LAZY, r.choice([4, 5, 6]))
        struct.pack_into('<Ii', s, PENDING_OUT, PEND, 0)  # (deflate() flushes before it compresses: nothing pending, or no room)
        regs[CPU] = cpu_region(1, [(r.choice([1, 2, 3, 0xd, 0x16, 0x20]), 0x756e6547, 0x6c65746e, 0x49656e69)])
        return Call(address, [S, r.choice([0, 0, 2, 3, 4])], regs, ret='a')  # Z_NO_FLUSH, Z_SYNC_FLUSH, Z_FULL_FLUSH, Z_FINISH
    op.__name__ = 'deflate_%x' % address
    return op


def too_far(r):
    """deflate_slow at windowBits 13 with the next string's hash chain holding one match of exactly MIN_MATCH bytes about TOO_FAR back
    (zlib drops a 3-byte match further than that). The other chains are empty and the lookahead short, so no other match is that far:
    at this memLevel many distant matches make the block's codes overrun d_buf in pending_buf (zlib's overlay; CVE-2018-25032) and
    d_code read past _dist_code, where the exe's and the library's neighbours differ. The game's codec (memLevel 9) never gets there."""
    s, regs = deflating(r, big=True)
    regs[PREV_BIG][:] = bytes(len(regs[PREV_BIG]))
    regs[HEAD][:] = bytes(len(regs[HEAD]))
    regs[STRM] = regs[STRM][:4] + struct.pack('<I', 0) + regs[STRM][8:]
    w_size, hash_shift, hash_mask, ins_h = (struct.unpack_from('<I', s, o)[0] for o in (W_SIZE, HASH_SHIFT, HASH_MASK, INS_H))
    strstart = r.randrange(TOO_FAR + 8, 2 * w_size - MIN_LOOKAHEAD)
    match = strstart - TOO_FAR + r.choice([-1, 0, 0, 1])
    window, prev, head = regs[WIN_BIG], regs[PREV_BIG], regs[HEAD]
    window[match:match + 3] = window[strstart:strstart + 3]
    window[match + 3] = window[strstart + 3] ^ 1
    struct.pack_into('<H', prev, 2 * (match & (w_size - 1)), 0)  # (the chain ends there)
    struct.pack_into('<H', head, 2 * (((ins_h << hash_shift) ^ window[strstart + 2]) & hash_mask), match)
    struct.pack_into('<iIIiIIII', s, BLOCK_START, strstart - r.randrange(64), MIN_MATCH - 1, 0, 0, strstart, 0,
                     r.randrange(MIN_LOOKAHEAD, min(MIN_LOOKAHEAD + 400, 2 * w_size - strstart + 1)), MIN_MATCH - 1)
    struct.pack_into('<I', s, MAX_LAZY, r.choice([4, 16, 258]))
    struct.pack_into('<i', s, STRATEGY, 0)  # (Z_DEFAULT_STRATEGY: Z_FILTERED drops it anyway)
    struct.pack_into('<Ii', s, PENDING_OUT, PEND, 0)
    regs[CPU] = cpu_region(1, [(0xd, 0x756e6547, 0x6c65746e, 0x49656e69)])
    return Call(0x636100, [S, r.choice([0, 2, 4])], regs, ret='a')


deflate_stored, deflate_fast, deflate_slow = deflate_block(0x635910), deflate_block(0x635c20), deflate_block(0x636100)

OPS = [f for f in (adler32, init_block, set_data_type, bi_flush, bi_windup, copy_block, build_tree, pqdownheap, gen_bitlen, gen_codes, scan_tree,
                   send_tree, build_bl_tree, send_all_trees, compress_block, tr_stored_block, tr_align, tr_flush_block, put_short_msb,
                   flush_pending, read_buf, lm_init, longest_match, fill_window, deflate_stored, deflate_fast, deflate_slow)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)

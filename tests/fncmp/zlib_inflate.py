# zlib 1.1.3's inflate side (src/game/lib/zlib: the library built from third_party/zlib-1.1.3) against the exe's copies of it: the
# decoding tables (inftrees.c), the codes' decoders (infcodes.c, inffast.c), the window's flush (infutil.c) the blocks (infblock.c)
# and inflate (inflate.c).
# A z_stream at Z whose zalloc / zfree are fncmp's ZALLOC / ZFREE over the heap at HEAP.
# usage: zlib_inflate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=huft_build,...)
#
# Masks (fncmp.Mask): the +2 and +3 bytes of every inflate_huft in the table space (hp). huft_build builds each entry in a local
# `struct inflate_huft_s r` and stores it whole (`q[j] = r`, `u[h-1][j] = r`); r's union is {Byte Exop, Bits} | uInt pad, and only
# Exop and Bits are ever set, so +2 / +3 are whatever was on the stack: the exe's 0x64a0e2 / 0x64a0f3 / 0x64a13e ... write the bytes
# at [ESP + 0x30] / [ESP + 0x31] and 0x64a10d / 0x64a110 (and the other stores) copy the dwords at [ESP + 0x30] and [ESP + 0x34].
# Nothing reads them: every reader of an entry takes Exop and Bits as bytes and base as a dword at +4 (inflate_fast 0x64bc5f
# `MOV BL,[ESI + ECX*8]`, 0x64bc6f `MOV CL,[ESI + 1]`, 0x64bc8f `MOV EBX,[ESI + 4]`; inflate_codes and inflate_blocks the same), and
# huft_build itself never reads the table space. The emulator checks it on every case: a read of a masked byte fails the case.
import os
import struct
import sys
import zlib

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, ZFREE, Bench, Call, Mask, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {
    'huft_build': (0x649e10, 0x64a2c0), 'inflate_trees_bits': (0x649d60, 0x649e10), 'inflate_trees_dynamic': (0x64a2c0, 0x64a450),
    'inflate_codes_new': (0x649550, 0x649590), 'inflate_codes': (0x649590, 0x649d40), 'inflate_codes_free': (0x649d40, 0x649d60),
    'inflate_flush': (0x64a480, 0x64a5c0), 'inflate_fast': (0x64bbd0, 0x64bf80),
    'inflate_blocks_reset': (0x640690, 0x640710), 'inflate_blocks_new': (0x640710, 0x6407c0), 'inflate_blocks': (0x6407c0, 0x6414b0),
    'inflate_blocks_free': (0x6414b0, 0x641500), 'inflateReset': (0x636560, 0x6365b0), 'inflateEnd': (0x6365b0, 0x636600),
    'inflateInit2_': (0x636600, 0x636710), 'inflate': (0x636710, 0x636b70),
}
# (and the exe's inflate_trees_fixed, which inflate_blocks calls, and adler32, which inflateInit2_'s blocks reset calls)
B = Bench(sorted(list(FUNCS.values()) + [(0x64a450, 0x64a480), (0x63ea00, 0x63eb30)]))
for a in (0x636600, 0x636710, 0x6365b0):  # (fncmp's zlib stand-ins for the codec's benches: here the exe's own)
    del B.emu.hooks[a]
Z, OUTS, LENS, V, HP = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x800, RAM + 0x10000
MANY = 1440
CPLENS, CPLEXT, CPDIST, CPDEXT = 0x704f08, 0x704f88, 0x705008, 0x705080  # (the exe's: inflate_trees_dynamic pushes them)
MSGS = [0, 0x779b80, RAM + 0x1234]  # (strm->msg before the call: none, one of the exe's, anything)


def stream(r):
    """A z_stream: input and output are not read here; msg, zalloc / zfree / opaque are."""
    return struct.pack('<14I', RAM + 0x5000, r.getrandbits(8), r.getrandbits(16), RAM + 0x6000, r.getrandbits(8), r.getrandbits(16),
                       r.choice(MSGS), 0, ZALLOC, ZFREE, r.getrandbits(32), 0, r.getrandbits(32), 0)


def code_lengths(r, n, maxlen, used=None, complete=3):
    """n code lengths: a complete code over `used` (or a random number of) symbols (a random leaf split until there are enough, so the
    Kraft sum is exactly 1), then sometimes made incomplete (a leaf dropped), oversubscribed (one added), a single code, all zero, or
    lengths at random."""
    kind = r.choice(['complete'] * complete + ['incomplete', 'over', 'single', 'zero', 'random'])
    out = [0] * n
    if kind == 'zero':
        return out, kind
    if kind == 'random':
        return [r.choice([0, r.randrange(maxlen + 1)]) for _ in range(n)], kind
    if kind == 'single':
        out[r.randrange(n)] = r.choice([1, 1, r.randrange(1, maxlen + 1)])
        return out, kind
    k = min(n, used or r.choice([2, 3, r.randrange(2, max(n, 2) + 1), n]))
    leaves = [1, 1]
    while len(leaves) < k:
        i = r.choice([j for j, l in enumerate(leaves) if l < maxlen] or [None])
        if i is None:
            break
        if r.random() < 0.6:  # (deep: split the deepest splittable leaf, for long codes and second-level tables)
            i = max((j for j, l in enumerate(leaves) if l < maxlen), key=lambda j: leaves[j])
        leaves[i:i + 1] = [leaves[i] + 1, leaves[i] + 1]
    if kind == 'incomplete' and len(leaves) > 1:
        leaves.pop(r.randrange(len(leaves)))
    if kind == 'over':
        leaves.append(r.randrange(1, maxlen + 1))
    for sym, l in zip(r.sample(range(n), min(n, len(leaves))), leaves):
        out[sym] = l
    return out, kind


def entry_masks(region, size):
    return [Mask(region, [o for i in range(0, size, 8) for o in (i + 2, i + 3)],
                 'inflate_huft union padding: copied from huft_build\'s unset local r, read by no code (header)')]


def huft_build(r):
    which = r.choice(['bits', 'lit', 'dist'])
    if which == 'bits':
        n, s, d, e, maxlen, m = 19, 19, 0, 0, 7, 7
    elif which == 'lit':
        n, s, d, e, maxlen, m = r.randrange(257, 289), 257, CPLENS, CPLEXT, 15, 9
    else:
        n, s, d, e, maxlen, m = r.randrange(1, 31), 0, CPDIST, CPDEXT, 15, 6
    lens, _ = code_lengths(r, n, maxlen)
    m = r.choice([m, m, r.randrange(1, 16), 0])
    hn = r.choice([0, r.randrange(MANY - 0x200, MANY + 1), r.randrange(MANY - 0x60, MANY + 1),  # (from the end: Z_MEM_ERROR)
                   MANY - (1 << r.randrange(10))])  # (a first table that just fits)
    size = 8 * (MANY - hn)
    outs = struct.pack('<3I', r.getrandbits(32), m, hn)
    regions = {OUTS: outs, LENS: struct.pack('<%dI' % n, *lens), V: r.bytes(288 * 4), HP + 8 * hn: r.bytes(max(size, 8))}
    return Call(0x649e10, [LENS, n, s, d, e, OUTS, OUTS + 4, HP, OUTS + 8, V], regions, ret='a', masks=entry_masks(HP + 8 * hn, size))


def trees_bits(r):
    lens, _ = code_lengths(r, 19, 7)
    outs = struct.pack('<2I', r.choice([7, 7, r.randrange(1, 8)]), r.getrandbits(32))
    regions = {Z: stream(r), OUTS: outs, LENS: struct.pack('<19I', *lens), HP: r.bytes(0x400),
               HEAP: heap(0x400, r.choice([0] * 9 + [1]))}
    return Call(0x649d60, [LENS, OUTS, OUTS + 4, HP, Z], regions, ret='a', masks=entry_masks(HP, 0x400))


def trees_dynamic(r):
    empty = r.random() < 0.12  # (no distance codes, but lengths: "empty distance tree with lengths", unless nl is 257)
    nl, nd = 257 if empty and r.random() < 0.5 else r.randrange(257, 287), r.randrange(1, 31)
    ll, kind = code_lengths(r, nl, 15, complete=12)
    if kind != 'zero' and not ll[256] and r.random() < 0.8:  # (a real block has an end-of-block code: one symbol's length moved there)
        j = r.choice([i for i, l in enumerate(ll) if l])
        ll[256], ll[j] = ll[j], 0
    dl, _ = code_lengths(r, nd, 15, complete=12)
    if empty:
        dl = [0] * nd
    outs = struct.pack('<4I', r.choice([9, 9, r.randrange(1, 16)]), r.choice([6, 6, r.randrange(1, 16)]), r.getrandbits(32), r.getrandbits(32))
    regions = {Z: stream(r), OUTS: outs, LENS: struct.pack('<%dI' % (nl + nd), *(ll + dl)), HP: r.bytes(8 * MANY),
               HEAP: heap(0x800, r.choice([0] * 9 + [1]))}
    return Call(0x64a2c0, [nl, nd, LENS, OUTS, OUTS + 4, OUTS + 8, OUTS + 12, HP, Z], regions, ret='a', masks=entry_masks(HP, 8 * MANY))


# The codes: a raw deflate stream from Python's zlib (zlib-ng) whose first block is fixed or dynamic (its header read here), sometimes
# with bits flipped after the header and random bytes after its end; the tables are the exe's fixed_tl / fixed_td or the exe's
# inflate_trees_dynamic's (run here at case generation), so both runs read the same tables. The window is a power of two no smaller
# than the farthest distance the block reaches (decoded here, to its end of block or first bad code), so no copy reads outside it.
LBASE = [3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258]
LEXT = [0] * 8 + [1] * 4 + [2] * 4 + [3] * 4 + [4] * 4 + [5] * 4 + [0]
DBASE = [1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289,
         16385, 24577]
DEXT = [0, 0, 0, 0] + [i // 2 for i in range(2, 28)]
ORDER = [16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15]
FIXED_L, FIXED_D = [8] * 144 + [9] * 112 + [7] * 24 + [8] * 8, [5] * 30
FIXED_TL, FIXED_TD = 0x77a0a0, 0x77b0a0  # (the exe's: 9 and 5 bits)
S, C, IN, W, OUT = RAM + 0x40, RAM + 0x80, RAM + 0x20000, RAM + 0x30000, RAM + 0x40000
START, CODES = 0, 6  # (inflate_codes' mode, inflate_blocks')


class Huff:
    def __init__(self, lengths):
        self.counts = [0] * 16
        for n in lengths:
            self.counts[n] += 1
        self.counts[0] = 0
        self.symbols = [i for n in range(1, 16) for i, x in enumerate(lengths) if x == n]


def getbits(data, p, n):
    if p + n > 8 * len(data):
        raise EOFError
    return int.from_bytes(data[p >> 3:((p + n + 7) >> 3) + 1], 'little') >> (p & 7) & ((1 << n) - 1), p + n


def decode(data, p, h):  # (canonical codes, a bit at a time: as puff.c)
    code = first = index = 0
    for n in range(1, 16):
        b, p = getbits(data, p, 1)
        code |= b
        if code - h.counts[n] < first:
            return h.symbols[index + code - first], p
        index, first = index + h.counts[n], (first + h.counts[n]) << 1
        code <<= 1
    raise ValueError('no such code')


def header(data, p=0):
    """The literal / length and distance code lengths of the block at bit p and the bit after its header (None: a stored block)."""
    btype, p = getbits(data, p + 1, 2)
    if btype == 1:
        return FIXED_L, FIXED_D, p
    if btype != 2:
        return None
    nl, p = getbits(data, p, 5)
    nd, p = getbits(data, p, 5)
    nc, p = getbits(data, p, 4)
    nl, nd = nl + 257, nd + 1
    cl = [0] * 19
    for i in range(nc + 4):
        cl[ORDER[i]], p = getbits(data, p, 3)
    h, out = Huff(cl), []
    while len(out) < nl + nd:
        x, p = decode(data, p, h)
        if x < 16:
            out.append(x)
            continue
        n, p = getbits(data, p, (2, 3, 7)[x - 16])
        out += [out[-1] if x == 16 else 0] * ((3, 3, 11)[x - 16] + n)
    return out[:nl], out[nl:nl + nd], p


def farthest(data, p, ll, dl):
    """The farthest distance the block's symbols from bit p reach back, up to its end of block, a bad code or the end of the data;
    and the bit after its end of block (None: it has none)."""
    hl, hd, far = Huff(ll), Huff(dl), 0
    try:
        while True:
            x, p = decode(data, p, hl)
            if x < 256:
                continue
            if x == 256 or x > 285:
                return far, p if x == 256 else None
            _, p = getbits(data, p, LEXT[x - 257])
            d, p = decode(data, p, hd)
            if d >= 30:
                return far, None
            e, p = getbits(data, p, DEXT[d])
            far = max(far, DBASE[d] + e)
    except (EOFError, ValueError):
        return far, None


def payload(r, n):
    alpha = r.choice([b'ab', b'abcdefgh', bytes(range(256)), b'the quick brown fox '])
    out = bytearray()
    while len(out) < n:
        if out and r.random() < 0.4:
            d = r.randrange(1, len(out) + 1)
            for _ in range(r.randrange(3, 80)):
                out.append(out[-d])
        else:
            out += bytes(r.choice(alpha) for _ in range(r.randrange(1, 12)))
    return bytes(out[:n])


def deflated(r):
    """A raw deflate stream (flipped bits sometimes, random bytes after it) and its first block's header: (data, ll, dl, p)."""
    while True:
        strategy = r.choice([zlib.Z_DEFAULT_STRATEGY] * 3 + [zlib.Z_FIXED, zlib.Z_HUFFMAN_ONLY, zlib.Z_RLE, zlib.Z_FILTERED])
        z = zlib.compressobj(r.randrange(1, 10), zlib.DEFLATED, -r.choice([9, 10, 11, 12, 15]), 8, strategy)
        data = bytearray(z.compress(payload(r, r.randrange(1, 1 << r.randrange(4, 12)))) + z.flush())
        try:
            h = header(bytes(data))
        except (EOFError, ValueError):
            continue
        if h:
            break
    ll, dl, p = h
    if r.random() < 0.25:
        for _ in range(r.choice([1, 1, 2, 8])):
            if 8 * len(data) > p:
                i = r.randrange(p, 8 * len(data))
                data[i >> 3] ^= 1 << (i & 7)
    data += r.bytes(r.choice([0, r.randrange(16), r.randrange(300)]))
    return bytes(data), ll, dl, p


def tables(r, ll, dl):
    """(bl, bd, tl, td, the table space's bytes or None): the exe's fixed tables, or the exe's inflate_trees_dynamic's (None: it failed)."""
    if ll is FIXED_L:
        return 9, 5, FIXED_TL, FIXED_TD, None
    nl, nd = len(ll), len(dl)
    c = Call(0x64a2c0, [nl, nd, LENS, OUTS, OUTS + 4, OUTS + 8, OUTS + 12, HP, Z],
             {Z: stream(r), OUTS: struct.pack('<4I', 9, 6, 0, 0), LENS: struct.pack('<%dI' % (nl + nd), *(ll + dl)), HP: bytes(8 * MANY),
              HEAP: heap(0x800)}, ret='a')
    out = B.original(c)
    if isinstance(out, str) or int(out[0], 16):
        return None
    bl, bd, tl, td = struct.unpack('<4I', bytes.fromhex(out[4]))
    hp = bytes.fromhex(out[6])
    used = max(i for i in range(0, len(hp), 8) if any(hp[i:i + 8])) + 8
    return bl, bd, tl, td, hp[:used]


def window_ends(r, wsize, fast=False):
    """The window's read / write offsets: anywhere (the end too); for inflate_fast, 258 bytes of room from write."""
    if fast:
        wo = r.choice([0, r.randrange(wsize - 257)])  # (0: the stream's first copy can reach back to the window's start exactly)
        return r.choice([wo, r.randrange(wo + 1)] + ([r.randrange(wo + 259, wsize + 1)] if wo + 259 <= wsize else [])), wo
    pick = lambda: r.choice([0, wsize, r.randrange(wsize + 1), r.randrange(wsize + 1)])  # noqa: E731
    wo = pick()
    return r.choice([wo, pick()]), wo


def codes_case(r, fast=False):
    """A decoding state at the start of the first block's codes: the regions, and (bl, bd, tl, td)."""
    while True:
        data, ll, dl, p = deflated(r)
        t = tables(r, ll, dl)
        if t:
            break
    bl, bd, tl, td, hp = t
    far, _ = farthest(data, p, ll, dl)
    wsize = max(1 << max(far - 1, 0).bit_length(), 1 << r.choice([8, 9, 10, 11, 12] if not fast else [9, 10, 11]))
    k = (-p) % 8
    b = data[p >> 3] >> (p & 7) if k else 0
    nxt = (p + 7) >> 3
    for _ in range(r.choice([0, 0, 1, 2])):  # (whole bytes already in the bit buffer)
        if nxt < len(data) and k <= 16:
            b, k, nxt = b | data[nxt] << k, k + 8, nxt + 1
    left = len(data) - nxt
    avail = r.choice([left, left, r.randrange(left + 1)]) if not fast else r.randrange(min(10, left), left + 1)
    ro, wo = window_ends(r, wsize, fast)
    out = r.choice([0, r.randrange(1, 64), r.randrange(0x1000)])
    regions = {Z: struct.pack('<14I', IN + nxt, avail, r.getrandbits(16), OUT, out, r.getrandbits(16), r.choice(MSGS), 0, ZALLOC, ZFREE,
                              r.getrandbits(32), r.getrandbits(2), r.getrandbits(32), 0),
               S: struct.pack('<16I', CODES, C, *[r.getrandbits(32) for _ in range(4)], r.getrandbits(1), k, b, HP, W, W + wsize, W + ro, W + wo,
                              r.choice([0, recorder(0, 3, cdecl=True)]), r.getrandbits(32)),
               C: struct.pack('<4I2BH2I', START, r.getrandbits(16), r.getrandbits(32), r.getrandbits(32), bl, bd, r.getrandbits(16), tl, td),
               IN: data, W: r.bytes(wsize), OUT: r.bytes(out + 16), LOG: log_region()}
    if hp:
        regions[HP] = hp
    return regions, (bl, bd, tl, td)


def resumed(r, regions, address=0x649590, args=None):
    """The state after the exe's inflate_codes (or inflate_blocks) ran on part of the input into part of the room (a state inside a symbol, a copy, ...),
    then given the rest of the input (or part of it) and a fresh output buffer: None if that run crashed."""
    z = bytearray(regions[Z])
    struct.pack_into('<I', z, 4, r.randrange(struct.unpack_from('<I', z, 4)[0] + 1))
    pre = {**regions, Z: bytes(z)}
    out = B.original(Call(address, args or [S, Z, r.choice([0, -5])], pre, ret='a'))
    if isinstance(out, str):
        return None
    after = {k: bytearray.fromhex(v) for k, v in zip(pre, out[3:])}
    nxt = struct.unpack_from('<I', after[Z], 0)[0] - IN
    left = len(regions[IN]) - nxt
    room = r.choice([0, r.randrange(1, 64), r.randrange(0x1000)])
    if not struct.unpack_from('<I', after[Z], 0)[0]:
        return None
    struct.pack_into('<5I', after[Z], 0, IN + nxt, r.choice([left, left, r.randrange(left + 1)]), struct.unpack_from('<I', after[Z], 8)[0], OUT, room)
    after[OUT] = r.bytes(room + 16)
    after[LOG] = log_region()
    return after


def codes(r):
    regions, _ = codes_case(r)
    if r.random() < 0.6:
        regions = resumed(r, regions) or regions
    return Call(0x649590, [S, Z, r.choice([0, 0, -5])], regions, ret='a')


def fast(r):
    regions, (bl, bd, tl, td) = codes_case(r, fast=True)
    return Call(0x64bbd0, [bl, bd, tl, td, S, Z], regions, ret='a')


def flush(r):
    wsize = 1 << r.randrange(4, 12)
    ro, wo = window_ends(r, wsize)
    out = r.choice([0, r.randrange(1, 64), r.randrange(wsize + 16)])
    z = bytearray(stream(r))
    struct.pack_into('<2I', z, 0xc, OUT, out)
    regions = {Z: z, S: struct.pack('<16I', CODES, C, *[r.getrandbits(32) for _ in range(8)], W, W + wsize, W + ro, W + wo,
                                    r.choice([0, recorder(0, 3, cdecl=True)]), r.getrandbits(32)),
               W: r.bytes(wsize), OUT: r.bytes(out + 16), LOG: log_region()}
    return Call(0x64a480, [S, Z, r.choice([0, -5, -5, 1, -3])], regions, ret='a')


def codes_new(r):
    regions = {Z: stream(r), HEAP: heap(0x100, r.choice([0] * 5 + [1]))}
    return Call(0x649550, [r.choice([9, r.getrandbits(8)]), r.choice([5, r.getrandbits(8)]), r.getrandbits(32), r.getrandbits(32), Z], regions,
                ret='a')


def codes_free(r):
    return Call(0x649d40, [r.choice([HEAP + 0x10, r.getrandbits(32)]), Z], {Z: stream(r), HEAP: heap(0x100)})


# The blocks: a whole raw deflate stream (several blocks sometimes: flushes part way, level 0's stored blocks), sometimes with bits
# flipped anywhere (bad block types, stored lengths, headers, codes) or random bytes from the start. The window: no smaller than the
# farthest distance any block reaches (decoded here) for an intact stream; 32 KB, more than any distance, for a damaged one (its
# headers are not decoded here as the exe would).
TYPE, BTREE, DTREE, BAD = 0, 4, 5, 9  # (inflate_blocks' modes)


def farthest_all(data):
    """The farthest distance any block of an intact stream reaches back."""
    p, far = 0, 0
    try:
        while True:
            last, _ = getbits(data, p, 1)
            btype, _ = getbits(data, p + 1, 2)
            if btype == 0:
                p = (p + 3 + 7) & ~7
                n, p = getbits(data, p, 16)
                p += 16 + 8 * n
            elif btype == 3:
                return far
            else:
                ll, dl, p = header(data, p)
                f, p = farthest(data, p, ll, dl)
                if p is None:
                    return max(far, f)
                far = max(far, f)
            if last:
                return far
    except (EOFError, ValueError, IndexError):  # (IndexError: a repeat first)
        return far


def crafted(r):
    """A block's header made here, then random bytes: a stored block's lengths (LEN with its top bit set sometimes, NLEN wrong
    sometimes), or a dynamic block's counts (past the 29 allowed sometimes) and bit length code (a complete code over symbols that
    include the repeats, so a repeat can come first)."""
    acc, n = 0, 0

    def put(v, bits):
        nonlocal acc, n
        acc, n = acc | v << n, n + bits
    put(r.getrandbits(1), 1)
    if r.random() < 0.5:
        put(0, 2)
        n = (n + 7) & ~7
        length = r.choice([r.getrandbits(16), r.getrandbits(8), 0x8000 | r.getrandbits(8), 0x8000 | r.getrandbits(8)])
        put(length, 16)
        put(length ^ 0xffff if r.random() < 0.8 else r.getrandbits(16), 16)
    else:
        put(2, 2)
        put(r.choice([r.randrange(30), 29, 30, 30, 31]), 5)
        put(r.choice([r.randrange(30), 29, 29, 30, 31]), 5)
        put(15, 4)
        cl, _ = code_lengths(r, 19, 7, used=r.randrange(3, 20), complete=12)
        if r.random() < 0.3:  # (a repeat or a length as likely as each other first)
            cl = [0] * 19
            cl[16] = cl[r.choice([0, 8, 17, 18])] = 1
        for i in range(19):
            put(cl[ORDER[i]], 3)
    return acc.to_bytes((n + 7) >> 3, 'little') + r.bytes(r.randrange(400))


def whole_stream(r):
    """A raw deflate stream (damaged sometimes) and whether it is intact."""
    if r.random() < 0.08:
        return r.bytes(r.randrange(1, 400)), False
    if r.random() < 0.2:
        return crafted(r), False
    z = zlib.compressobj(r.choice([0, r.randrange(1, 10)]), zlib.DEFLATED, -r.choice([9, 10, 11, 12, 15]), 8,
                         r.choice([zlib.Z_DEFAULT_STRATEGY] * 3 + [zlib.Z_FIXED, zlib.Z_HUFFMAN_ONLY, zlib.Z_RLE]))
    data = bytearray()
    for _ in range(r.choice([1, 1, 2, 3])):
        data += z.compress(payload(r, r.randrange(1, 1 << r.randrange(4, 12))))
        data += z.flush(r.choice([zlib.Z_NO_FLUSH, zlib.Z_SYNC_FLUSH, zlib.Z_FULL_FLUSH, zlib.Z_PARTIAL_FLUSH]))
    data += z.flush()
    intact = r.random() < 0.75
    if not intact:
        for _ in range(r.choice([1, 1, 2, 8])):
            i = r.randrange(8 * len(data))
            data[i >> 3] ^= 1 << (i & 7)
    return bytes(data + r.bytes(r.choice([0, r.randrange(16), r.randrange(300)]))), intact


def blocks_case(r):
    data, intact = whole_stream(r)
    wsize = max(1 << max(farthest_all(data) - 1, 0).bit_length(), 1 << r.randrange(8, 13)) if intact else 0x8000
    k, b, nxt = 0, 0, 0
    for _ in range(r.choice([0, 0, 0, 1, 2])):  # (whole bytes already in the bit buffer)
        if nxt < len(data):
            b, k, nxt = b | data[nxt] << k, k + 8, nxt + 1
    left = len(data) - nxt
    ro, wo = window_ends(r, wsize)
    out = r.choice([0, r.randrange(1, 64), r.randrange(0x1000)])
    return {Z: struct.pack('<14I', IN + nxt, r.choice([left, left, r.randrange(left + 1)]), r.getrandbits(16), OUT, out, r.getrandbits(16),
                           r.choice(MSGS), 0, ZALLOC, ZFREE, r.getrandbits(32), r.getrandbits(2), r.getrandbits(32), 0),
            S: struct.pack('<16I', TYPE, *[r.getrandbits(32) for _ in range(5)], r.getrandbits(1), k, b, HP, W, W + wsize, W + ro, W + wo,
                           r.choice([0, recorder(0, 3, cdecl=True)]), r.getrandbits(32)),
            HP: r.bytes(8 * MANY), IN: data, W: r.bytes(wsize), OUT: r.bytes(out + 16), LOG: log_region(),
            HEAP: heap(0x8000, r.choice([0] * 12 + [1, 2]))}


def blocks(r):
    regions = blocks_case(r)
    if r.random() < 0.6:
        regions = resumed(r, regions, 0x6407c0) or regions
    return Call(0x6407c0, [S, Z, r.choice([0, 0, -5])], regions, ret='a', masks=entry_masks(HP, 8 * MANY))


def blocks_state(r, mode):
    """A blocks state in mode (its union's pointers: a codes state, a bit length array, all hashed only by zfree)."""
    return struct.pack('<16I', mode, *[r.choice([HEAP + 0x10, r.getrandbits(32)]) for _ in range(5)], r.getrandbits(1), r.getrandbits(5),
                       r.getrandbits(32), HP, W, W + 0x100, W + r.randrange(0x101), W + r.randrange(0x101),
                       r.choice([0, recorder(0, 3, cdecl=True)]), r.getrandbits(32))


def blocks_reset(r):
    regions = {Z: stream(r), S: blocks_state(r, r.choice([TYPE, BTREE, DTREE, CODES, CODES, r.randrange(10)])), OUTS: r.bytes(8),
               HEAP: heap(0x100), LOG: log_region()}
    return Call(0x640690, [S, Z, r.choice([0, OUTS])], regions)


def blocks_new(r):
    w = r.choice([0x8000, 1 << r.randrange(8, 16), r.randrange(1, 0x8000)])
    regions = {Z: stream(r), HEAP: heap(0xc000, r.choice([0] * 4 + [1, 2, 3])), LOG: log_region()}
    return Call(0x640710, [Z, r.choice([0, recorder(0, 3, cdecl=True)]), w], regions, ret='a')


def blocks_free(r):
    regions = {Z: stream(r), S: blocks_state(r, r.choice([TYPE, BTREE, DTREE, CODES, r.randrange(10)])), HEAP: heap(0x100), LOG: log_region()}
    return Call(0x6414b0, [S, Z], regions, ret='a')


# inflate: a zlib stream (a raw one for nowrap; a preset dictionary sometimes, which this zlib only reports), damaged sometimes, on a
# state at its start or where the exe's inflate left it part way. The check function is a recorder, made to answer the stream's
# Adler-32 when it is intact (the driver cannot run the exe's adler32), so the trailer check passes or fails as the data says.
IS, VER = RAM + 0x140, RAM + 0x180
METHOD, BLOCKS, DONE, BADI = 0, 7, 12, 13  # (inflate's modes)


def inflate_case(r):
    nowrap = r.random() < 0.3
    wbits = r.choice([9, 10, 11, 12, 15])
    dict_ = False
    if r.random() < 0.06:
        data, intact = r.bytes(r.randrange(1, 300)), False
    else:
        dict_ = r.random() < 0.08 and not nowrap
        z = zlib.compressobj(r.choice([0, r.randrange(1, 10)]), zlib.DEFLATED, -wbits if nowrap else wbits, 8, zlib.Z_DEFAULT_STRATEGY,
                             **({'zdict': payload(r, 40)} if dict_ else {}))
        data = bytearray()
        for _ in range(r.choice([1, 1, 2])):
            data += z.compress(payload(r, r.randrange(1, 1 << r.randrange(4, 12))))
            data += z.flush(r.choice([zlib.Z_NO_FLUSH, zlib.Z_SYNC_FLUSH, zlib.Z_FULL_FLUSH]))
        data += z.flush()
        intact = r.random() < 0.75
        if not intact:
            for _ in range(r.choice([1, 1, 2, 8])):
                i = r.randrange(8 * len(data))
                data[i >> 3] ^= 1 << (i & 7)
        data = bytes(data)
    raw = data if nowrap else data[6:] if dict_ else data[2:]  # (the header, and a preset dictionary's id)
    wsize = max(1 << max(farthest_all(raw) - 1, 0).bit_length(), 1 << r.randrange(8, 13)) if intact else 0x8000
    log = log_region()
    if intact and not nowrap:
        set_return(log, 0, int.from_bytes(data[-4:], 'big'))
    data += r.bytes(r.choice([0, r.randrange(16)]))
    ro, wo = window_ends(r, wsize)
    out = r.choice([0, r.randrange(1, 64), r.randrange(0x1000)])
    return {Z: struct.pack('<14I', IN, r.choice([len(data), len(data), r.randrange(len(data) + 1)]), r.getrandbits(16), OUT, out,
                           r.getrandbits(16), r.choice(MSGS), IS, ZALLOC, ZFREE, r.getrandbits(32), r.getrandbits(2), r.getrandbits(32), 0),
            IS: struct.pack('<6I', BLOCKS if nowrap else METHOD, r.getrandbits(32), r.getrandbits(32), nowrap,
                            r.choice([15, 15, wbits, r.randrange(8, 16)]), S),
            S: struct.pack('<16I', TYPE, *[r.getrandbits(32) for _ in range(5)], r.getrandbits(1), 0, 0, HP, W, W + wsize, W + ro, W + wo,
                           r.choice([0, recorder(0, 3, cdecl=True)] if nowrap else [recorder(0, 3, cdecl=True)]), r.getrandbits(32)),
            HP: r.bytes(8 * MANY), IN: data, W: r.bytes(wsize), OUT: r.bytes(out + 16), LOG: log, HEAP: heap(0x8000, r.choice([0] * 12 + [1, 2]))}


FLUSHES = [0, 0, 4, 4, 2, 3]  # (Z_NO_FLUSH, Z_FINISH, Z_SYNC_FLUSH, Z_FULL_FLUSH)


def inflate(r):
    regions = inflate_case(r)
    if r.random() < 0.6:
        regions = resumed(r, regions, 0x636710, [Z, r.choice(FLUSHES)]) or regions
    z = bytearray(regions[Z])
    if r.random() < 0.03:  # (no state, no input: Z_STREAM_ERROR)
        struct.pack_into('<I', z, r.choice([0, 0x1c]), 0)
    regions[Z] = z
    return Call(0x636710, [Z, r.choice(FLUSHES)], regions, ret='a', masks=entry_masks(HP, 8 * MANY))


def inflate_state(r, blocks=S):
    return {Z: stream(r), IS: struct.pack('<6I', r.randrange(14), r.getrandbits(32), r.getrandbits(32), r.getrandbits(1), r.randrange(8, 16),
                                         blocks),
            S: blocks_state(r, r.choice([TYPE, BTREE, DTREE, CODES, r.randrange(10)])), HEAP: heap(0x100), LOG: log_region()}


def inflate_reset(r):
    regions = inflate_state(r)
    z = bytearray(regions[Z])
    struct.pack_into('<I', z, 0x1c, r.choice([IS] * 9 + [0]))
    regions[Z] = z
    return Call(0x636560, [Z], regions, ret='a')


def inflate_end(r):
    regions = inflate_state(r, r.choice([S, S, S, 0]))  # (no blocks state: as inflateInit2_ cleans up when inflate_blocks_new fails)
    z = bytearray(regions[Z])
    struct.pack_into('<I', z, 0x1c, r.choice([IS] * 9 + [0]))
    if r.random() < 0.08:
        struct.pack_into('<I', z, 0x24, 0)
    regions[Z] = z
    return Call(0x6365b0, [Z], regions, ret='a')


def inflate_init(r):
    """The game's own call is inflateInit2_(z, 15, "1.1.3", 0x38) through inflateInit_; here the window bits, the version and the size
    vary. zalloc / zfree are always given (zcalloc / zcfree stay the exe's: see docs/porting.md)."""
    w = r.choice([15, 15, -15, r.randrange(8, 16), -r.randrange(8, 16), r.choice([0, 7, 16, -7, -16, 0x7fffffff])])
    regions = {Z: stream(r), VER: r.choice([b'1.1.3\0'] * 4 + [b'1.2.11\0', b'2\0']), HEAP: heap(0x10000, r.choice([0] * 6 + [1, 2, 3, 4]))}
    return Call(0x636600, [Z, w, r.choice([VER] * 12 + [0]), r.choice([0x38] * 12 + [0x34])], regions, ret='a')






OPS = [f for f in (huft_build, trees_bits, trees_dynamic, codes, fast, flush, codes_new, codes_free, blocks, blocks_reset, blocks_new,
                   blocks_free, inflate, inflate_reset, inflate_end, inflate_init)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)

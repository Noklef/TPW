# zlib 1.1.3's inflate side (src/game/lib/zlib: the library built from third_party/zlib-1.1.3) against the exe's copies of it: the
# decoding tables (inftrees.c), the codes' decoders (infcodes.c, inffast.c) and the window's flush (infutil.c).
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
from fncmp import HEAP, LOG, RAM, ZALLOC, ZFREE, Bench, Call, Mask, heap, log_region, recorder  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {
    'huft_build': (0x649e10, 0x64a2c0), 'inflate_trees_bits': (0x649d60, 0x649e10), 'inflate_trees_dynamic': (0x64a2c0, 0x64a450),
    'inflate_codes_new': (0x649550, 0x649590), 'inflate_codes': (0x649590, 0x649d40), 'inflate_codes_free': (0x649d40, 0x649d60),
    'inflate_flush': (0x64a480, 0x64a5c0), 'inflate_fast': (0x64bbd0, 0x64bf80),
}
B = Bench(sorted(FUNCS.values()))
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


def header(data):
    """The first block's literal / length and distance code lengths and the bit after its header (None: a stored block)."""
    btype, p = getbits(data, 1, 2)
    if btype == 1:
        return FIXED_L, FIXED_D, 3
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
    """The farthest distance the block's symbols from bit p reach back, up to its end of block, a bad code or the end of the data."""
    hl, hd, far = Huff(ll), Huff(dl), 0
    try:
        while True:
            x, p = decode(data, p, hl)
            if x < 256:
                continue
            if x == 256 or x > 285:
                return far
            _, p = getbits(data, p, LEXT[x - 257])
            d, p = decode(data, p, hd)
            if d >= 30:
                return far
            e, p = getbits(data, p, DEXT[d])
            far = max(far, DBASE[d] + e)
    except (EOFError, ValueError):
        return far


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
        wo = r.randrange(wsize - 257)
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
    far = farthest(data, p, ll, dl)
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


def resumed(r, regions):
    """The state after the exe's inflate_codes ran on part of the input into part of the room (a state inside a symbol, a copy, ...),
    then given the rest of the input (or part of it) and a fresh output buffer: None if that run crashed."""
    z = bytearray(regions[Z])
    struct.pack_into('<I', z, 4, r.randrange(struct.unpack_from('<I', z, 4)[0] + 1))
    pre = {**regions, Z: bytes(z)}
    out = B.original(Call(0x649590, [S, Z, r.choice([0, -5])], pre, ret='a'))
    if isinstance(out, str):
        return None
    after = {k: bytearray.fromhex(v) for k, v in zip(pre, out[3:])}
    nxt = struct.unpack_from('<I', after[Z], 0)[0] - IN
    left = len(regions[IN]) - nxt
    room = r.choice([0, r.randrange(1, 64), r.randrange(0x1000)])
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


OPS = [f for f in (huft_build, trees_bits, trees_dynamic, codes, fast, flush, codes_new, codes_free)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)

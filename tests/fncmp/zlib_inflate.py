# zlib 1.1.3's inflate side (src/game/lib/zlib: the library built from third_party/zlib-1.1.3) against the exe's copies of it: the
# decoding tables (inftrees.c). A z_stream at Z whose zalloc / zfree are fncmp's ZALLOC / ZFREE over the heap at HEAP.
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

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, ZALLOC, ZFREE, Bench, Call, Mask, heap  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {
    'huft_build': (0x649e10, 0x64a2c0), 'inflate_trees_bits': (0x649d60, 0x649e10), 'inflate_trees_dynamic': (0x64a2c0, 0x64a450),
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
    hn = r.choice([0, r.randrange(MANY - 0x200, MANY + 1), r.randrange(MANY - 0x60, MANY + 1)])  # (from the end: Z_MEM_ERROR)
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


OPS = [f for f in (huft_build, trees_bits, trees_dynamic)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)

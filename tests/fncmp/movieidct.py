# The video library's 8-point inverse DCTs (src/game/gfx/movieidct: 0x6780c1 and 0x678224 with the constants at 0xfbc018, 0x67673c,
# 0x67687b and 0x676965 with those at 0xfbb598) against the originals, called in their register conventions (block in ESI, out in
# EDI for 0x67673c / 0x676965; tools/fncmp.py conv 'esi' / 'esiedi'): a random block (small, medium or any dwords, a third of the
# column ones with dwords 1-7 zero), the out buffer random, the scratch 0xfbc008-0xfbc017 random, the constants the game's
# (0xfbc018: Q31 0.7071, 0.5412, 1.3066, 0.3827, 1.5 * 2^52) or with random multipliers, and the rounding constant sometimes
# 1.5 * 2^23 (so that at 24-bit precision the low dword carries the rounded value) or random; a tenth of the cases built so that
# fild; fmul's product sits just above a 24-bit midpoint the double product lands on (rounding()).
# Block (0x67832e, cdecl (out, pitch)): 0xfbc000-0xfbc24c random but for the constants (as above) and the 8 input rows (as the blocks
# above), the pitch 8, 16, 32 or 8-64 dwords, out random over its 8 rows.
# usage: movieidct.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=DctColumn,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

CALLS = {'DctColumn': 0x6780c1, 'DctRow': 0x678224, 'Column': 0x67673c, 'Row': 0x67687b, 'RowShift': 0x676965, 'Block': 0x67832e}
B = Bench([(0x6780c1, 0x678224), (0x678224, 0x67832e), (0x67673c, 0x67687b), (0x67687b, 0x676965), (0x676965, 0x676a6e), (0x67832e, 0x6784c1)])
BLOCK, OUT = RAM, RAM + 0x1000
DCT, MOVIE = 0xfbc000, 0xfbb598  # (the out pointer, the scratch and the constants; the constants)


def constants(r):
    k, c1, c2, c3, rnd = struct.unpack('<iIIII', bytes.fromhex('9a79825ad48b0a3f753da73f15efc33e0000c059'))
    if r.random() < 0.25:
        k = r.getrandbits(32) - (1 << 31)
        c1, c2, c3 = r.f(2), r.f(2), r.f(2)
    rnd = r.choice([rnd, rnd, fb(12582912.0), r.f()])
    return struct.pack('<iIIII', k, c1, c2, c3, rnd)


def block(r, column):
    if column and r.random() < 0.3:
        return struct.pack('<i', r.getrandbits(32) - (1 << 31)) + bytes(28)
    kind = r.choice(['small', 'medium', 'any'])
    out = bytearray()
    for _ in range(8):
        v = r.randrange(-2048, 2048) if kind == 'small' else r.randrange(-(1 << 20), 1 << 20) if kind == 'medium' else r.getrandbits(32) - (1 << 31)
        if kind != 'any' and r.random() < 0.3:
            v = 0
        out += struct.pack('<i', v)
    return out


def rounding(r):
    """A block and constants where fild a; fmul c1 is a hair above a 24-bit midpoint that the double product a * c1 lands on exactly (so
    rounding the double product again would tie to even, down): c1's significand m odd, a = (2^30 + t) / m mod 2^31 (t 1 or 2, under
    the double's last place), the 55-bit product's kept 24 bits even. B = -A and the rounding constant 0, so U is the product alone."""
    while True:
        m = r.getrandbits(23) | (1 << 23) | 1
        t = r.choice([1, 2])
        a = ((1 << 30) + t) * pow(m, -1, 1 << 31) % (1 << 31)
        p = a * m
        if p.bit_length() == 55 and not (p >> 31) & 1:
            break
    s = [r.randrange(-2048, 2048) for _ in range(8)]
    s[3], s[5], s[7], s[1] = 0, a, 0, -a
    k, c1, c2, c3, _ = struct.unpack('<iIIII', constants(r))
    return struct.pack('<8i', *s), struct.pack('<iIIII', k, (127 << 23) | (m & 0x7fffff), c2, c3, 0)


def case(r):
    op = r.deal(OPS)
    if op == 'Block':
        pitch = r.choice([8, 0x10, 0x20, r.randrange(8, 0x41)])
        state = r.bytes(0x18) + constants(r) + r.bytes(0x120) + b''.join(block(r, False) for _ in range(8))
        return Call(CALLS[op], [OUT, pitch], {DCT: state, OUT: r.bytes(pitch * 4 * 7 + 0x20)}, ret='v')
    column = op in ('DctColumn', 'Column')
    regions = {BLOCK: block(r, column), OUT: r.bytes(0x120 if column else 0x20)}
    consts = constants(r)
    if r.random() < 0.1:
        regions[BLOCK], consts = rounding(r)
    if op.startswith('Dct'):
        regions[DCT] = r.bytes(4) + struct.pack('<I', OUT) + r.bytes(16) + consts
        return Call(CALLS[op], [BLOCK], regions, conv='esi', ret='v')
    regions[MOVIE] = consts
    return Call(CALLS[op], [BLOCK, OUT], regions, conv='esi' if op == 'Row' else 'esiedi', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

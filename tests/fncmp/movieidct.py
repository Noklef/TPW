# The video library's 8-point inverse DCTs (src/game/gfx/movieidct: 0x6780c1 and 0x678224 with the constants at 0xfbc018, 0x67673c,
# 0x67687b and 0x676965 with those at 0xfbb598) against the originals, called in their register conventions (block in ESI, out in
# EDI for 0x67673c / 0x676965; tools/fncmp.py conv 'esi' / 'esiedi'): a random block (small, medium or any dwords, a third of the
# column ones with dwords 1-7 zero), the out buffer random, the scratch 0xfbc008-0xfbc017 random, the constants the game's
# (0xfbc018: Q31 0.7071, 0.5412, 1.3066, 0.3827, 1.5 * 2^52) or with random multipliers, and the rounding constant sometimes
# 1.5 * 2^23 (so that at 24-bit precision the low dword carries the rounded value) or random.
# usage: movieidct.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=DctColumn,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

CALLS = {'DctColumn': 0x6780c1, 'DctRow': 0x678224, 'Column': 0x67673c, 'Row': 0x67687b, 'RowShift': 0x676965}
B = Bench([(0x6780c1, 0x678224), (0x678224, 0x67832e), (0x67673c, 0x67687b), (0x67687b, 0x676965), (0x676965, 0x676a6e)])
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


def case(r):
    op = r.deal(OPS)
    column = op in ('DctColumn', 'Column')
    regions = {BLOCK: block(r, column), OUT: r.bytes(0x120 if column else 0x20)}
    if op.startswith('Dct'):
        regions[DCT] = r.bytes(4) + struct.pack('<I', OUT) + r.bytes(16) + constants(r)
        return Call(CALLS[op], [BLOCK], regions, conv='esi', ret='v')
    regions[MOVIE] = constants(r)
    return Call(CALLS[op], [BLOCK, OUT], regions, conv='esi' if op == 'Row' else 'esiedi', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

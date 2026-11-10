# The C library's floating point output (src/game/core/crtfloat: $I10_OUTPUT 0x68fbd0, _fltout2 0x68c1c0, __dtold 0x68c250, _fptostr 0x68c120, _cftoe 0x6806c0, _cftof 0x680840,
# _cftog 0x680970, _cfltcvt 0x680a30) against the originals: the numbers about the
# rounding edges (runs of nines, powers of ten, denormals, NaNs and infinities), the digit counts past the cap of 0x15 and below 0, the output's bytes.
# usage: crt_fltout.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x68c120, 0x68c1b1), (0x68c1c0, 0x68c24b), (0x68c250, 0x68c30e), (0x68fbd0, 0x68ff52), (0x68f9d0, 0x68f9f3), (0x68fa00, 0x68fa70), (0x68fa70, 0x68faa0), (0x68faa0, 0x68fad0), (0x68ff60, 0x690218), (0x690220, 0x6902aa),
           (0x6806c0, 0x680733), (0x680740, 0x680832), (0x680840, 0x6808a6), (0x6808b0, 0x680966), (0x680970, 0x680a26), (0x680a30, 0x680a95), (0x680aa0, 0x680ac9), (0x67bb90, 0x67bed0)])
OUT, FLT, X, BUF, TEXT, WIDE = RAM, RAM + 0x40, RAM + 0x60, RAM + 0x80, RAM + 0xc0, RAM + 0x400
OPS = [n for n in ('i10', 'i10', 'i10', 'fltout', 'dtold', 'fptostr', 'cftoe', 'cftof', 'cftog', 'cftog', 'cfltcvt') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
VALUES = [0.0, 1.0, 0.5, 0.1, 0.25, 9.5, 9.99999, 99.5, 0.999999999, 0.9999999999999999, 1e-5, 1e15, 1e16, 1e17, 1e21, 1e22, 1e23, 1.5e300, 1e308, 1e-300, 1e-307, 5e-324, 2.2250738585072014e-308, 1e-320, 123456789.0, 1.7976931348623157e308, 3.0, 7.0, 10.0, 100.0, 1000.0]
SPECIAL = [0x7ff0000000000000, 0x7ff8000000000000, 0xfff8000000000000, 0x7ff0000000000001, 0xfff0000000000000, 0x7ff4000000000000, 0x8000000000000000, 0x0000000000000001, 0x000fffffffffffff, 0x8000000000000001]


def double_bits(r):
    if r.random() < 0.2:
        return r.choice(SPECIAL)
    if r.random() < 0.15:
        return r.getrandbits(64)
    x = r.choice(VALUES)
    if r.random() < 0.3:
        x *= r.uniform(0.5, 2)
    return struct.unpack('<Q', struct.pack('<d', -x if r.random() < 0.3 else x))[0]


def extended(r):
    """The ten bytes as _dtold leaves them (a double's value), or bits of no double."""
    if r.random() < 0.1:   # (exponent 0 with low bits alone; an infinity's high word with low bits)
        return r.choice([(1, 0, 0), (0xffffffff, 0, 0x8000), (1, 0x80000000, 0x7fff), (0xffffffff, 0x80000000, 0xffff)])
    if r.random() < 0.7:
        bits = double_bits(r)
        exponent, fraction = bits >> 52 & 0x7ff, bits & 0xfffffffffffff
        sign = bits >> 63 << 15
        if exponent == 0 and fraction == 0:
            return 0, 0, sign
        if exponent == 0x7ff:
            return fraction << 11 & 0xffffffff, (fraction >> 21 | 0x80000000), sign | 0x7fff
        mantissa, e = (fraction << 11 | 1 << 63, exponent + 0x3c00) if exponent else (fraction << 11, 0x3c01)
        while not mantissa >> 63:
            mantissa, e = mantissa << 1, e - 1
        return mantissa & 0xffffffff, mantissa >> 32, sign | e
    high = r.choice([0x80000000, 0xc0000000, 0xffffffff, 0x40000000, 0, 1, 0xfffffff0, r.getrandbits(32), r.getrandbits(32) | 0x80000000])
    low = r.choice([0, 0, 1, 0xffffffff, r.getrandbits(32)])
    return low, high, r.choice([0, 1, 0x7fff, 0xffff, 0x8000, 0x3fff, 0x3ffe, 0x4000, 0x7ffe, r.getrandbits(16), r.randrange(0x3000, 0x5000)])


def case(r):
    name = r.deal(OPS)
    if name == 'i10':
        low, high, top = extended(r)
        digits = r.choice([0, 1, 2, 3, 5, 6, 0x11, 0x14, 0x15, 0x16, 0x40, -1, -2, -3, -20, r.randrange(-30, 30)])
        return Call(0x68fbd0, [low, high, top | r.getrandbits(16) << 16, digits, r.choice([0, 0, 1, 1, 2, 3, r.getrandbits(32)]), OUT], {OUT: r.bytes(0x20)}, conv='c', ret='a')
    if name == 'fltout':
        bits = double_bits(r)
        return Call(0x68c1c0, [bits & 0xffffffff, bits >> 32, FLT, BUF], {FLT: r.bytes(0x10), BUF: r.bytes(0x20)}, conv='c', ret='a')
    if name in ('cftoe', 'cftof', 'cftog', 'cfltcvt'):   # (the buffer room for a double's 309 whole digits and the decimals)
        regions = {X: struct.pack('<Q', double_bits(r)), WIDE: r.bytes(0x200), 0x77e7e0: bytes([r.choice([0x2e, 0x2e, 0x2c])])}
        decimals, caps = r.choice([0, 0, 1, 2, 3, 5, 6, 0x10, 0x11, 0x14, -1]) & 0xffffffff, r.choice([0, 1, 0x100])
        if name == 'cftoe':
            return Call(0x6806c0, [X, WIDE, decimals, caps], regions, conv='c', ret='a')
        if name == 'cftof':
            return Call(0x680840, [X, WIDE, decimals], regions, conv='c', ret='a')
        if name == 'cftog':
            return Call(0x680970, [X, WIDE, decimals, caps], regions, conv='c', ret='a')
        return Call(0x680a30, [X, WIDE, r.choice(b'eEfgGa'), decimals, caps], regions, conv='c', ret='a')
    if name == 'dtold':
        return Call(0x68c250, [OUT, X], {OUT: r.bytes(0x10), X: struct.pack('<Q', double_bits(r))}, conv='c', ret='a')
    text = bytes(r.choice(b'0123456789' if r.random() < 0.3 else b'99990004569') for _ in range(r.choice([0, 1, 2, 3, 5, 8, 17, 20])))
    return Call(0x68c120, [BUF, r.choice([0, 1, 2, 3, 5, 8, 16, 17, 18, 0x20, -1, -2, -9]), FLT], {BUF: r.bytes(0x40), FLT: r.bytes(0xc) + struct.pack('<I', TEXT), TEXT: text + b'\0'}, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

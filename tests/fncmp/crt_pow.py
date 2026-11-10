# The C library's pow (src/game/core/crtpow: _frnd 0x6855b0, _d_inttype 0x686f60, _powhlp 0x686d30) against the originals: doubles of
# every class (zeros, denormals, integers odd and even, halves, near 1, big, infinities, quiet and signalling NaNs, random bits) under the
# game's single precision, double and extended (the control word compared after); the C library's infinity, NaN and -0 globals varied.
# usage: crt_pow.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x67b440, 0x67b4f0), (0x6855b0, 0x6855e0), (0x685ee0, 0x685f50), (0x686d30, 0x687000)])
OPS = [n for n in ('frnd', 'inttype', 'powhlp') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
RESULT = RAM
INF, NAN, MINUS_ZERO = 0x781198, 0x7811a0, 0x7811b8
BITS = [0, 1 << 63, 1, 0x000fffffffffffff, 0x0010000000000000, 0x3ff0000000000000, 0xbff0000000000000, 0x7ff0000000000000, 0xfff0000000000000,
        0x7ff8000000000000, 0xfff8000000000000, 0x7ff0000000000001, 0x7ff4000000000000, 0xfff4000000000123, 0x7fefffffffffffff]


def double(r):
    """A double's bits: one of every class, an integer (odd past 2^24 too), a half, near 1, or random bits."""
    pick = r.randrange(9)
    if pick == 0:
        return r.choice(BITS)
    if pick == 1:
        v = float(r.choice([r.randrange(-100, 100), r.randrange(1 << 24, 1 << 31) | 1, r.randrange(1 << 50, 1 << 53), 1 << r.randrange(80)]))
    elif pick == 2:
        v = r.randrange(-1000, 1000) + 0.5
    elif pick == 3:
        v = 1.0 + r.choice([1, -1]) * 2.0 ** -r.randrange(1, 60)
    elif pick == 4:
        v = r.uniform(-1e6, 1e6)
    elif pick == 5:
        v = r.uniform(-3, 3)
    else:
        return r.getrandbits(64)
    return struct.unpack('<Q', struct.pack('<d', -v if r.random() < 0.3 else v))[0]


def words(bits):
    return [bits & 0xffffffff, bits >> 32]


def globals_(r):
    """The C library's infinity, NaN and -0 (writable data: usually as the exe has them)."""
    pick = lambda d: struct.pack('<Q', d if r.random() < 0.7 else r.choice([r.getrandbits(64), 0x7ff4000000000001, 0xfff8000000000000, 0x3ff0000000000000]))
    return {INF: pick(0x7ff0000000000000), NAN: pick(0xfff8000000000000), MINUS_ZERO: pick(1 << 63)}


def case(r):
    name = r.deal(OPS)
    cw = r.choice([0x7f, 0x7f, 0x27f, 0x37f])
    regions = {RESULT: r.bytes(8)}
    if name == 'frnd':
        return Call(0x6855b0, words(double(r)), regions, conv='c', ret='dc', cw=cw)
    if name == 'inttype':
        return Call(0x686f60, words(double(r)), regions, conv='c', ret='wc', cw=cw)
    regions.update(globals_(r))
    x, y = double(r), double(r)
    special = [0x7ff0000000000000, 0xfff0000000000000]
    if r.random() < 0.6:   # (an infinity on one side: the paths it has)
        if r.random() < 0.5:
            y = r.choice(special)
            x = r.choice([x, 0x3ff0000000000000, 0xbff0000000000000, x ^ 1 << 63])
        else:
            x = r.choice(special)
    return Call(0x686d30, words(x) + words(y) + [RESULT], regions, conv='c', ret='wc', cw=cw)


if __name__ == '__main__':
    B.main(case)

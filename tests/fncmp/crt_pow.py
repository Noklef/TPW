# The C library's pow (src/game/core/crtpow: _CIpow 0x67c5d0 through its thunk, _frnd 0x6855b0, _d_inttype 0x686f60, _powhlp 0x686d30) against the originals: doubles of
# every class (zeros, denormals, integers odd and even, halves, near 1, big, infinities, quiet and signalling NaNs, random bits) under the
# game's single precision, double and extended (the control word compared after); the C library's infinity, NaN and -0 globals varied.
# usage: crt_pow.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x67b440, 0x67b4f0), (0x6855b0, 0x6855e0), (0x685ee0, 0x686043), (0x686d30, 0x687000),
           (0x67c5d0, 0x67c7ed), (0x686110, 0x686163), (0x686170, 0x6861f8), (0x68620e, 0x68621b), (0x686259, 0x6862fc),   # (_CIpow and its exits)
           (0x68e6f0, 0x68e7f0), (0x685a20, 0x685d50), (0x6856e0, 0x685a04), (0x686050, 0x686071), (0x6860b0, 0x686109), (0x686080, 0x6860a8),
           (0x685ea0, 0x685ed9), (0x685e00, 0x685e28), (0x686a00, 0x686a19), (0x5d60c0, 0x5d60c3)], hooks={0x682460: _recorder(2, 0, cdecl=True)})
OPS = [n for n in ('frnd', 'inttype', 'powhlp', 'pow', 'pow', 'pow') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
RESULT, PTD = RAM, RAM + 0x100
GETPTD = 2
INF, NAN, MINUS_ZERO = 0x781198, 0x7811a0, 0x7811b8
FASTFLAG, HALF, BIG, INDEFINITE, MATHERR_OFF = 0xfb1330, 0x77e7b0, 0x781350, 0x780f30, 0x781edc
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


def bits(v):
    return struct.unpack('<Q', struct.pack('<d', v))[0]


def pow_case(r, regions, cw):
    """_CIpow: x and y on the x87 (x in ST1); x positive, negative with integer and other y, zeros, infinities and NaNs, results past a double's
    range both ways; __fastflag, the 0.5, the infinity and the indefinite read live."""
    pick = r.randrange(8)
    if pick < 3:
        x, y = bits(r.choice([r.uniform(0, 4), r.uniform(0, 1e6), 2.0 ** r.uniform(-1000, 1000)])), bits(r.choice([r.uniform(-30, 30), float(r.randrange(-20, 20)), r.uniform(-3, 3)]))
    elif pick == 3:   # (a negative x: an integer y, odd or even, or not)
        x, y = bits(-r.uniform(0, 10)), bits(r.choice([float(r.randrange(-40, 40)), r.uniform(-5, 5), float(r.randrange(1 << 53) | 1)]))
    elif pick == 4:   # (past a double: overflow, underflow, denormal results)
        x = bits(r.choice([2.0, 10.0, 0.5, 1e-10, 1e10, r.uniform(1, 3)]) * r.choice([1, 1, -1]))
        y = bits(r.choice([float(r.randrange(-1200, 1200)), r.uniform(-1100, 1100), r.choice([1, -1]) * r.uniform(1020, 1090)]))
    elif pick == 5:   # (zeros on either side)
        x = r.choice([0, 1 << 63, double(r), 1, 1 << 63 | 1])
        y = r.choice([0, 1 << 63, double(r), bits(float(r.randrange(-9, 9))), 1, bits(r.uniform(-3, 3))])
    else:
        x, y = double(r), double(r)
        if r.random() < 0.5:
            x, y = r.choice([(x, r.choice([0x7ff0000000000000, 0xfff0000000000000, 0x7ff8000000000001, 0x7ff0000000000001])), (r.choice(
                [0x7ff0000000000000, 0xfff0000000000000, 0xfff8000000000000, 0x7ff4000000000000]), y)])
    regions[PTD] = bytes(0x40)
    log = log_region()
    set_return(log, GETPTD, PTD)
    regions[LOG] = log
    regions[FASTFLAG] = struct.pack('<I', 0 if r.random() < 0.85 else r.choice([1, r.getrandbits(32)]))
    regions[MATHERR_OFF] = struct.pack('<I', r.choice([0, 0, 1]))
    if r.random() < 0.1:
        regions[HALF] = struct.pack('<d', r.choice([0.25, 1.0, 0.5000000000000001]))
    for at in (BIG, INDEFINITE):   # (valid 80-bit encodings: the x87 takes an unnormal as invalid, the emulator does not model that)
        if r.random() < 0.1:
            regions[at] = struct.pack('<QH', r.getrandbits(64) | 1 << 63, r.randrange(1, 0x8000) | r.choice([0, 0x8000]))
    return Call(0x67c5d0, words(x) + words(y), regions, conv='st', ret='dc', cw=cw)


def case(r):
    name = r.deal(OPS)
    cw = r.choice([0x7f, 0x7f, 0x27f, 0x37f])
    regions = {RESULT: r.bytes(8)}
    if name == 'frnd':
        return Call(0x6855b0, words(double(r)), regions, conv='c', ret='dc', cw=cw)
    if name == 'inttype':
        return Call(0x686f60, words(double(r)), regions, conv='c', ret='wc', cw=cw)
    regions.update(globals_(r))
    if name == 'pow':
        return pow_case(r, regions, r.choice([0x7f, 0x27f, 0x27f, 0x37f]))
    x, y = double(r), double(r)
    special = [0x7ff0000000000000, 0xfff0000000000000]
    if r.random() < 0.6:   # (an infinity on one side: the paths it has)
        if r.random() < 0.5:
            y = r.choice(special)
            x = r.choice([x, 0x3ff0000000000000, 0xbff0000000000000, x ^ 1 << 63])
        else:
            x = r.choice(special)
            y = r.choice([y, y, 0, 1 << 63])   # (a zero y: 1, not 0)
    return Call(0x686d30, words(x) + words(y) + [RESULT], regions, conv='c', ret='wc', cw=cw)


if __name__ == '__main__':
    B.main(case)

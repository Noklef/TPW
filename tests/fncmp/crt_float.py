# The C library's floating point helpers (src/game/core/crtfloat: _abstract_cw 0x68bad0, _hw_cw 0x68bb70, __addl 0x68f9d0, _ZeroTail 0x68bc00, _IncMan 0x68bc70, _RoundMan 0x68bce0, _CopyMan 0x68bd80, _IsZeroMan 0x68bdb0,
# _ShrMan 0x68bdd0, _ld12cvt 0x68be90, _ld12tod 0x68c060, _ld12tof 0x68c080) against the originals: mantissas of edge patterns with noise after them, exponents at the formats' edges, the game's two format tables and made ones.
# usage: crt_float.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x68bad0, 0x68bb66), (0x68bb70, 0x68bbfb), (0x68bc00, 0x68bc66), (0x68bc70, 0x68bcd9), (0x68bce0, 0x68bd7d), (0x68bd80, 0x68bd9d), (0x68bda0, 0x68bdaf), (0x68bdb0, 0x68bdcd), (0x68bdd0, 0x68be81),
           (0x68be90, 0x68c05f), (0x68c060, 0x68c078), (0x68c080, 0x68c098), (0x68f9d0, 0x68f9f3)])
MAN, OUT, TEXT, FORMAT = (RAM + 0x40 * i for i in range(4))
OPS = [n for n in ('abstract', 'hw', 'addl', 'zerotail', 'incman', 'roundman', 'copyman', 'iszero', 'shrman', 'cvt', 'cvt', 'cvt', 'tod', 'tod', 'tof', 'tof') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
DOUBLE = (0x400, -0x3ff, 0x35, 0xb, 0x40, 0x3ff)
SINGLE = (0x80, -0x7f, 0x18, 8, 0x20, 0x7f)


def word(r):
    return r.choice([0, 0, 1, 0x80000000, 0x7fffffff, 0xffffffff, 0x400, 0x800, 0xfffff800, 0xc00, 0x100, 0x80, 0xffffff00, r.getrandbits(32), 1 << r.randrange(32)])


def mantissa(r, tail=True):
    return struct.pack('<3I', word(r), word(r), word(r)) + (r.bytes(8) if tail else b'')


def number(r, fmt):
    high, low, first = word(r), word(r), r.choice([0, 0, 0x8000, 0x7fff, 0xffff, r.getrandbits(16)])
    if r.random() < 0.7:
        high |= 0x80000000
    if r.random() < 0.05:
        high, low, first = 0, 0, 0
    if r.random() < 0.15:
        high, low, first = 0xffffffff, 0xffffffff, 0xffff    # (rounds up over the top)
    base = 0x3fff
    exponent = r.choice([0, 1, 0x7fff, 0x7ffe, base, base - 1, base + 1, base + fmt[0] + r.randrange(-3, 4), base + fmt[1] + r.randrange(-3, 4), base + fmt[1] - fmt[2] + r.randrange(-3, 4), base + r.randrange(fmt[1], fmt[0]), r.getrandbits(15)])
    if r.random() < 0.1:     # (a denormal's exact half after the shift, the last word's top bit keeping it from exact)
        shift = r.randrange(16, 20)
        bit = fmt[2] - shift
        words = [0x80000000, 0]
        words[bit // 32] |= 1 << (31 - bit % 32)
        high, low, first, exponent = words[0], words[1], r.choice([0x8000, 0, 0x4000]), base + fmt[1] - shift
    return struct.pack('<HIIH', first, low, high, exponent | r.choice([0, 0x8000])) + r.bytes(2)


def case(r):
    name = r.deal(OPS)
    regions = {MAN: mantissa(r), OUT: r.bytes(0x10)}
    if name in ('abstract', 'hw'):
        value = r.choice([r.getrandbits(32), r.getrandbits(16), 0x27f, 0x37f, 0x1f, 0x9001f, 0x10000, 0x20000, 0x30000, 0x40000, 0x80000, 0x100, 0x200, 0x300, 0x400, 0x800, 0xc00, 0x1000]) if name == 'hw' else r.choice([r.getrandbits(16), r.getrandbits(32), 0x27f, 0x37f, 0x400, 0x800, 0xc00, 0x100, 0x200, 0x300, 0x1000, 0x8000, 0x10000])
        return Call(0x68bad0 if name == 'abstract' else 0x68bb70, [value], regions, conv='c', ret='a')
    if name == 'addl':
        a = r.choice([0, 1, 0xffffffff, 0x80000000, r.getrandbits(32)])
        return Call(0x68f9d0, [a, r.choice([0, 1, 0xffffffff, 0x80000000, (-a) & 0xffffffff, r.getrandbits(32)]), OUT], regions, conv='c', ret='a')
    if name == 'zerotail':
        return Call(0x68bc00, [MAN, r.randrange(0, 97)], regions, conv='c', ret='a')
    if name == 'incman':
        return Call(0x68bc70, [MAN, r.randrange(0, 96)], regions, conv='c', ret='a')
    if name == 'roundman':
        bits = r.choice([0, 1, 31, 32, 33, 63, 64, 65, 95, 0x18, 0x35, 0x40, r.randrange(96)])
        if r.random() < 0.5:     # (a half just at the place, or just past it)
            words = [0, 0, 0]
            words[bits // 32] |= 1 << (31 - bits % 32)
            if r.random() < 0.5:
                at = r.randrange(bits, 96)
                words[at // 32] |= 1 << (31 - at % 32)
            if bits and r.random() < 0.5:
                at = r.randrange(bits)
                words[at // 32] |= 1 << (31 - at % 32)
            if r.random() < 0.3:
                words[0] = 0xffffffff
                words[1] |= 0xffffffff if bits > 32 else 0
            regions[MAN] = struct.pack('<3I', *words) + r.bytes(8)
        return Call(0x68bce0, [MAN, bits], regions, conv='c', ret='a')
    if name == 'copyman':
        return Call(0x68bd80, [OUT, MAN], regions, conv='c')
    if name == 'iszero':
        regions[MAN] = struct.pack('<3I', *[r.choice([0, 0, 0, 1, 0x80000000]) for _ in range(3)]) + r.bytes(8)
        return Call(0x68bdb0, [MAN], regions, conv='c', ret='a')
    if name == 'shrman':
        return Call(0x68bdd0, [MAN, r.choice([0, 1, 8, 11, 12, 31, 32, 33, 40, 63, 64, 65, 95, r.randrange(96)])], regions, conv='c')
    fmt = SINGLE if name == 'tof' else DOUBLE if name == 'tod' else r.choice([DOUBLE, SINGLE, (0x400, -0x3ff, 0x35, 0xb, 0x30, 0x3ff), (0x80, -0x7f, 0x18, 8, 0x40, 0x7f), (0x400, -0x3ff, 0x35, 0xb, 0x20, 0x3ff)])
    regions[TEXT] = number(r, fmt)
    regions[0x781728] = struct.pack('<6i', *DOUBLE) + struct.pack('<6i', *SINGLE)
    if name == 'cvt':
        regions[FORMAT] = struct.pack('<6i', *fmt)
        return Call(0x68be90, [TEXT, OUT, FORMAT], regions, conv='c', ret='a')
    return Call(0x68c060 if name == 'tod' else 0x68c080, [TEXT, OUT], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

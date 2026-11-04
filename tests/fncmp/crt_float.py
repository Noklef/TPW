# The C library's floating point helpers (src/game/core/crtfloat: _abstract_cw 0x68bad0, _hw_cw 0x68bb70, __addl 0x68f9d0, _ZeroTail 0x68bc00, _IncMan 0x68bc70, _RoundMan 0x68bce0, _CopyMan 0x68bd80, _IsZeroMan 0x68bdb0,
# _ShrMan 0x68bdd0, _ld12cvt 0x68be90, _ld12tod 0x68c060, _ld12tof 0x68c080, __add_12 0x68fa00, __shl_12 0x68fa70, __shr_12 0x68faa0, __mtold12 0x68fad0, __ld12mul 0x68ff60, __multtenpow12 0x690220, __strgtold12 0x68c440, _fltin2 0x6816a0, strtod 0x6d7710) against the originals: mantissas of edge patterns with noise after them, exponents at the formats' edges, the game's two format tables and made ones.
# usage: crt_float.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x68bad0, 0x68bb66), (0x68bb70, 0x68bbfb), (0x68bc00, 0x68bc66), (0x68bc70, 0x68bcd9), (0x68bce0, 0x68bd7d), (0x68bd80, 0x68bd9d), (0x68bda0, 0x68bdaf), (0x68bdb0, 0x68bdcd), (0x68bdd0, 0x68be81),
           (0x68be90, 0x68c05f), (0x68c060, 0x68c078), (0x68c080, 0x68c098), (0x68f9d0, 0x68f9f3),
           (0x68fa00, 0x68fa70), (0x68fa70, 0x68faa0), (0x68faa0, 0x68fad0), (0x68fad0, 0x68fbc1), (0x68ff60, 0x690218), (0x690220, 0x6902aa), (0x68c440, 0x68caa0), (0x6816a0, 0x681736), (0x6d7710, 0x6d7826),
           (0x680430, 0x6804e0), (0x686a00, 0x686a19)], hooks={0x682460: _recorder(2, 0, cdecl=True)})
GETPTD = 2
MAN, OUT, TEXT, FORMAT = (RAM + 0x40 * i for i in range(4))
ENDS, FLT, PTD, TEXT2, PCTYPE = RAM + 0x100, RAM + 0x120, RAM + 0x140, RAM + 0x200, RAM + 0x400
TWELVE = ('add12', 'shl12', 'shr12', 'mtold12', 'ld12mul', 'multtenpow', 'strgtold', 'fltin', 'strtod')
OPS = [n for n in ('abstract', 'hw', 'addl', 'zerotail', 'incman', 'roundman', 'copyman', 'iszero', 'shrman', 'cvt', 'cvt', 'cvt', 'tod', 'tod', 'tof', 'tof', 'add12', 'shl12', 'shr12', 'mtold12', 'mtold12', 'ld12mul', 'ld12mul', 'ld12mul', 'multtenpow', 'multtenpow', 'strgtold', 'strgtold', 'strgtold', 'strgtold', 'fltin', 'fltin', 'strtod', 'strtod', 'strtod') if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
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


def twelve_bytes(r):
    """A 12-byte number: exponents about the edges of a product's, mantissas normal, all ones, none or small."""
    low, high = word(r), word(r) | (0x80000000 if r.random() < 0.8 else 0)
    first = r.choice([0, 0x8000, 0xffff, 0x7fff, r.getrandbits(16)])
    if r.random() < 0.1:
        low, high, first = 0, 0, 0
    if r.random() < 0.15:
        low, high, first = 0xffffffff, 0xffffffff, 0xffff
    exponent = r.choice([0, 0, 1, 0x7ffe, 0x7fff, 0x3fff, 0x3ffe, 0x2000, 0x1fe0, 0x1ff0, 0x5ffe, 0x6000, r.randrange(0x1fd0, 0x2010), r.randrange(0x8000)])
    return struct.pack('<HIIH', first, low, high, exponent | r.choice([0, 0x8000]))


def number_text(r):
    """Every state of the reader: the white space, sign, whole part, point, fraction, exponent and what follows."""
    many = ''.join(r.choice('0123456789') for _ in range(r.randrange(20, 34)))
    whole = r.choice(['', '0', '00', '1', '12', '123', '9' * 30, '1' + '0' * 30, '1' * 23 + '49', '1' * 23 + '50', '1' * 23 + '99999', many])
    fraction = r.choice(['', '0', '000', '5', '25', '0005', '1' * 30, '0' * 30 + '1', many])
    text = r.choice(['', '', ' ', '\t', '\n\r  ']) + r.choice(['', '', '+', '-', '+-']) + whole + r.choice(['', '.', '.', ',', '..']) + fraction
    text += r.choice(['', '', 'e', 'E', 'd', 'D', 'q']) + r.choice(['', '+', '-', '+-']) + r.choice(['', '0', '000', '5', '10', '308', '309', '4932', '5199', '5200', '5201', '5202', '99999', '0005'])
    return text.encode() + r.choice([b'', b'', b'x', b' 1', b'+5', b'-5', b'.', b'e', b'+', b'-1', b'+x', b'-', b'x7']) + b'\0'


def number_world(r, regions):
    """The reader's globals: MB_CUR_MAX, _pctype's table (now and then a letter taken for a digit), the point, the text."""
    regions[TEXT2] = number_text(r)
    regions[ENDS] = r.bytes(4)
    regions[0x77e7dc] = struct.pack('<i', r.choice([1, 1, 1, 0, 2]))
    table = bytearray(0x200)
    for c in b'0123456789':
        table[c * 2] = 4
    for c in b' \t\n\v\f\r':
        table[c * 2] = 8
    if r.random() < 0.1:
        table[ord('x') * 2] = 4
    regions[PCTYPE] = table
    regions[0x77ea90] = struct.pack('<I', PCTYPE)
    regions[0x77e7e0] = bytes([r.choice([0x2e, 0x2e, 0x2e, 0x2c, 0x30, 0x2b])])


def twelve(r, name, regions):
    if name == 'add12':
        regions[OUT] = twelve_bytes(r) + r.bytes(4)
        regions[MAN] = r.choice([twelve_bytes(r), struct.pack('<3I', 0xffffffff, 0xffffffff, word(r)), struct.pack('<3I', 1, 0, 0)]) + r.bytes(8)
        if r.random() < 0.4:
            regions[OUT] = struct.pack('<3I', 0xffffffff, r.choice([0xffffffff, 0xfffffffe, word(r)]), word(r)) + r.bytes(4)
        return Call(0x68fa00, [OUT, MAN], regions, conv='c')
    if name in ('shl12', 'shr12'):
        return Call(0x68fa70 if name == 'shl12' else 0x68faa0, [MAN], regions, conv='c')
    if name == 'mtold12':     # (not all zero: that never ends, in both)
        length = r.choice([1, 2, 3, 19, 20, 24, 25, r.randrange(1, 26)])
        digits = bytes([r.randrange(1, 10)]) + bytes(r.choice([r.randrange(10), r.randrange(10), r.getrandbits(8)]) for _ in range(length - 1))
        regions[TEXT2] = digits + r.bytes(4)
        return Call(0x68fad0, [TEXT2, length, OUT], regions, conv='c')
    if name == 'ld12mul':
        regions[OUT], regions[MAN] = twelve_bytes(r) + r.bytes(4), twelve_bytes(r) + r.bytes(8)
        return Call(0x68ff60, [OUT, r.choice([MAN, MAN, MAN, MAN, OUT])], regions, conv='c')
    if name == 'multtenpow':
        regions[OUT] = twelve_bytes(r) + r.bytes(4)
        power = r.choice([0, 1, -1, 7, -7, 8, -8, 9, 63, -64, 511, -512, 4096, 5200, -5200, 308, -308, r.randrange(-5200, 5201)])
        if r.random() < 0.25:     # (the tables are read as they stand, a row with its low word's top bit set unrounded first)
            row = r.choice([0x781f70, 0x7820d0]) + 0x54 + r.randrange(1, 8) * 12
            regions[row] = struct.pack('<HIIH', r.choice([0x8000, 0xffff, 0x7fff]), r.choice([0, 1, word(r)]), word(r) | 0x80000000, r.randrange(0x3000, 0x5000))
        return Call(0x690220, [OUT, power, r.choice([0, 1])], regions, conv='c')
    number_world(r, regions)
    if name == 'strgtold':
        return Call(0x68c440, [OUT, ENDS, TEXT2, r.choice([0, 1]), r.choice([0, 0, 3, -3, 5000, -5000, 5300]), r.choice([0, 0, 2, -2, 5000, 5300]), r.choice([0, 0, 1])], regions, conv='c', ret='a')
    if name == 'fltin':
        regions[FLT] = r.bytes(0x18)
        return Call(0x6816a0, [FLT, TEXT2, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='c', ret='a')
    log = log_region()
    regions[LOG] = log
    regions[PTD] = bytes(0x40)
    set_return(log, GETPTD, PTD)
    regions[0x784c10] = struct.pack('<d', r.choice([float('inf'), 1.5e300, 2.0]))
    return Call(0x6d7710, [TEXT2, r.choice([ENDS, ENDS, 0])], regions, conv='c', ret='d')


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
    if name in TWELVE:
        return twelve(r, name, regions)
    fmt = SINGLE if name == 'tof' else DOUBLE if name == 'tod' else r.choice([DOUBLE, SINGLE, (0x400, -0x3ff, 0x35, 0xb, 0x30, 0x3ff), (0x80, -0x7f, 0x18, 8, 0x40, 0x7f), (0x400, -0x3ff, 0x35, 0xb, 0x20, 0x3ff)])
    regions[TEXT] = number(r, fmt)
    regions[0x781728] = struct.pack('<6i', *DOUBLE) + struct.pack('<6i', *SINGLE)
    if name == 'cvt':
        regions[FORMAT] = struct.pack('<6i', *fmt)
        return Call(0x68be90, [TEXT, OUT, FORMAT], regions, conv='c', ret='a')
    return Call(0x68c060 if name == 'tod' else 0x68c080, [TEXT, OUT], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

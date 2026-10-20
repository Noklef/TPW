# The game's __stdcall wrappers around the C library (src/game/core/libc: 0x5f76c0-0x5f78c0) called directly against the originals, the
# game's MSVC6 CRT run in both: lengths, copies and appends of narrow and wide strings (the output's junk past what is written must stay),
# with counts about the source's length; atoi / _wtoi of crt.py's numbers; the digit tests over every char and word (sign-extended chars
# from 0x80; from 0x100 the CRT asks Windows only when a locale is set, which 0xfb1440 = 0 says it is not); vsprintf / _vsnprintf /
# vswprintf of crt.py's formats with a va_list in memory (wide: strings and chars only, see crt.py).
# usage: libc.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=StrLen,... to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from crt import FORMATS, WFORMATS, number, text, wide  # noqa: E402
from fncmp import IMPORT_TABLE, RAM, Bench, Call  # noqa: E402

W2MB = 0x6fd800  # (a hook's address: no code there)


def _w2mb(e):
    """WideCharToMultiByte(code page, flags, wide, -1, out, size, 0, 0), stdcall: units below 0x100 as bytes, the rest '?', up to the NUL."""
    sp = e.r['ESP']
    src, out, size = e.u32(sp + 8), e.u32(sp + 16), e.u32(sp + 20)
    text = bytearray()
    while True:
        c = struct.unpack('<H', e.read(src + 2 * len(text), 2))[0]
        text.append(c if c < 0x100 else 0x3f)
        if not c:
            break
    e.write(out, bytes(text[:size]))
    e.r['EAX'] = len(text) if len(text) <= size else 0
    e.r['ESP'] += 32


B = Bench([(0x5f76c0, 0x5f78e0), (0x67a400, 0x6902c0)], hooks={W2MB: _w2mb})
CALLS = {'StrLen': 0x5f76c0, 'StrCpy': 0x5f76e0, 'StrNCpy': 0x5f7710, 'StrCat': 0x5f7730, 'StrNCat': 0x5f7770, 'AtoI': 0x5f7790,
         'IsDigit': 0x5f77a0, 'VSPrintf': 0x5f77c0, 'VSNPrintf': 0x5f77e0, 'WcsLen': 0x5f7800, 'WcsCpy': 0x5f7810, 'WcsNCpy': 0x5f7830,
         'WcsCat': 0x5f7850, 'WcsNCat': 0x5f7870, 'WtoI': 0x5f7890, 'IsDigitW': 0x5f78a0, 'VSWPrintf': 0x5f78c0}
A, Z, OUT, FMT, ARGS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x380
# (_wtoi converts with WideCharToMultiByte into a 0x14-byte buffer on its stack, then atol: that import is _w2mb here; tpw_fncmp's stand-in
# reads the wide text itself)


def count(r, n):
    return r.choice([0, 1, 2, 3, 4, 5, 7, 8, max(n - 1, 0), n, n + 1, n + 5, r.randrange(0x40)])


def case(r):
    op = r.deal(OPS)
    junk = lambda: r.choice([b'', r.bytes(8)])  # noqa: E731  (what follows a terminator must not matter)
    narrow = op.startswith('Str')
    if op in ('IsDigit', 'IsDigitW'):
        c = r.choice([r.randrange(0x100), r.choice(b'0123456789/:a \xb2\xb3\xb9'), r.randrange(0x10000), 0xffff, r.getrandbits(32)])
        return Call(CALLS[op], [c], {0xfb1440: bytes(4)}, conv='s', ret='a')
    if op in ('AtoI', 'WtoI'):
        s = number(r)
        if op == 'AtoI':
            return Call(CALLS[op], [A], {A: s + bytes(2) + junk()}, conv='s', ret='a')
        table = bytearray(0x400)
        struct.pack_into('<I', table, 0x1dc, W2MB)
        regions = {A: s.decode('latin-1').encode('utf-16-le') + bytes(2) + junk(), IMPORT_TABLE: table}
        return Call(CALLS[op], [A], regions, conv='s', ret='a')
    if op in ('VSPrintf', 'VSNPrintf', 'VSWPrintf'):
        w = op == 'VSWPrintf'
        fmt, args = b'', []
        for _ in range(r.randrange(5)):
            f = r.choice(WFORMATS if w else FORMATS)
            fmt += r.choice([b'', b'n=', b' ', b'-']) + f
            if b's' in f:
                args.append(r.choice([A, Z]))
            elif f != b'%%':
                args.append(r.choice([0x41, 0x2d, 0xe9, 0x20ac, r.randrange(1, 0xffff)] if w else
                                     [0, 1, 0x41, 0xffffffff, 0x80000000, r.getrandbits(32), r.randrange(-99, 99) & 0xffffffff])
                            | (r.getrandbits(16) << 16 if w and r.random() < 0.3 else 0))
        if w:
            regions = {A: wide(r)[:40] + bytes(2), Z: wide(r)[:40] + bytes(2), FMT: fmt.decode().encode('utf-16-le') + bytes(2)}
        else:
            regions = {A: text(r).replace(b'%', b'')[:40] + b'\0', Z: text(r)[:40] + b'\0', FMT: fmt + b'\0'}
        regions.update({OUT: r.bytes(0x100), ARGS: struct.pack('<%dI' % len(args), *args) + r.bytes(0x10)})
        if op == 'VSNPrintf':
            return Call(CALLS[op], [OUT, r.choice([0, 1, 2, 5, 8, 16, r.randrange(0x60)]), FMT, ARGS], regions, conv='s', ret='a')
        return Call(CALLS[op], [OUT, FMT, ARGS], regions, conv='s', ret='a')
    src = text(r)[:0x60] if narrow else wide(r)[:0x60]
    end = b'\0' if narrow else bytes(2)
    n = len(src) if narrow else len(src) // 2
    regions = {A: src + end + junk()}
    if op in ('StrLen', 'WcsLen'):
        return Call(CALLS[op], [A], regions, conv='s', ret='a')
    if op in ('StrCpy', 'WcsCpy'):
        regions[OUT] = r.bytes(0x100)
        return Call(CALLS[op], [OUT, A], regions, conv='s', ret='a')
    if op in ('StrNCpy', 'WcsNCpy'):
        regions[OUT] = r.bytes(0x100)
        return Call(CALLS[op], [OUT, A, count(r, n)], regions, conv='s', ret='a')
    head = (text(r) if narrow else wide(r))[:0x60]
    regions[OUT] = head + end + r.bytes(0x100 - len(head) - len(end))
    if op in ('StrCat', 'WcsCat'):
        return Call(CALLS[op], [OUT, A], regions, conv='s', ret='a')
    return Call(CALLS[op], [OUT, A, count(r, n)], regions, conv='s', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

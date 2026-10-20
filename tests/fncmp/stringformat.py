# The game strings' Format (src/game/core/string: 0x5f63d0 String_Format, 0x5f6c50 WString_Format, cdecl, varargs) against the originals: formats of
# literal text (bytes past 0x7f among it) and conversions d i u x X o c C s S with the flags - + space 0 #, widths and precisions (numbers or * taken
# from the arguments, negative ones too), the h / l modifiers; strings narrow and wide of up to 40 units, or null ("(null)"); the string formatted
# into one of gamestring.py's. The length estimate (MFC's CString::FormatV) sets the block's capacity, then the C library prints: the game's
# vsprintf / vswprintf (0x67c1e0 / 0x67f4d0) are stand-ins here (_vsprintf: MSVC6's output for these conversions), the host's in the driver;
# _wtoi (0x67f4a0) too (_wtoi: its WideCharToMultiByte is not emulated).
# usage: stringformat.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Narrow,Wide)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gamestring import LOCALE, string  # noqa: E402


def _read_units(e, a, wide, limit=None):
    out, w = [], 2 if wide else 1
    while limit is None or len(out) < limit:
        c = int.from_bytes(e.read(a + w * len(out), w), 'little')
        if not c:
            break
        out.append(c)
    return out


def _s32(v, bits=32):
    v &= (1 << bits) - 1
    return v - (1 << bits) if v >> (bits - 1) else v


def _vsprintf(wide):
    def hook(e):  # cdecl (out, format, args)
        sp = e.r['ESP'] + (4 if e.tail else 0)
        out, fmt, ap = e.u32(sp), e.u32(sp + 4), e.u32(sp + 8)
        f = _read_units(e, fmt, wide)
        res, i = [], 0

        def arg():
            nonlocal ap
            v = e.u32(ap)
            ap += 4
            return v
        while i < len(f):
            c = f[i]
            i += 1
            if c != ord('%'):
                res.append(c)
                continue
            flags = ''
            while chr(f[i]) in '-+ 0#':
                flags += chr(f[i])
                i += 1
            width = None
            if f[i] == ord('*'):
                width = _s32(arg())
                i += 1
                if width < 0:
                    flags, width = flags + '-', -width
            else:
                while ord('0') <= f[i] <= ord('9'):
                    width = (width or 0) * 10 + f[i] - ord('0')
                    i += 1
            prec = None
            if f[i] == ord('.'):
                i += 1
                prec = 0
                if f[i] == ord('*'):
                    prec = _s32(arg())
                    i += 1
                    if prec < 0:
                        prec = None
                else:
                    while ord('0') <= f[i] <= ord('9'):
                        prec = prec * 10 + f[i] - ord('0')
                        i += 1
            mod = ''
            if chr(f[i]) in 'hl':
                mod = chr(f[i])
                i += 1
            conv = chr(f[i])
            i += 1
            if conv == '%':
                body = [ord('%')]
            elif conv in 'cC':
                body = [arg() & (0xffff if (mod == 'l' or (wide and conv == 'c' and mod != 'h') or (not wide and conv == 'C' and mod != 'h')) else 0xff)]
            elif conv in 'sS':
                a = arg()
                w = mod == 'l' or (mod != 'h' and (wide == (conv == 's')))
                body = [ord(x) for x in '(null)'] if not a else _read_units(e, a, w, prec)
                if not a and prec is not None:
                    body = body[:prec]
            else:
                v = arg()
                signed = conv in 'di'
                if mod == 'h':
                    v = _s32(v, 16) if signed else v & 0xffff
                elif signed:
                    v = _s32(v)
                neg, mag = signed and v < 0, abs(v) if signed else v
                digits = {'d': '%d', 'i': '%d', 'u': '%d', 'x': '%x', 'X': '%X', 'o': '%o'}[conv] % mag
                if prec is not None:
                    digits = '' if prec == 0 and mag == 0 else digits.rjust(prec, '0')
                prefix = '-' if neg else ('+' if '+' in flags else ' ' if ' ' in flags else '') if signed else ''
                if '#' in flags and mag and conv in 'xX':
                    prefix += '0' + conv
                if '0' in flags and '-' not in flags and prec is None and width:
                    digits = digits.rjust(width - len(prefix), '0')
                body = [ord(x) for x in prefix + digits]
            zero = '0' in flags and '-' not in flags and conv in 'cCsS'   # (MSVC6 pads characters and strings with zeros too)
            pad = [ord('0' if zero else ' ')] * max(0, (width or 0) - len(body))
            res += body + pad if '-' in flags else pad + body
        e.write(out, struct.pack('<%d%s' % (len(res) + 1, 'H' if wide else 'B'), *(res + [0])))
        e.r['EAX'] = len(res)
    return hook


def _wtoi(e):  # cdecl (s): _wtoi, which converts to the code page first (WideCharToMultiByte, not emulated): the formats' units past 0x7f are no digits
    sp = e.r['ESP'] + (4 if e.tail else 0)
    text = ''.join(chr(c) if c < 0x80 else '?' for c in _read_units(e, e.u32(sp), True)).lstrip(' \t\n\v\f\r')
    sign = -1 if text[:1] == '-' else 1
    text = text[1:] if text[:1] in '+-' else text
    n = len(text) - len(text.lstrip('0123456789'))
    e.r['EAX'] = (sign * int(text[:n] or '0')) & 0xffffffff


B = Bench([(0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)], hooks={0x67c1e0: _vsprintf(False), 0x67f4d0: _vsprintf(True), 0x67f4a0: _wtoi})
S, FMT, DATA, STRS = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000
TEXT = [b'a', b'b', b' ', b'.', b'9', b'0', b'-', b'\xe9', b'\x80', b'\xff']


def case(r):
    op = r.deal(OPS)
    wide = op == 'Wide'
    regions = {HEAP: heap(0x8000), LOCALE: bytes(4)}
    string(r, wide, S, DATA, regions)
    fmt, args, strs = [], [], bytearray()

    def text(w, n):
        nonlocal strs
        at = STRS + len(strs)
        units = [r.choice(b'abcXYZ019 ') for _ in range(n)]
        strs += struct.pack('<%d%s' % (n + 1, 'H' if w else 'B'), *(units + [0]))
        strs += bytes(-len(strs) % 4)
        return at
    for _ in range(r.choice([0, 1, 2, 3, 5])):
        if r.random() < 0.5:
            fmt += [r.choice(TEXT)[0] for _ in range(r.randrange(1, 6))]
        if r.random() < 0.08:
            fmt += list(b'%%')
            continue
        spec = '%' + ''.join(r.sample('-+ 0#', r.choice([0, 0, 1, 2])))
        k = r.random()
        if k < 0.15:
            spec += '*'
            args.append(r.choice([0, 1, 5, 12, 40, -3, -20]) & 0xffffffff)
        elif k < 0.5:
            spec += str(r.choice([1, 2, 5, 9, 16, 33]))
        if r.random() < 0.3:
            if r.random() < 0.3:
                spec += '.*'
                args.append(r.choice([0, 1, 3, 20, -1]) & 0xffffffff)
            else:
                spec += '.' + str(r.choice([0, 1, 2, 4, 10, 31]))
        conv = r.choice('diuxXocCsS' + 'ssdx')
        if r.random() < 0.25:
            spec += r.choice('hl')
        spec += conv
        if '#' in spec and conv == 'o':   # (MSVC6's "#o" with a precision is not the host's)
            spec = spec.replace('#', '')
        fmt += list(spec.encode())
        if conv in 'sS':
            w = 'l' in spec or ('h' not in spec and (wide == (conv == 's')))
            args.append(text(w, r.choice([0, 1, 4, 9, 20, 40])) if r.random() < 0.9 else 0)
        elif conv in 'cC':
            args.append(r.choice([0x41, 0x7a, 0x30, 0x20]) | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0))
        else:
            args.append(r.choice([0, 1, 7, 9, 10, 255, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32), r.getrandbits(16), 0xffff8000]))
    regions[FMT] = struct.pack('<%d%s' % (len(fmt) + 1, 'H' if wide else 'B'), *(fmt + [0]))
    regions[STRS] = bytes(strs) or bytes(4)
    return Call(0x5f6c50 if wide else 0x5f63d0, [S, FMT if r.random() < 0.97 else 0] + args, regions, ret='v')


OPS = [k for k in os.environ.get('OPS', 'Narrow,Wide').split(',') if k]


if __name__ == '__main__':
    B.main(case)

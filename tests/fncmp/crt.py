# tpw_fncmp's CRT stand-ins (tools/fncmp/crt.cpp, which ports reach through CrtEnv) against the
# game's MSVC6 CRT run under the emulator.
# usage: crt.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x67a400, 0x6902c0)])
A, Z, OUT = RAM, RAM + 0x100, RAM + 0x200  # two strings and an output buffer
PICK = b'aAbBzZ@[`{.\\ 09-+\t\x80\xc1\xe1\xff'


def text(r, base=b''):
    """A short string, often sharing a prefix with base, in mixed case."""
    n = r.choice([0, 1, r.randrange(12), r.randrange(40)])
    s = bytearray(r.choice(PICK) if r.random() < 0.5 else r.randrange(1, 256) for _ in range(n))
    if base and r.random() < 0.6:
        k = r.randrange(len(base) + 1)
        flip = lambda c: c ^ 0x20 if chr(c).isalpha() or c in b'@[`{' else c  # [ and { differ only by case's bit
        s = bytearray(flip(c) if r.random() < 0.3 else c for c in base[:k]) + s
    return bytes(s[:0xf0])


def number(r):
    s = r.choice([b'', b' ', b'  \t', b'\n\x0b\x0c\r']) + r.choice([b'', b'-', b'+', b'--', b'+-'])
    s += str(r.choice([0, 7, r.getrandbits(16), r.getrandbits(32), r.getrandbits(40)])).encode()
    return s + r.choice([b'', b'xyz', b'.5', b' 12'])


def decimal(r):
    """A float as balance files and MSVC6's grammar have them: short mantissas, d or e exponents, the odd bare sign."""
    d = lambda k: ''.join(r.choice('0123456789') for _ in range(k))
    s = r.choice(['', ' ', '\t ', '\n\x0b\x0c\r']) + r.choice(['', '', '-', '+', '--', '-+'])
    s += r.choice(['', '0', '00', d(1), d(r.randrange(1, 6)), d(r.randrange(1, 10))])
    if r.random() < 0.6:
        s += r.choice(['.', '.', ',']) + r.choice(['', d(1), d(r.randrange(1, 7)), '0' * r.randrange(1, 9) + d(2)])
    if r.random() < 0.4:
        s += r.choice('eEdD+-xf') + r.choice(['', '+', '-', '-', '+-']) + r.choice(['', '0', d(1), d(2), '00' + d(1)])
    if r.random() < 0.15:
        s = r.choice(['', ' ', '-']) + r.choice(['inf', 'nan', '0x1a', '.', '-.', 'e5', '1e400', '1e-400', '12345678901234567890123456789'])
    return (s + r.choice(['', '', 'xyz', ' 12', '.5', '#'])).encode()


def wide(r):
    """A short wide string (no NUL): ASCII, the odd '-', '%' or character above 0xff."""
    n = r.choice([0, 1, r.randrange(8), r.randrange(30)])
    pick = lambda: r.choice([r.randrange(0x20, 0x7f), 0x2d, 0x25, 0xe9, r.randrange(0x100, 0x10000)]) if r.random() < 0.9 else r.randrange(1, 0x20)
    return b''.join(pick().to_bytes(2, 'little') for _ in range(n))


# (strings and characters: MSVC6's wide integers go through code the emulator does not run, and a %c of 0xffff, WEOF, fails there)
WFORMATS = [b'%s', b'%s', b'%s', b'%c', b'%.3s', b'%10s|', b'%-6s|', b'%%', b'%.0s']
FORMATS = [b'%d',b'%i', b'%u', b'%x', b'%X', b'%08x', b'%-5d|', b'%5d', b'%+d', b'%c', b'%s', b'%.3s',
           b'%10s|', b'%-10s|', b'%%', b'%ld', b'%hd', b'%hx', b'%o', b'%#x', b'% d', b'%05d', b'%lu']


def case(r):
    op = r.choice(OPS)
    a = text(r)
    z = text(r, a) if r.random() < 0.8 else a
    junk = lambda: r.choice([b'', r.bytes(8)])  # what follows the terminator must not matter
    regions = {A: a + b'\0' + junk(), Z: z + b'\0' + junk()}
    if op == 'stricmp':
        return Call(0x67df40, [A, Z], regions, ret='a')
    if op in ('strrchr', 'strchr'):
        c = r.choice([r.choice(a) if a else 0, 0, r.randrange(256), r.getrandbits(32), ord('.')])
        return Call(0x67b5f0 if op == 'strrchr' else 0x67bfc0, [A, c], regions, ret='a')
    if op == 'strncmp':
        return Call(0x67c250, [A, Z, r.choice([0, 1, 3, r.randrange(50), 0xffffffff])], regions, ret='a')
    if op == 'strstr':
        z = a[r.randrange(len(a) + 1):][:r.randrange(6)] if r.random() < 0.6 else text(r)[:4]
        return Call(0x67c080, [A, Z], {A: a + b'\0', Z: z + b'\0'}, ret='a')
    if op in ('toupper', 'tolower'):
        c = r.choice([r.randrange(256), r.randrange(-300, 300) & 0xffffffff, r.getrandbits(32), r.choice(b'@AZ[`az{')])
        return Call(0x67f1a0 if op == 'toupper' else 0x67fa60, [c], {}, ret='a')
    if op == 'atoi':
        return Call(0x67a520, [A], {A: number(r) + b'\0'}, ret='a')
    if op == 'strncpy':   # (the output's junk after the count must stay)
        n = r.choice([0, 1, 2, 3, 4, 5, 7, 8, len(a), len(a) + 1, len(a) + 5, r.randrange(0x50)])
        return Call(0x67ae00, [OUT, A, n], {A: a + b'\0' + junk(), OUT: r.bytes(0x100)}, ret='a')
    if op == 'iswspace':   # (below 0x80, with junk in the high half: MSVC6 asks a Latin-1 table above that, then Windows)
        c = r.choice([r.randrange(0x80), r.choice(b' \t\n\x0b\x0c\r\x08\x0e\x1f!0aZ')])
        return Call(0x67a730, [c | (r.getrandbits(16) << 16 if r.random() < 0.4 else 0)], {}, ret='a')
    if op in CLASSES:   # (the callers' sign-extended chars: from 0x80 the words before the table)
        c = r.choice([r.randrange(-128, 256), r.randrange(-1, 0x80), r.choice(b' \t\n\x0b\x0c\r09azAZ_.-@[`{')])
        return Call(CLASSES[op], [c & 0xffffffff], {}, ret='a')
    if op == 'atof':
        return Call(0x67a6c0, [A], {A: decimal(r) + b'\0' + junk()}, ret='d')
    if op == 'wcslen':
        n = r.randrange(60)
        w = b''.join(r.randrange(1, 0x10000).to_bytes(2, 'little') for _ in range(n))
        return Call(0x67acd0, [A], {A: w + b'\0\0'}, ret='a')
    if op == 'wcsncpy':   # (the output's junk after the count must stay)
        w = wide(r)
        n = r.choice([0, 1, 2, 3, len(w) // 2, len(w) // 2 + 1, len(w) // 2 + 4, r.randrange(0x40)])
        return Call(0x67b0d0, [OUT, A, n], {A: w + b'\0\0' + junk(), OUT: r.bytes(0x100)}, ret='a')
    if op == 'wcsicmp':   # a wide string and a copy with case flips, a changed unit or a cut
        units = [r.choice([r.randrange(0x20, 0x7f), r.choice(b'@AZ[`az{'), 0xc9, 0xe9, 0x141, 0x161, 0xff21, 0xff41, 0xffff, r.randrange(1, 0x10000)])
                 for _ in range(r.choice([0, 1, r.randrange(8), r.randrange(30)]))]
        other = [c ^ 0x20 if r.random() < 0.4 and (c & ~0x20) in range(0x40, 0x5c) or c > 0xff and r.random() < 0.2 else c for c in units]
        if other and r.random() < 0.3:
            other[r.randrange(len(other))] = r.choice([0x40, 0x5b, 0x60, 0x7b, 0x41, 0x61, 0xffff, r.randrange(1, 0x10000)])
        if r.random() < 0.2:
            other = other[:r.randrange(len(other) + 1)]
        w = lambda us: b''.join(c.to_bytes(2, 'little') for c in us) + b'\0\0' + junk()
        a, z = (w(units), w(other)) if r.random() < 0.5 else (w(other), w(units))
        return Call(0x67b140, [A, Z], {A: a, Z: z}, ret='a')
    if op == 'swprintf':   # wide: %s takes a wide string, %c a wide char
        fmt, args = b'', []
        for _ in range(r.randrange(5)):
            f = r.choice(WFORMATS)
            fmt += r.choice([b'', b'n=', b' ', b'-']) + f
            if b's' in f:
                args.append(r.choice([A, Z]))
            elif f != b'%%':
                args.append(r.choice([0x41, 0x2d, 0xe9, 0x20ac, r.randrange(1, 0xffff)]) | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0))
        regions = {A: wide(r)[:40] + b'\0\0', Z: wide(r)[:40] + b'\0\0', OUT: bytes(0x100), RAM + 0x300: fmt.decode().encode('utf-16-le') + b'\0\0'}
        return Call(0x67af00, [OUT, RAM + 0x300] + args, regions, ret='a')
    # sprintf: up to three conversions with integer or string arguments
    fmt, args = b'', []
    for _ in range(r.randrange(4)):
        f = r.choice(FORMATS)
        fmt += r.choice([b'', b'n=', b' ', b'\\']) + f
        if b's' in f:
            args.append(r.choice([A, Z]))
        elif f != b'%%':
            args.append(r.choice([0, 1, 0x41, 0xffffffff, 0x80000000, r.getrandbits(32), r.randrange(-99, 99) & 0xffffffff]))
    a = a.replace(b'%', b'')[:40]
    regions = {A: a + b'\0', Z: text(r)[:40] + b'\0', OUT: bytes(0x100), RAM + 0x300: fmt + b'\0'}
    return Call(0x67a650, [OUT, RAM + 0x300] + args, regions, ret='a')


CLASSES = {'isalpha': 0x67a530, 'isdigit': 0x67a560, 'isspace': 0x67a590, 'isalnum': 0x67a5c0}
KINDS = ['stricmp', 'strrchr', 'strchr', 'strncmp', 'strstr', 'toupper', 'tolower', 'atoi', 'wcslen', 'sprintf', 'strncpy', 'iswspace',
         'isalpha', 'isdigit', 'isspace', 'isalnum', 'atof', 'swprintf', 'wcsncpy', 'wcsicmp']
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

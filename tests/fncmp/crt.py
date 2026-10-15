# tpw_fncmp's CRT stand-ins (tools/fncmp/crt.cpp, which ports reach through CrtEnv) against the
# game's MSVC6 CRT run under the emulator.
# usage: crt.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x67a400, 0x690000)])
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


FORMATS = [b'%d', b'%i', b'%u', b'%x', b'%X', b'%08x', b'%-5d|', b'%5d', b'%+d', b'%c', b'%s', b'%.3s',
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
    if op == 'wcslen':
        n = r.randrange(60)
        w = b''.join(r.randrange(1, 0x10000).to_bytes(2, 'little') for _ in range(n))
        return Call(0x67acd0, [A], {A: w + b'\0\0'}, ret='a')
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


KINDS = ['stricmp', 'strrchr', 'strchr', 'strncmp', 'strstr', 'toupper', 'tolower', 'atoi', 'wcslen', 'sprintf']
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

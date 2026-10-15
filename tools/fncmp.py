"""Differential tests of reimplementations: the hooked C++ (build/tpw_fncmp.exe) against the
original code run by x87emu, on random cases, comparing every byte the cases give.

A test script defines a case generator and calls run():

    import sys; sys.path.insert(0, r'C:/git/Noklef/TPW/tools')
    from fncmp import *
    B = Bench([(0x43e1b0, 0x43e480)])           # listing ranges the original needs
    def case(r):                               # r: Rand (seeded; r.wild for extreme values)
        node = r.bytes(0x60)
        r.floats(node, range(0x34, 0x58, 4))
        return Call(0x43e1b0, [A, B_, r.f(), OUT], {A: node, ...})
    B.main(case)                                # cmd line: CASES SEED [--wild] [--show N]

Call(address, args, regions, conv='c'|'s'|'t'|'f', ret='v'|'i'|'d'): ret 'i' compares EAX
(and EDX), 'd' the double left in st0. Region addresses must be in the game's data
(0x6e7000-0xfc5200) or at 0x10000000 and up (RAM, ALLOC.. are free slots); anything a case
does not set reads as the exe image there, else zero, in both.

Mutation testing: B.mutants(case, [(path, old, new), ...]) applies each edit to the source
(old must occur once), rebuilds, runs, restores, and reports which edits were caught.

The emulator gets __ftol, modf and _CIfmod stubs; Bench(..., sines=N) also gives both sides
an N-entry sine table at 0x7b095c (see sintable.h).
"""
import math
import os
import random
import re
import struct
import subprocess
import sys
import tempfile

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import x87emu  # noqa: E402

EXE = os.path.join(ROOT, '.data', 'nocd', 'tp.exe')
DRIVER = os.path.join(ROOT, 'build', 'tpw_fncmp.exe')
LISTING = os.path.join(ROOT, '_re', 'text.asm')
RAM = 0x30000000  # free addresses for case regions: RAM, RAM + 0x1000, ...


def fb(x):
    """float -> its bits"""
    return struct.unpack('<I', struct.pack('<f', x))[0]


def bf(b):
    """bits -> float"""
    return struct.unpack('<f', struct.pack('<I', b & 0xffffffff))[0]


def f32(x):
    return bf(fb(x))


SPECIAL = [0, 0x80000000, fb(1.0), fb(-1.0), fb(0.5), 0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000,
           fb(3e38), fb(-3e38), fb(1e-40), fb(-1e-40), fb(1e-30), 0x7f800001, 0xff812345]


class Rand(random.Random):
    """random with float-bits helpers; wild adds extreme exponents and arbitrary bits."""

    def __init__(self, seed, wild=False):
        super().__init__(seed)
        self.wild = wild

    def f(self, scale=None, special=0.06):
        """the bits of a float in -scale..scale (scale picked if None), sometimes special."""
        if self.wild and self.random() < 0.3:
            if self.random() < 0.5:
                return fb(self.uniform(-1, 1) * 10.0 ** self.randrange(-45, 39))
            return self.getrandbits(32)
        if self.random() < special:
            return self.choice(SPECIAL)
        return fb(self.uniform(-1, 1) * (scale or self.choice([1, 10, 300, 5000])))

    def pos(self, scale=None):
        return fb(self.uniform(0, scale or self.choice([1, 5, 50])))

    def bytes(self, n):
        return bytearray(self.getrandbits(8) for _ in range(n))

    def floats(self, buf, offsets, scale=None):
        for off in offsets:
            struct.pack_into('<I', buf, off, self.f(scale))
        return buf


def put(buf, off, fmt, *values):
    struct.pack_into('<' + fmt, buf, off, *values)
    return buf


class Call:
    def __init__(self, address, args, regions, conv='c', ret='v'):
        self.address, self.args, self.regions, self.conv, self.ret = address, list(args), dict(regions), conv, ret


def sine_table(n):
    """sintable.h's layout at 0x7b095c: scale, mask, (pad), values."""
    return bytearray(struct.pack('<fIi', f32(n / (2 * math.pi)), n - 1, 0) +
                     struct.pack('<%df' % n, *[math.sin(2 * math.pi * i / n) for i in range(n)]))


def slice_listing(ranges):
    import bisect
    lines = open(LISTING).read().splitlines()
    addrs = [int(l[:8], 16) for l in lines]
    out = []
    for lo, hi in ranges:
        out += lines[bisect.bisect_left(addrs, lo):bisect.bisect_left(addrs, hi)]
    path = os.path.join(tempfile.gettempdir(), 'fncmp_%d.asm' % os.getpid())
    open(path, 'w').write('\n'.join(out) + '\n')
    return path


def _ftol(e):
    x = e.st.pop(0)
    v = int(x) if math.isfinite(x) and -2 ** 63 <= int(x) < 2 ** 63 else -2 ** 63
    v &= (1 << 64) - 1
    e.r['EAX'], e.r['EDX'] = v & 0xffffffff, v >> 32


def _modf(e):  # modf(double x, double *whole): args at [ESP] (hooks see no return address)
    sp = e.r['ESP']
    x = struct.unpack('<d', e.read(sp, 8))[0]
    frac, whole = math.modf(x)
    e.write(e.u32(sp + 8), struct.pack('<d', whole))
    e.st.insert(0, frac)


def _cifmod(e):  # _CIfmod: st1 fmod st0, popping both
    y, x = e.st.pop(0), e.st.pop(0)
    e.st.insert(0, math.fmod(x, y) if math.isfinite(x) and y != 0 and not math.isnan(y) else x87emu.INDEFINITE)


class Bench:
    def __init__(self, ranges, sines=None, hooks=None):
        self.emu = x87emu.Emu([slice_listing(ranges)])
        self.emu.hooks.update({0x67a830: _ftol, 0x67b280: _modf, 0x67b24a: _cifmod})
        self.emu.hooks.update(hooks or {})
        self.extra = {0x7b095c: sine_table(sines)} if sines else {}

    def original(self, c):
        e = self.emu
        e.mem.clear()
        for k, v in c.regions.items():
            e.write(k, bytes(v))
        e.reset()
        args = c.args
        if c.conv in 'tf' and args:
            e.r['ECX'] = args[0]
            args = args[1:]
            if c.conv == 'f' and args:
                e.r['EDX'] = args[0]
                args = args[1:]
        for a in reversed(args):
            e.push(a & 0xffffffff)
        e.push(x87emu.SENTINEL)
        pc = c.address
        for _ in range(5_000_000):
            if pc == x87emu.SENTINEL:
                break
            pc = e.step(pc)
        else:
            return 'step limit'
        st0 = struct.unpack('<Q', struct.pack('<d', e.st[0]))[0] if c.ret == 'd' and e.st else 0
        head = ['%08x' % e.r['EAX'] if c.ret == 'i' else '-', '%08x' % e.r['EDX'] if c.ret == 'i' else '-',
                '%016x' % st0 if c.ret == 'd' else '-']
        return head + [e.read(k, len(v)).hex() for k, v in c.regions.items()]

    def cases(self, gen, n, seed, wild):
        r = Rand(seed, wild)
        out = []
        for _ in range(n):
            c = gen(r)
            for k, v in self.extra.items():
                c.regions.setdefault(k, v)
            out.append(c)
        return out

    def native(self, cases):
        inp = ''.join('%x %s %s %x %s %x %s\n' % (
            c.address, c.conv, c.ret, len(c.args), ' '.join('%x' % (a & 0xffffffff) for a in c.args), len(c.regions),
            ' '.join('%x %s' % (k, bytes(v).hex()) for k, v in c.regions.items())) for c in cases)
        p = subprocess.run([DRIVER, EXE], input=inp, capture_output=True, text=True)
        out = []
        for c, line in zip(cases, p.stdout.splitlines()):
            w = line.split()
            if len(w) < 3 or w[0] in ('crash', 'nohook', 'fncmp:'):
                out.append(line)
                continue
            out.append([w[0] if c.ret == 'i' else '-', w[1] if c.ret == 'i' else '-', w[2] if c.ret == 'd' else '-'] + w[3:])
        if len(out) < len(cases):
            out += ['driver stopped (exit %x): %s' % (p.returncode & 0xffffffff, p.stderr.strip()[:200])] * (len(cases) - len(out))
        return out

    def run(self, gen, n, seed, wild=False, show=3, quiet=False):
        """Returns the number of differing cases."""
        cases = self.cases(gen, n, seed, wild)
        nat = self.native(cases)
        bad = 0
        for i, (c, cpp) in enumerate(zip(cases, nat)):
            emu = self.original(c)
            if emu == cpp:
                continue
            bad += 1
            if bad > show:
                continue
            print('DIFF case %d: %x(%s)' % (i, c.address, ', '.join('%x' % (a & 0xffffffff) for a in c.args)))
            if isinstance(cpp, str) or isinstance(emu, str):
                print('  cpp %s\n  emu %s' % (cpp if isinstance(cpp, str) else 'ok', emu if isinstance(emu, str) else 'ok'))
                continue
            names = ['EAX', 'EDX', 'ST0'] + ['%x' % k for k in c.regions]
            for name, x, y in zip(names, cpp, emu):
                if x != y:
                    d = next((j for j in range(0, min(len(x), len(y)), 8) if x[j:j + 8] != y[j:j + 8]), 0)
                    print('  %s +%x: cpp %s emu %s' % (name, d // 2, x[d:d + 16], y[d:d + 16]))
        if not quiet:
            print('%d cases%s, seed %d: %d differ' % (n, ' (wild)' if wild else '', seed, bad))
        return bad

    def main(self, gen):
        """CASES SEED [--wild] [--show N] [--both]: run (--both: normal then wild)."""
        a = sys.argv[1:]
        n, seed = int(a[0]) if a else 2000, int(a[1]) if len(a) > 1 else 1
        show = int(a[a.index('--show') + 1]) if '--show' in a else 3
        bad = 0
        if '--wild' not in a or '--both' in a:
            bad += self.run(gen, n, seed, False, show)
        if '--wild' in a or '--both' in a:
            bad += self.run(gen, n, seed + 1, True, show)
        sys.exit(1 if bad else 0)

    def mutants(self, gen, edits, n=1500, seed=7, wild=False):
        """Applies each (path, old, new) edit alone, rebuilds and runs; True where caught."""
        results = []
        for path, old, new in edits:
            full = path if os.path.isabs(path) else os.path.join(ROOT, path)
            src = open(full, newline='').read()
            if src.count(old) != 1:
                raise ValueError('%s: %r occurs %d times' % (path, old, src.count(old)))
            open(full, 'w', newline='').write(src.replace(old, new))
            try:
                ok = build()
                bad = self.run(gen, n, seed, wild, show=0, quiet=True) if ok else None
            finally:
                open(full, 'w', newline='').write(src)
            print('%-9s %s: %r -> %r' % ('BUILD ERR' if bad is None else 'caught' if bad else 'SURVIVED',
                                        os.path.basename(path), old[:50], new[:50]))
            results.append(bool(bad))
        build()
        return results


def build():
    """Runs build.bat; True if it succeeded."""
    p = subprocess.run(['cmd', '/c', os.path.join(ROOT, 'build.bat')], capture_output=True, text=True,
                       stdin=subprocess.DEVNULL)
    if p.returncode:
        print('\n'.join(l for l in p.stdout.splitlines() if 'error' in l.lower())[:2000])
    return p.returncode == 0

"""Writes C++ and a bench for the tiny unported functions whose whole body is one of a few known shapes.

  python -I tools/tinygen.py LO HI TAG [--list] [--max N] [--skip ADDR,ADDR...]

Scans _re/text.asm for functions in LO-HI (hex) that are not hooked yet and whose instructions are
exactly one of the shapes below (a vtable pointer store, a getter or setter of a global or of a
field of ECX, a constant, a stub that only pops its arguments). With --list it only counts them.
Otherwise it writes src/game/core/tiny_TAG.{h,cpp}, tests/fncmp/tiny_TAG.py and tiny_TAG.spec in the
current directory's scratch (printed), for tools/port.py to integrate, bench and commit.

Names are `Shape_<what>_<address>` so that they are unique; they say what the function does, not what
it is for. A setter returns the value it was given (the code leaves it in EAX); a byte getter returns
only AL. Whatever tools/retcheck.py then finds suspect (a caller reading a register the C++ doesn't
keep) is listed and left out with --skip.
"""
import collections
import glob
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

H = r'(-?0x[0-9a-f]+)'
FIELD = r'\[ECX(?: \+ %s)?\]' % H


def num(s):
    return int(s, 16) if s else 0


def norm(ins):
    return ins.replace('dword ptr ', '').replace('word ptr ', 'w').replace('byte ptr ', 'b').strip()


# shape: (regex list for the instructions, a function building the function description from the groups)
# A description: dict(name, conv, nargs, ret, doc, decl, body, globals)
SHAPES = []


def shape(*patterns):
    def deco(fn):
        SHAPES.append(([re.compile('^%s$' % p) for p in patterns], fn))
        return fn
    return deco


def offs(v):
    """C++ for a field offset (negative ones as unsigned wrap)."""
    return '(u32)%d' % v if v < 0 else '0x%x' % v


def hexs(v):
    return ('-0x%x' % -v) if v < 0 else ('0x%x' % v)


def nm(v):
    """A field offset for a name: 4c, or m38 for -0x38."""
    return ('m%x' % -v) if v < 0 else ('%x' % v)


@shape(r'MOV \[ECX\],(0x[0-9a-f]+)', r'RET')
def _vtbl(a, m):
    c = num(m[0].group(1))
    return dict(name='Vtbl%x_Set_%06x' % (c, a), conv='t', nargs=0, ret='v', doc='the vtable pointer +0 = 0x%x' % c,
                body='at<u32>(o, 0) = 0x%08x;' % c)


@shape(r'MOV EAX,\[(0x[0-9a-f]+)\]', r'RET')
def _gget(a, m):
    g = num(m[0].group(1))
    return dict(name='Global%x_Get_%06x' % (g, a), conv='c', nargs=0, ret='a', doc='the dword at 0x%x' % g,
                body='return global<u32>(0x%08x);' % g, globals=[g])


@shape(r'MOV EAX,\[ESP \+ 0x4\]', r'MOV \[(0x[0-9a-f]+)\],EAX', r'RET')
def _gset(a, m):
    g = num(m[1].group(1))
    return dict(name='Global%x_Set_%06x' % (g, a), conv='c', nargs=1, ret='a', doc='0x%x = value; returns it' % g,
                body='global<u32>(0x%08x) = v0;\n    return v0;' % g, globals=[g])


@shape(r'MOV EAX,%s' % FIELD, r'RET')
def _fget(a, m):
    k = num(m[0].group(1))
    return dict(name='Get%s_%06x' % (nm(k), a), conv='t', nargs=0, ret='a', doc='the dword at %s' % hexs(k),
                body='return at<u32>(o, %s);' % offs(k))


@shape(r'MOV AL,b%s' % FIELD, r'RET')
def _bget(a, m):
    k = num(m[0].group(1))
    return dict(name='GetByte%s_%06x' % (nm(k), a), conv='t', nargs=0, ret='b', doc='the byte at %s' % hexs(k),
                body='return at<u8>(o, %s);' % offs(k))


@shape(r'MOV AX,w%s' % FIELD, r'RET')
def _wget(a, m):
    k = num(m[0].group(1))
    return dict(name='GetWord%s_%06x' % (nm(k), a), conv='t', nargs=0, ret='w', doc='the word at %s' % hexs(k),
                body='return at<u16>(o, %s);' % offs(k))


@shape(r'MOV EAX,\[ESP \+ 0x4\]', r'MOV %s,EAX' % FIELD, r'RET 0x4')
def _fset(a, m):
    k = num(m[1].group(1))
    return dict(name='Set%s_%06x' % (nm(k), a), conv='t', nargs=1, ret='a', doc='%s = value; returns it' % hexs(k),
                body='at<u32>(o, %s) = v0;\n    return v0;' % offs(k))


@shape(r'MOV %s,(0x[0-9a-f]+)' % FIELD, r'RET')
def _fimm(a, m):
    k, c = num(m[0].group(1)), num(m[0].group(2))
    return dict(name='Set%s_To%x_%06x' % (nm(k), c, a), conv='t', nargs=0, ret='v', doc='%s = 0x%x' % (hexs(k), c),
                body='at<u32>(o, %s) = 0x%x;' % (offs(k), c))


@shape(r'LEA EAX,%s' % FIELD, r'RET')
def _lea(a, m):
    k = num(m[0].group(1))
    return dict(name='Part%s_%06x' % (nm(k), a), conv='t', nargs=0, ret='p', doc='the address of the part at %s' % hexs(k),
                body='return o + %s;' % (('(s32)%d' % k) if k < 0 else '0x%x' % k))


@shape(r'MOV EAX,(0x[0-9a-f]+)', r'RET(?: (0x[0-9a-f]+))?')
def _const(a, m):
    c = num(m[0].group(1))
    n = num(m[1].group(1)) // 4 if m[1].group(1) else 0
    return dict(name='Const%x_%06x' % (c, a), conv='s' if n else 'c', nargs=n, ret='a', doc='returns 0x%x' % c, body='return 0x%x;' % c)


@shape(r'XOR EAX,EAX', r'RET(?: (0x[0-9a-f]+))?')
def _zero(a, m):
    n = num(m[1].group(1)) // 4 if m[1].group(1) else 0
    return dict(name='Zero_%06x' % a, conv='s' if n else 'c', nargs=n, ret='a', doc='returns 0', body='return 0;')


@shape(r'OR EAX,-0x1', r'RET(?: (0x[0-9a-f]+))?')
def _minus1(a, m):
    n = num(m[1].group(1)) // 4 if m[1].group(1) else 0
    return dict(name='MinusOne_%06x' % a, conv='s' if n else 'c', nargs=n, ret='a', doc='returns -1', body='return 0xffffffffu;')


@shape(r'RET (0x[0-9a-f]+)')
def _stub(a, m):
    n = num(m[0].group(1)) // 4
    return dict(name='Stub_Ret%d_%06x' % (n * 4, a), conv='s', nargs=n, ret='v', doc='does nothing (pops %d bytes)' % (n * 4), body='')


@shape(r'RET')
def _stub0(a, m):
    return dict(name='Stub_%06x' % a, conv='c', nargs=0, ret='v', doc='does nothing', body='')


def known_names():
    """Names symbols.csv already gives functions (kept: the generated name would be a step back)."""
    names = {}
    for line in open(os.path.join(ROOT, 're/symbols.csv'), encoding='utf-8', errors='replace'):
        p = line.split(',')
        if p[0].startswith('0x') and len(p) > 2 and p[2] == 'function' and re.match(r'^[A-Za-z_][A-Za-z0-9_]*$', p[1]):
            names[int(p[0], 16)] = p[1]
    return names


def functions(lo, hi):
    hooked = set(int(x, 16) for x in re.findall(r'0x00([0-9a-f]{6})', open(os.path.join(ROOT, 'src/hook/hooks.cpp')).read()))
    starts = set()
    for f in glob.glob(os.path.join(ROOT, '_re/decomp-typed/decomp_*.c')):
        for m in re.finditer(r'^// ==== (\S+) @ ([0-9a-f]{8}) ====', open(f, errors='replace').read(), re.M):
            starts.add(int(m.group(2), 16))
    ks = sorted(starts)
    end = {a: ks[i + 1] for i, a in enumerate(ks[:-1])}
    body = collections.defaultdict(list)
    cur, j = None, 0
    for line in open(os.path.join(ROOT, '_re/text.asm'), errors='replace'):
        try:
            va = int(line[:8], 16)
        except ValueError:
            continue
        while j < len(ks) and va >= ks[j]:
            cur, j = ks[j], j + 1
        if cur is None or not lo <= cur < hi or cur in hooked or va >= end.get(cur, cur):
            continue
        body[cur].append((va, norm(line[8:])))
    return body, end


def frame(a, texts):
    """A function in the debug build's frame style: PUSH EBP; MOV EBP,ESP; [save ECX]; [a constant result]; POP EBP; RET [n]."""
    if len(texts) < 4 or texts[:2] != ['PUSH EBP', 'MOV EBP,ESP'] or texts[-2] != 'POP EBP' or not texts[-1].startswith('RET'):
        return None
    mid = texts[2:-2]
    if mid[:2] == ['PUSH ECX', 'MOV [EBP + -0x4],ECX']:
        if mid[-1:] != ['MOV ESP,EBP']:
            return None
        mid = mid[2:-1]
    m = re.match(r'^RET(?: (0x[0-9a-f]+))?$', texts[-1])
    n = num(m.group(1)) // 4 if m.group(1) else 0
    conv = 's' if n else 'c'
    if not mid:
        return dict(name='FrameStub_%06x' % a, conv=conv, nargs=n, ret='v', doc='does nothing (frame style)', body='')
    if len(mid) != 1:
        return None
    r = mid[0]
    mm = re.match(r'^MOV EAX,(0x[0-9a-f]+)$', r)
    if mm:
        c = num(mm.group(1))
        return dict(name='FrameConst%x_%06x' % (c, a), conv=conv, nargs=n, ret='a', doc='returns 0x%x' % c, body='return 0x%x;' % c)
    if r == 'XOR EAX,EAX':
        return dict(name='FrameZero_%06x' % a, conv=conv, nargs=n, ret='a', doc='returns 0', body='return 0;')
    mm = re.match(r'^(?:MOV|OR) AX,(-?0x[0-9a-f]+)$', r)
    if mm:
        c = num(mm.group(1)) & 0xffff
        return dict(name='FrameWord%x_%06x' % (c, a), conv=conv, nargs=n, ret='w', doc='returns the word 0x%x' % c, body='return 0x%x;' % c)
    if r == 'XOR AX,AX':
        return dict(name='FrameWordZero_%06x' % a, conv=conv, nargs=n, ret='w', doc='returns the word 0', body='return 0;')
    return None


def match(a, ins):
    texts = [t for _, t in ins]
    d = frame(a, texts)
    if d:
        d.setdefault('globals', [])
        d['addr'] = a
        return d
    for pats, fn in SHAPES:
        if len(pats) != len(texts):
            continue
        ms = [p.match(t) for p, t in zip(pats, texts)]
        if all(ms):
            d = fn(a, ms)
            d.setdefault('globals', [])
            d['addr'] = a
            return d
    return None


RET = {'v': 'void', 'a': 'u32', 'b': 'u8', 'w': 'u16', 'p': 'u8 *'}
CONV = {'t': '__fastcall', 'c': '__cdecl', 's': '__stdcall'}


def decl(d):
    args = []
    if d['conv'] == 't':
        args.append('u8 *o' if d['ret'] == 'p' or True else '')
        if d['nargs']:
            args.append('void *')
    args += ['u32 v%d' % i for i in range(d['nargs'])]
    if d['conv'] == 't' and not d['nargs'] and d['ret'] in 'abw' and d['body'].startswith('return at<'):
        args[0] = 'const u8 *o'
    return '%s %s %s(%s)' % (RET[d['ret']], CONV[d['conv']], d['name'], ', '.join(args))


def main():
    a = sys.argv[1:]
    if len(a) < 2:
        sys.exit(__doc__)
    lo, hi = int(a[0], 16), int(a[1], 16)
    body, end = functions(lo, hi)
    found = []
    for addr in sorted(body):
        d = match(addr, body[addr])
        if d:
            d['end'] = end[addr]
            found.append(d)
    skip = set()
    if '--skip' in a:
        skip = set(int(x, 16) for x in a[a.index('--skip') + 1].split(','))
    found = [d for d in found if d['addr'] not in skip]
    names = known_names()
    for d in found:
        if d['addr'] in names:
            d['name'] = names[d['addr']]
    if '--max' in a:
        found = found[:int(a[a.index('--max') + 1])]
    kinds = collections.Counter(d['name'].split('_')[0].rstrip('0123456789abcdefm') or d['name'] for d in found)
    print('%d functions match: %s' % (len(found), dict(kinds)))
    if '--list' in a or len(a) < 3 or a[2].startswith('--'):
        for d in found[:60]:
            print('  %06x  %s' % (d['addr'], decl(d)))
        return
    tag = a[2]
    out = os.path.join(ROOT, 'src/game/core/tiny_%s' % tag)
    with open(out + '.h', 'w', newline='\n') as f:
        f.write('// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the\n'
                '// instruction listing (0x%06x-0x%06x). Named by what they do and where they are; what they are for\n'
                '// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.\n'
                '#pragma once\n\n#include "../../../re/types.h"\n\nextern "C" {\n\n' % (lo, hi))
        for d in found:
            f.write('// 0x%06x: %s.\n%s;\n' % (d['addr'], d['doc'], decl(d)))
        f.write('}\n')
    with open(out + '.cpp', 'w', newline='\n') as f:
        f.write('#include "tiny_%s.h"\n\n#include <cstdint>\n\nnamespace {\n\n'
                'template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }\n'
                'template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }\n\n'
                '}  // namespace\n\nextern "C" {\n\n' % tag)
        for d in found:
            sig = decl(d)
            # unused parameter names are fine for stubs; keep the names where the body uses them
            if not d['body']:
                sig = re.sub(r' (o|v\d+)\b', '', sig)
            f.write('%s {' % sig)
            if d['body']:
                f.write('\n    ' + d['body'].replace('return at<u8>', 'return at<u8>') + '\n')
            f.write('}\n\n')
        f.write('}\n')
    bench = os.path.join(ROOT, 'tests/fncmp/tiny_%s.py' % tag)
    with open(bench, 'w', newline='\n') as f:
        f.write('# The tiny functions of src/game/core/tiny_%s (written by tools/tinygen.py) against the originals: any\n'
                '# object bytes, any global values, any arguments.\n'
                '# usage: tiny_%s.py [CASES [SEED]] [--wild|--both] [--show N]\n' % (tag, tag))
        f.write('import os\nimport sys\n\nsys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), \'..\', \'..\', \'tools\'))\n'
                'from fncmp import Bench, Call, RAM  # noqa: E402\n\n')
        f.write('# (address, end, convention, stack arguments, returns, globals touched)\nFUNCS = [\n')
        for d in found:
            f.write('    (0x%06x, 0x%06x, %r, %d, %r, %r),\n' % (d['addr'], d['end'], d['conv'], d['nargs'], 'a' if d['ret'] in 'ap' else d['ret'], d['globals']))
        f.write(']\nB = Bench([(a, e) for a, e, *_ in FUNCS])\n\n\n'
                'def case(r):\n'
                '    address, _end, conv, n, ret, globs = r.choice(FUNCS)\n'
                '    regions = {RAM: r.bytes(0x400)}\n'
                '    for g in globs:\n'
                '        regions[g & ~0xf] = r.bytes(0x20)\n'
                '    args = [r.choice([0, 1, 0xff, 0x7fffffff, 0xffffffff, r.getrandbits(32)]) for _ in range(n)]\n'
                '    if conv == \'t\':\n'
                '        args = [RAM + 0x80] + args\n'
                '    return Call(address, args, regions, conv=conv, ret=ret)\n\n\n'
                'if __name__ == \'__main__\':\n    B.main(case)\n')
    spec = os.path.join(os.environ.get('TINYGEN_OUT', ROOT), 'tiny_%s.spec' % tag)
    with open(spec, 'w', newline='\n') as f:
        for d in found:
            f.write('%s|%06x|core/tiny_%s|%s\n' % (d['name'], d['addr'], tag, d['doc']))
    print('wrote %s.{h,cpp}, %s, %s' % (out, bench, spec))


if __name__ == '__main__':
    main()

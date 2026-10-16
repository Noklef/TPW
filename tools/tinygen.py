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
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from tinysym import symbolic  # noqa: E402
from tinycall import deleter as deleter_via, parse_decls, wrapper  # noqa: E402

_CALLS = {}


def call_tables():
    """(the names symbols.csv gives ported functions by address, the C++ declarations by name), read once."""
    if not _CALLS:
        known = {}
        for line in open(os.path.join(ROOT, 're/symbols.csv'), encoding='utf-8', errors='replace'):
            p = line.split(',')
            if p[0].startswith('0x') and len(p) > 3 and p[2] == 'function' and 'reimplemented' in line:
                known[int(p[0], 16)] = p[1]
        heads = []
        base = os.path.join(ROOT, 'src/game')
        for path in glob.glob(os.path.join(base, '**', '*.h'), recursive=True):
            heads.append((os.path.relpath(path, base).replace(os.sep, '/'), open(path, errors='replace').read()))
        _CALLS['k'], _CALLS['d'] = known, parse_decls(heads)
    return _CALLS['k'], _CALLS['d']

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
    calls = collections.defaultdict(set)
    indirect = set()
    cur, j = None, 0
    for line in open(os.path.join(ROOT, '_re/text.asm'), errors='replace'):
        try:
            va = int(line[:8], 16)
        except ValueError:
            continue
        while j < len(ks) and va >= ks[j]:
            cur, j = ks[j], j + 1
        if cur is not None and va < end.get(cur, cur):
            m = re.match(r'^(?:CALL|JMP) 0x([0-9a-f]+)$', line[8:].strip())
            if m:
                calls[cur].add(int(m.group(1), 16))
            elif re.match(r'^(?:CALL|JMP) ', line[8:].strip()):
                indirect.add(cur)
        if cur is None or not lo <= cur < hi or cur in hooked or va >= end.get(cur, cur):
            continue
        body[cur].append((va, norm(line[8:])))
    return body, end, calls, indirect


@shape(r'MOV EAX,\[ESP \+ 0x4\]', r'MOV \[EAX\],(-?0x[0-9a-f]+)', r'RET (0x[0-9a-f]+)')
def _ptrstore(a, m):
    c, n = num(m[1].group(1)) & 0xffffffff, num(m[2].group(1)) // 4
    return dict(name='StoreThrough_%x_%06x' % (c, a), conv='s', nargs=n, ret='a', doc='*first argument = 0x%x; returns that pointer' % c,
                body='at<u32>((void *)(uintptr_t)v0, 0) = 0x%x;\n    return v0;' % c)


@shape(r'MOV EAX,\[ESP \+ 0x4\]', r'MOV ECX,%s' % FIELD, r'MOV \[EAX\],ECX', r'RET 0x4')
def _ptrcopy(a, m):
    k = num(m[1].group(1))
    return dict(name='CopyTo_%s_%06x' % (nm(k), a), conv='t', nargs=1, ret='a', doc='*argument = the dword at %s; returns the pointer' % hexs(k),
                body='at<u32>((void *)(uintptr_t)v0, 0) = at<u32>(o, %s);\n    return v0;' % offs(k))


@shape(r'MOV EAX,\[ESP \+ 0x4\]', r'MOV \[ECX \+ (0x[0-9a-f]+)\],EAX', r'MOV EAX,\[ECX \+ (0x[0-9a-f]+)\]',
       r'OR (AL|AH|EAX),(0x[0-9a-f]+)', r'MOV \[ECX \+ (0x[0-9a-f]+)\],EAX', r'RET 0x4')
def _flagset(a, m):
    k1, k2, k3 = num(m[1].group(1)), num(m[2].group(1)), num(m[4].group(1))
    if k2 != k3:
        return None
    bits = num(m[3].group(2)) << (8 if m[3].group(1) == 'AH' else 0)
    return dict(name='SetThenFlag%x_%x_%06x' % (k1, k2, a), conv='t', nargs=1, ret='a',
                doc='+0x%x = value; the flags at +0x%x |= 0x%x; returns the flags' % (k1, k2, bits),
                body='at<u32>(o, 0x%x) = v0;\n    at<u32>(o, 0x%x) |= 0x%x;\n    return at<u32>(o, 0x%x);' % (k1, k2, bits, k2))


@shape(r'MOV EAX,\[ECX \+ (0x[0-9a-f]+)\]', r'OR EAX,(0x[0-9a-f]+)', r'MOV \[ECX \+ (0x[0-9a-f]+)\],EAX', r'MOV EAX,\[ESP \+ 0x4\]',
       r'MOV \[ECX \+ (0x[0-9a-f]+)\],EAX', r'RET 0x4')
def _flagthenset(a, m):
    k1, bits, k2, k3 = num(m[0].group(1)), num(m[1].group(1)), num(m[2].group(1)), num(m[4].group(1))
    if k1 != k2:
        return None
    return dict(name='FlagThenSet%x_%x_%06x' % (k1, k3, a), conv='t', nargs=1, ret='a',
                doc='the flags at +0x%x |= 0x%x; +0x%x = value; returns it' % (k1, bits, k3),
                body='at<u32>(o, 0x%x) |= 0x%x;\n    at<u32>(o, 0x%x) = v0;\n    return v0;' % (k1, bits, k3))


@shape(r'MOV EDX,%s' % FIELD, r'XOR EAX,EAX', r'CMP EDX,(0x[0-9a-f]+)', r'SETZ AL', r'RET')
def _feq(a, m):
    k, c = num(m[0].group(1)), num(m[2].group(1))
    return dict(name='Is%s_Eq%x_%06x' % (nm(k), c, a), conv='t', nargs=0, ret='a', doc='1 when the dword at %s is 0x%x, else 0' % (hexs(k), c),
                body='return at<u32>(o, %s) == 0x%x ? 1 : 0;' % (offs(k), c))


@shape(r'MOV EDX,%s' % FIELD, r'XOR EAX,EAX', r'TEST EDX,EDX', r'SETZ AL', r'RET')
def _fzero(a, m):
    k = num(m[0].group(1))
    return dict(name='Is%s_Zero_%06x' % (nm(k), a), conv='t', nargs=0, ret='a', doc='1 when the dword at %s is 0, else 0' % hexs(k),
                body='return at<u32>(o, %s) == 0 ? 1 : 0;' % offs(k))


@shape(r'OR AX,(-?0x[0-9a-f]+)', r'RET(?: (0x[0-9a-f]+))?')
def _orax(a, m):
    c = num(m[0].group(1)) & 0xffff
    n = num(m[1].group(1)) // 4 if m[1].group(1) else 0
    return dict(name='WordConst%x_%06x' % (c, a), conv='s' if n else 'c', nargs=n, ret='w', doc='returns the word 0x%x' % c, body='return 0x%x;' % c)


def ctor(a, texts):
    """MOV EAX,ECX; [XOR ECX,ECX | XOR CL,CL | MOV ECX,[ESP + 4]]; stores through EAX; RET [4]: a constructor that fills fields."""
    if texts[0] != 'MOV EAX,ECX' or len(texts) < 3:
        return None
    m = re.match(r'^RET(?: (0x[0-9a-f]+))?$', texts[-1])
    if not m:
        return None
    n = num(m.group(1)) // 4 if m.group(1) else 0
    ecx = None  # what ECX holds after the first instruction(s)
    stores = []
    for t in texts[1:-1]:
        if t in ('XOR ECX,ECX', 'XOR CL,CL'):
            ecx = '0'
        elif t == 'MOV ECX,[ESP + 0x4]' and n == 1:
            ecx = 'v0'
        else:
            mm = re.match(r'^MOV (b?)\[EAX(?: \+ (0x[0-9a-f]+))?\],(ECX|CL|-?0x[0-9a-f]+)$', t)
            if not mm:
                return None
            width = 'u8' if mm.group(1) else 'u32'
            k = num(mm.group(2))
            src = mm.group(3)
            if src in ('ECX', 'CL'):
                if ecx is None or (src == 'CL' and width != 'u8') or (src == 'ECX' and width != 'u32'):
                    return None
                val = ecx
            else:
                val = '0x%x' % (num(src) & (0xff if width == 'u8' else 0xffffffff))
            stores.append((width, k, val))
    if not stores:
        return None
    fields = ', '.join('+0x%x' % k for _, k, _ in stores)
    body = '\n    '.join('at<%s>(o, 0x%x) = %s;' % (w, k, v) for w, k, v in stores) + '\n    return o;'
    return dict(name='Init_%06x' % a, conv='t', nargs=n, ret='p', doc='sets %s; returns the object' % fields, body=body)


def deleter(a, texts):
    """A scalar deleting destructor with nothing left in the destructor but the vtable store(s): [MOV AL,[ESP + 4]; PUSH ESI;
    MOV ESI,ECX; TEST AL,1; MOV [ESI..],vtable] in any order, JZ over PUSH ESI; CALL Mem_Free; ADD ESP,4, then MOV EAX,ESI; POP ESI; RET 4."""
    tail = ['PUSH ESI', 'CALL 0x0067acb0', 'ADD ESP,0x4', 'MOV EAX,ESI', 'POP ESI', 'RET 0x4']
    if len(texts) < 9 or texts[-6:] != tail or not texts[-7].startswith('JZ '):
        return None
    pre = texts[:-7]
    need = ['MOV AL,b[ESP + 0x4]', 'PUSH ESI', 'MOV ESI,ECX', 'TEST AL,0x1']
    stores = []
    for t in pre:
        mm = re.match(r'^MOV \[ESI(?: \+ (0x[0-9a-f]+))?\],(0x[0-9a-f]+)$', t)
        if mm:
            stores.append('at<u32>(o, 0x%x) = 0x%08x;' % (num(mm.group(1)), num(mm.group(2))))
        elif t in need:
            need.remove(t)
        else:
            return None
    if need or not stores:
        return None
    vt = re.search(r'0x([0-9a-f]+);', stores[0]).group(1).lstrip('0')
    return dict(name='ScalarDelete%s_%06x' % (vt, a), conv='t', nargs=1, ret='p', doc='sets the vtable pointer(s), then frees the object when bit 0 of the flags is set',
                body=' '.join(stores) + ' if (v0 & 1) GameHeap_Free(o); return o;', includes=['core/gameheap.h'])


def listdtor(a, texts):
    """The destructor of a singly linked list (head +4, tail +8, count +0xc, next at +4 of each node) with the vtable store inlined:
    clears the list, freeing each node (Mem_Free)."""
    pats = [r'^PUSH ESI$', r'^PUSH EDI$', r'^MOV EDI,ECX$', r'^MOV ESI,\[EDI \+ 0x4\]$', r'^MOV \[EDI\],(0x[0-9a-f]+)$', r'^TEST ESI,ESI$',
            r'^MOV \[EDI \+ 0x8\],0x0$', r'^MOV \[EDI \+ 0x4\],0x0$', r'^JZ 0x', r'^MOV EAX,ESI$', r'^MOV ESI,\[ESI \+ 0x4\]$', r'^PUSH EAX$',
            r'^CALL 0x0067acb0$', r'^ADD ESP,0x4$', r'^TEST ESI,ESI$', r'^JNZ 0x', r'^MOV \[EDI \+ 0xc\],0x0$', r'^POP EDI$', r'^POP ESI$', r'^RET$']
    if len(texts) != len(pats):
        return None
    ms = [re.match(p, t) for p, t in zip(pats, texts)]
    if not all(ms):
        return None
    vt = num(ms[4].group(1))
    return dict(name='ListDtor%x_%06x' % (vt, a), conv='t', nargs=0, ret='v', doc='a linked list destructor: sets the vtable pointer 0x%x and frees every node' % vt,
                body='const u32 first = at<u32>(o, 4); at<u32>(o, 0) = 0x%08x; at<u32>(o, 8) = 0; at<u32>(o, 4) = 0; '
                     'for (u32 node = first; node;) { const u32 next = at<u32>(reinterpret_cast<const u8 *>(node), 4); GameHeap_Free(reinterpret_cast<void *>(node)); node = next; } '
                     'at<u32>(o, 0xc) = 0;' % vt, includes=['core/gameheap.h'], listy=True)


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
    d = frame(a, texts) or ctor(a, texts) or deleter(a, texts) or listdtor(a, texts)
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
            if d is None:
                continue
            d.setdefault('globals', [])
            d['addr'] = a
            return d
    d = symbolic(a, texts)
    if d:
        d['addr'] = a
        return d
    d = wrapper(a, texts, *call_tables()) or deleter_via(a, texts, *call_tables())
    if d:
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
    return '%s %s %s(%s)' % (d.get('rettype') or RET[d['ret']], CONV[d['conv']], d['name'], ', '.join(args))


def main():
    a = sys.argv[1:]
    if len(a) < 2:
        sys.exit(__doc__)
    lo, hi = int(a[0], 16), int(a[1], 16)
    body, end, calls, indirect = functions(lo, hi)
    found = []
    for addr in sorted(body):
        d = match(addr, body[addr])
        if d:
            d['end'] = end[addr]
            found.append(d)
    def reach(start):
        seen, todo = set(), [start]
        while todo:
            t = todo.pop()
            if t in seen or t not in end or t in (0x5da3c0, 0x67acb0, 0x67acc0):  # these are stood in for (core/gameheap), not run
                continue
            seen.add(t)
            todo += list(calls.get(t, ()))
        return seen

    # a wrapper whose callees call through a register or a vtable can't run on random objects
    found = [d for d in found if not d.get('callee') or not (reach(d['callee']) & indirect)]
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
        incs = sorted(set(h for d in found for h in d.get('includes', [])))
        incs = ['"%s"' % (h[5:] if h.startswith('core/') else '../' + h) for h in incs]
        f.write('#include "tiny_%s.h"\n\n#include <cstdint>\n\n' % tag + ''.join('#include %s\n' % i for i in incs) + '\nnamespace {\n\n'
                'template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }\n'
                'template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }\n\n'
                '}  // namespace\n\nextern "C" {\n\n')
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
                'from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402\n\n')
        f.write('# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)\nFUNCS = [\n')
        for d in found:
            f.write('    (0x%06x, 0x%06x, %r, %d, %r, %r, %r),\n' % (d['addr'], d['end'], d['conv'], d['nargs'], 'a' if d['ret'] in 'ap' else d['ret'], d['globals'], tuple(sorted(set(int(x) for x in re.findall(r'\(uintptr_t\)v(\d)', d['body']))))))
        # what the wrappers call, and what that calls: the originals run in the emulator
        extra, todo = set(), [d['callee'] for d in found if d.get('callee')]
        while todo:
            t = todo.pop()
            if t in extra or t not in end or t in (0x67acc0, 0x67acb0, 0x67bb90):
                continue
            extra.add(t)
            todo += list(calls.get(t, ()))
        extra_ranges = [(t, end[t]) for t in sorted(extra)]
        listy = tuple(d['addr'] for d in found if d.get('listy'))
        f.write(']\nEXTRA = %r\nLISTY = %r  # linked list destructors: a real chain (next at +4) is built for them\nB = Bench([(a, e) for a, e, *_ in FUNCS] + EXTRA)\n\n\n' % (extra_ranges, listy) +
                'def case(r):\n'
                '    address, _end, conv, n, ret, globs, ptr = r.choice(FUNCS)\n'
                '    obj = bytearray(r.bytes(0x400))\n'
                '    for k in range(0, 0x400, 4):  # many dwords point into the object, so a chain of pointers can be followed\n'
                '        if r.random() < 0.5:\n'
                '            put(obj, k, \'I\', RAM + 0x100 + 4 * r.randrange(0x40))\n'
                '    regions = {RAM: obj}\n' + ('    regions[HEAP] = heap(0x1000)  # the game heap the code frees into\n' if any('GameHeap_' in d['body'] for d in found) else '') +
                ('    if address in LISTY:\n'
                 '        count = r.choice([0, 1, 2, 3, 6])\n'
                 '        chain = bytearray(r.bytes(0x200))\n'
                 '        for i in range(count):\n'
                 '            put(chain, 0x20 * i + 4, \'I\', RAM + 0x1000 + 0x20 * (i + 1) if i + 1 < count else 0)\n'
                 '        put(obj, 0x84, \'I\', RAM + 0x1000 if count else 0)  # the object is at RAM + 0x80\n'
                 '        regions[RAM + 0x1000] = chain\n' if any(d.get('listy') for d in found) else '') +
                '    for g in globs:\n'
                '        regions[g & ~0xf] = r.bytes(0x20)\n'
                '    args = [r.choice([0, 1, 0xff, 0x7fffffff, 0xffffffff, r.getrandbits(32)]) for _ in range(n)]\n'
                '    for i in ptr:\n'
                '        args[i] = RAM + 0x200 + 0x40 * min(i, 3) + r.choice([0, 4, 0x20])\n'
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

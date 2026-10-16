"""The porting queue: unported functions that are ready to port, grouped by the object they belong to.

A function is ready when it has no indirect calls or jumps and calls nothing unported (or at most
--calls N unported targets). Each is placed in a group, the first that applies:
  vt:ADDR    a vtable that holds it (from the .rdata dwords that point at it)
  this:ADDR  the global object its callers pass in ECX (MOV ECX,imm before the CALL)
  near:NAME  the nearest named function below it (its unit, roughly)
Groups are ranked by how much of the object is already ported (its other vtable slots / ECX
callees that are hooked), so similar, already-understood work comes first; within a group,
smallest first. Each line: address, size, x87 instruction count, callers (as their nearest
symbol), the first instructions. Tail JMPs out of the body count as calls; adjustor and JMP thunks are left out.

Inputs: _re/text.asm (the Ghidra listing), _re/decomp-typed (function starts), src/hook/hooks.cpp,
re/symbols.csv and the no-CD exe (data references).

usage: python -I tools/leafq.py LO HI [--calls N] [--groups N] [--peek N]
"""
import bisect
import collections
import glob
import os
import re
import struct
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
EXE = os.path.join(ROOT, '.data', 'nocd', 'tp.exe')


PROVIDED = {0x5da3c0, 0x67acb0, 0x67acc0,  # Debug_Printf, operator delete / new (core/gameheap)
            0x67df40, 0x67b5f0, 0x67bfc0, 0x67c250, 0x67c080, 0x67f1a0, 0x67fa60, 0x67a520, 0x67a650, 0x67acd0}  # the CRT calls of core/crt (CrtEnv)


def opt(name, default):
    a = sys.argv
    return int(a[a.index(name) + 1]) if name in a else default


def sections(d):
    pe = struct.unpack_from('<I', d, 0x3c)[0]
    n = struct.unpack_from('<H', d, pe + 6)[0]
    size = struct.unpack_from('<H', d, pe + 20)[0]
    out = []
    for i in range(n):
        o = pe + 24 + size + 40 * i
        name = d[o:o + 8].rstrip(b'\0').decode()
        vsize, va, rsize, raw = struct.unpack_from('<IIII', d, o + 8)
        out.append((name, 0x400000 + va, rsize, raw))
    return out


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    lo, hi = int(sys.argv[1], 16), int(sys.argv[2], 16)
    maxcalls, ngroups, peek = opt('--calls', 0), opt('--groups', 25), opt('--peek', 90)

    hooked = set(int(x, 16) for x in re.findall(r'0x00([0-9a-f]{6})', open(os.path.join(ROOT, 'src', 'hook', 'hooks.cpp')).read()))
    # Not hooked, but the ported code has a way through: the empty debug printf (the C++ simply does not call it) and the game's
    # operator new / delete (core/gameheap calls them, or the tests' stand-in).
    hooked_or_provided = hooked | PROVIDED
    names = {}
    for line in open(os.path.join(ROOT, 're', 'symbols.csv'), encoding='utf-8', errors='replace'):
        p = line.split(',')
        if p[0].startswith('0x') and len(p) > 2 and p[2] == 'function':
            names[int(p[0], 16)] = p[1]
    named = sorted(names)
    starts = set()
    for f in glob.glob(os.path.join(ROOT, '_re', 'decomp-typed', 'decomp_*.c')):
        for m in re.finditer(r'^// ==== (\S+) @ ([0-9a-f]{8}) ====', open(f, errors='replace').read(), re.M):
            starts.add(int(m.group(2), 16))
    ks = sorted(starts)
    end = {a: ks[i + 1] for i, a in enumerate(ks[:-1])}

    # One pass over the listing: each function's instructions, calls and x87 use; every call site
    # with the ECX immediate loaded just before it.
    body = collections.defaultdict(list)
    info = {}  # start -> [x87 count, unported targets, indirect]
    calls = collections.defaultdict(list)  # target -> [(site, ecx imm or None)]
    ecx = None
    cur, j = None, 0
    for line in open(os.path.join(ROOT, '_re', 'text.asm'), errors='replace'):
        try:
            va = int(line[:8], 16)
        except ValueError:
            continue
        ins = line[8:].strip()
        m = re.match(r'(CALL|JMP) 0x([0-9a-f]+)$', ins)
        if m and m.group(1) == 'CALL':
            calls[int(m.group(2), 16)].append((va, ecx))
        mi = re.match(r'MOV ECX,0x([0-9a-f]+)$', ins)
        ecx = int(mi.group(1), 16) if mi else (None if ins.startswith(('CALL', 'MOV ECX', 'LEA ECX', 'POP ECX')) else ecx)
        while j < len(ks) and va >= ks[j]:
            cur, j = ks[j], j + 1
        if cur is None or not lo <= cur < hi or cur in hooked or va >= end.get(cur, cur):
            continue
        rec = info.setdefault(cur, [0, set(), False])
        body[cur].append(ins.replace('dword ptr ', '').replace('word ptr ', 'w').replace('byte ptr ', 'b'))
        tail = m and m.group(1) == 'JMP' and not cur <= int(m.group(2), 16) < end.get(cur, cur)
        if ins.startswith('CALL') or tail:
            if not m:
                rec[2] = True
            elif int(m.group(2), 16) not in hooked_or_provided:
                rec[1].add(int(m.group(2), 16))
        elif ins.startswith('JMP') and not m:
            rec[2] = True
        if re.match(r'F[A-Z]', ins):
            rec[0] += 1

    ready = [a for a, r in info.items() if not r[2] and len(r[1]) <= maxcalls
             and not body[a][0].startswith(('SUB ECX,[ECX + -0x', 'JMP'))]

    # Vtables: .rdata dwords pointing at code; a slot's table starts where the run of code
    # pointers before it does.
    d = open(EXE, 'rb').read()
    secs = sections(d)
    text = next(s for s in secs if s[0] == '.text')
    slots, tables = collections.defaultdict(list), {}
    for name, va, rsize, raw in secs:
        if name != '.rdata':
            continue
        words = struct.unpack_from('<%dI' % (rsize // 4), d, raw)
        start = None
        for i, w in enumerate(words):
            if text[1] <= w < text[1] + text[2]:
                start = va + 4 * i if start is None else start
                slots[w].append(start)
                tables.setdefault(start, []).append(w)
            else:
                start = None

    def nearest(a):
        i = bisect.bisect_right(named, a) - 1
        return names[named[i]] if i >= 0 else '?'

    groups = collections.defaultdict(list)
    for a in ready:
        if slots.get(a):
            key = 'vt:%x' % slots[a][0]
        else:
            objs = collections.Counter(e for _, e in calls.get(a, []) if e)
            key = 'this:%x' % objs.most_common(1)[0][0] if objs else 'near:' + nearest(a)
        groups[key].append(a)

    def done(key):
        kind, _, v = key.partition(':')
        if kind == 'vt':
            members = tables.get(int(v, 16), [])
        elif kind == 'this':
            members = {t for t, sites in calls.items() if any(e == int(v, 16) for _, e in sites)}
        else:
            return sum(1 for h in hooked if nearest(h) == v), 0
        members = set(members)
        return len(members & hooked), len(members)

    ranked = sorted(groups, key=lambda k: (-done(k)[0], k))
    total = sum(len(v) for v in groups.values())
    print('%d ready in %x-%x, %d groups (showing %d)' % (total, lo, hi, len(groups), min(ngroups, len(groups))))
    for key in ranked[:ngroups]:
        h, n = done(key)
        print('== %s  (%d ported%s)' % (key, h, ' of %d' % n if n else ''))
        for a in sorted(groups[key], key=lambda a: (end.get(a, a) - a, a)):
            who = sorted({nearest(s)[:20] for s, _ in calls.get(a, [])})
            print('  %x %4d fpu=%-2d [%s] %s' % (a, end.get(a, a) - a, info[a][0], ' '.join(who[:3]),
                                                '; '.join(body[a])[:peek]))


if __name__ == '__main__':
    main()

"""Recover C++ class structure from vtables.

Inputs: the decrypted exe, the raw decompile dump (_re/decomp-raw) and _re/string_hints.tsv
(from tools/string_hints.py). Outputs a TSV report (git-ignored: it quotes game strings).

For every vtable address stored into an object (`*this = &PTR_..._<addr>;`):
  - methods:   consecutive code pointers read from the vtable in the exe
  - ctors:     functions that store the vtable at offset 0 (constructors/destructors)
  - base:      vtable stored at offset 0 by a callee of one of our ctors (base-class ctor)
  - hints:     debug strings referenced by its methods and ctors (for naming)

Usage: python -I tools/vtables.py <exe> <decomp_dir> <string_hints.tsv> <out.tsv>
"""
import glob
import os
import re
import sys
from collections import Counter, defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pe_read import PE  # noqa: E402

exe, dump_dir, hints_path, out_path = sys.argv[1:5]
pe = PE(exe)
CODE = [(s[1], s[1] + s[2]) for s in pe.sections if s[0].lower() in ('.text', 'cseg')]


def is_code(va):
    return va is not None and any(lo <= va < hi for lo, hi in CODE)


HEADER = re.compile(r'^// ==== (\S+) @ ([0-9a-f]{8}) ====$')
# "*param_1 = &PTR_FUN_006fd708;" (offset 0) or "*(T *)(param_1 + 0x24) = &PTR_...;" (subobject)
STORE = re.compile(r'^\s*\*(?:\([^)]*\)\s*\(\s*(\w+)\s*\+\s*(0x[0-9a-f]+|\d+)\s*\)|(\w+))\s*=\s*&?PTR_\w*?_([0-9a-f]{8});')
CALL = re.compile(r'\b(?:FUN|thunk_FUN)_([0-9a-f]{8})\(')

stores = defaultdict(list)      # func addr -> [(vtable, offset)]
callees = defaultdict(set)      # func addr -> {callee addr}
names = {}
cur = None
for path in sorted(glob.glob(os.path.join(dump_dir, 'decomp_*.c'))):
    for line in open(path, encoding='utf-8', errors='replace'):
        m = HEADER.match(line.rstrip('\n'))
        if m:
            cur = m.group(2)
            names[cur] = m.group(1)
            continue
        if cur is None:
            continue
        s = STORE.match(line)
        if s:
            off = int(s.group(2), 0) if s.group(2) else 0
            stores[cur].append((int(s.group(4), 16), off))
        callees[cur].update(CALL.findall(line))

hints = {}
for line in open(hints_path, encoding='utf-8'):
    p = line.rstrip('\n').split('\t')
    if p[0] != 'address' and len(p) >= 6 and p[5]:
        hints[p[0][2:]] = p[5].split(' | ')

# vtable -> ctors (stored at offset 0), plus where it appears as an embedded subobject
ctors = defaultdict(set)
embedded = defaultdict(set)
for f, lst in stores.items():
    for vt, off in lst:
        (ctors if off == 0 else embedded)[vt].add(f)


VT_STARTS = []  # filled below; no RTTI means vtables are packed back-to-back


def methods_of(vt):
    out = []
    va = vt
    nxt = next((s for s in VT_STARTS if s > vt), None)
    while nxt is None or va < nxt:
        p = pe.u32(va)
        if not is_code(p):
            break
        out.append(p)
        va += 4
        if len(out) > 400:
            break
    return out


vtables = {vt for f in stores for vt, _ in stores[f]}
VT_STARTS.extend(sorted(vtables))
vt_by_ctor = defaultdict(set)
for vt, fs in ctors.items():
    for f in fs:
        vt_by_ctor[f].add(vt)

rows = []
for vt in sorted(vtables):
    meths = methods_of(vt)
    if not meths:
        continue
    # base class: a callee of our ctor that itself stores a *different* vtable at offset 0
    bases = Counter()
    for f in ctors.get(vt, ()):
        for c in callees.get(f, ()):
            for bvt in vt_by_ctor.get(c, ()):
                if bvt != vt:
                    bases[bvt] += 1
    words = Counter()
    sample = []
    for f in [f'{m:08x}' for m in meths] + sorted(ctors.get(vt, ())):
        for s in hints.get(f, []):
            sample.append(s)
            for w in re.findall(r'[A-Z][a-z]+|[A-Z]{3,}', s):
                words[w] += 1
    rows.append((vt, len(meths), sorted(ctors.get(vt, ())), [f'{b:x}' for b, _ in bases.most_common(2)],
                 sorted(embedded.get(vt, ())), words, sample))

with open(out_path, 'w', encoding='utf-8') as out:
    out.write('vtable\tn_methods\tctors\tbase_vtables\tembedded_in\ttop_words\tsample_strings\n')
    for vt, n, cs, bs, em, words, sample in rows:
        out.write(f"0x{vt:08x}\t{n}\t{','.join(cs)}\t{','.join(bs)}\t{','.join(em)}\t"
                  f"{','.join(w for w, _ in words.most_common(6))}\t{' | '.join(sample[:6])}\n")

named = sum(1 for r in rows if r[6])
with_base = sum(1 for r in rows if r[3])
methods_total = len({m for vt in vtables for m in methods_of(vt)})
print(f'vtables: {len(rows)} (of {len(vtables)} stored pointers); with base class: {with_base}; '
      f'with string hints: {named}; distinct virtual methods: {methods_total}')
print('largest:', ', '.join(f'0x{r[0]:x}({r[1]})' for r in sorted(rows, key=lambda r: -r[1])[:10]))

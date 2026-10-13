"""Build per-function naming hints from the raw Ghidra decompile dump.

For every function in _re/decomp-raw/decomp_*.c, collect the string literals it references
(Ghidra renders them as s_Some_text_<addr>) and its callees. Writes a TSV report
(git-ignored, since it quotes game strings) and prints coverage statistics.

Usage: python -I tools/string_hints.py _re/decomp-raw _re/string_hints.tsv
"""
import glob
import os
import re
import sys
from collections import Counter

dump_dir, out_path = sys.argv[1], sys.argv[2]

HEADER = re.compile(r'^// ==== (\S+) @ ([0-9a-f]{8}) ====$')
STRREF = re.compile(r'\bs_([A-Za-z0-9_]+?)_([0-9a-f]{8})\b')
QUOTED = re.compile(r'"((?:[^"\\]|\\.){4,})"')
CALL = re.compile(r'\b(FUN_[0-9a-f]{8})\(')

funcs = {}  # address -> dict(name, strings, callees)
cur = None
for path in sorted(glob.glob(os.path.join(dump_dir, 'decomp_*.c'))):
    with open(path, encoding='utf-8', errors='replace') as f:
        for line in f:
            m = HEADER.match(line.rstrip('\n'))
            if m:
                cur = funcs.setdefault(m.group(2), {'name': m.group(1), 'strings': [], 'callees': set()})
                continue
            if cur is None:
                continue
            found = [s for s, _ in STRREF.findall(line)]
            # Ghidra also inlines some literals as "text"; normalise to the s_ style.
            found += [re.sub(r'\W+', '_', s).strip('_') for s in QUOTED.findall(line)]
            for s in found:
                if len(s) >= 4 and s not in cur['strings']:
                    cur['strings'].append(s)
            cur['callees'].update(CALL.findall(line))

# A prefix like "RSSE_" or "PAR_" or "Ride_" in the referenced strings hints at the subsystem.
def prefix_hint(strings):
    c = Counter()
    for s in strings:
        m = re.match(r'([A-Z][A-Za-z0-9]{2,})_', s)
        if m:
            c[m.group(1)] += 1
    return ','.join(f'{k}:{v}' for k, v in c.most_common(3))

unnamed = {a: f for a, f in funcs.items() if f['name'].startswith('FUN_')}
with_str = {a: f for a, f in unnamed.items() if f['strings']}

with open(out_path, 'w', encoding='utf-8') as out:
    out.write('address\tname\tn_strings\tn_callees\tprefix_hint\tstrings\n')
    for a in sorted(funcs):
        f = funcs[a]
        out.write(f"0x{a}\t{f['name']}\t{len(f['strings'])}\t{len(f['callees'])}\t"
                  f"{prefix_hint(f['strings'])}\t{' | '.join(f['strings'][:8])}\n")

print(f"functions: {len(funcs)}  unnamed: {len(unnamed)}  "
      f"unnamed with >=1 string: {len(with_str)} ({100 * len(with_str) / max(1, len(unnamed)):.1f}%)")
top = Counter()
for f in with_str.values():
    for k in prefix_hint(f['strings']).split(','):
        if k:
            top[k.split(':')[0]] += 1
print("most common subsystem prefixes:", ', '.join(f'{k}({v})' for k, v in top.most_common(30)))

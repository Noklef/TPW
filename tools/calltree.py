"""Print a function's callee tree from the decompile dump, with size and string hints.

Usage: python -I tools/calltree.py <decomp_dir> <hex_addr> [depth=3]
"""
import csv
import glob
import os
import re
import sys

dump_dir, root = sys.argv[1], sys.argv[2].lower().replace('0x', '').zfill(8)
depth = int(sys.argv[3]) if len(sys.argv) > 3 else 3

HEADER = re.compile(r'^// ==== (\S+) @ ([0-9a-f]{8}) ====$')
CALL = re.compile(r'\b(?:FUN|thunk_FUN)_([0-9a-f]{8})\(')
STR = re.compile(r'\bs_([A-Za-z0-9_]+?)_[0-9a-f]{8}\b')
names, calls, strs = {}, {}, {}
cur = None
for path in sorted(glob.glob(os.path.join(dump_dir, 'decomp_*.c'))):
    for line in open(path, encoding='utf-8', errors='replace'):
        m = HEADER.match(line.rstrip('\n'))
        if m:
            cur = m.group(2); names[cur] = m.group(1); calls[cur] = []; strs[cur] = []
            continue
        if cur:
            for c in CALL.findall(line):
                if c != cur and c not in calls[cur]:  # skip the signature line
                    calls[cur].append(c)
            strs[cur] += [s for s in STR.findall(line) if s not in strs[cur]]
sizes = {r['address']: r['size'] for r in csv.DictReader(open(os.path.join(dump_dir, 'functions.tsv')), delimiter='\t')}

seen = set()


def walk(a, d, indent):
    tag = ' (seen)' if a in seen else ''
    s = ' | '.join(strs.get(a, [])[:3])
    print(f"{indent}{names.get(a, a)} [{sizes.get(a, '?')}b]{tag}  {s[:90]}")
    if a in seen or d == 0:
        return
    seen.add(a)
    for c in calls.get(a, []):
        walk(c, d - 1, indent + '  ')


walk(root, depth, '')

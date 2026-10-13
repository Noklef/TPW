"""Split a decompiled switch statement into per-case files for easier reading.

Usage: python -I tools/split_switch.py <decomp_dir> <function_name> <labels.csv> <out_dir>
labels.csv: CSV with 'index' and 'mnemonic' columns (e.g. re/rsse_opcodes.csv).
Writes one file per case (<index>_<MNEMONIC>.c) plus an index with line counts and callees.
"""
import csv
import glob
import os
import re
import sys

dump_dir, func, labels_csv, out_dir = sys.argv[1:5]
labels = {int(r['index']): r['mnemonic'] for r in csv.DictReader(open(labels_csv))}

body = None
for path in sorted(glob.glob(os.path.join(dump_dir, 'decomp_*.c'))):
    text = open(path, encoding='utf-8', errors='replace').read()
    m = re.search(rf'^// ==== {re.escape(func)} @ [0-9a-f]+ ====\n(.*?)(?=^// ==== )', text, re.S | re.M)
    if m:
        body = m.group(1)
        break
if body is None:
    sys.exit(f'{func} not found')

lines = body.split('\n')
# find the outermost switch: the first 'switch(' line; its cases are at that indentation + 2
sw = next(i for i, l in enumerate(lines) if re.match(r'^\s*switch\(', l))
indent = len(lines[sw]) - len(lines[sw].lstrip())
case_re = re.compile(r'^' + ' ' * indent + r'case (0x[0-9a-f]+|\d+):|^' + ' ' * indent + r'default:')
starts = []
for i in range(sw + 1, len(lines)):
    m = case_re.match(lines[i])
    if m:
        starts.append((i, int(m.group(1), 0) if m.group(1) else -1))
    elif lines[i].startswith(' ' * indent + '}') and len(lines[i]) - len(lines[i].lstrip()) == indent:
        starts.append((i, None))
        break

os.makedirs(out_dir, exist_ok=True)
index = []
for (a, val), (b, _) in zip(starts, starts[1:]):
    if val is None:
        continue
    name = labels.get(val, 'default' if val == -1 else f'case_{val:#x}')
    chunk = lines[a:b]
    calls = sorted(set(re.findall(r'\b([A-Za-z_][A-Za-z0-9_]*)\(', '\n'.join(chunk))) -
                   {'if', 'while', 'for', 'switch', 'return', 'CONCAT31', 'CONCAT22', 'SUB41', 'SUB42'})
    fn = f'{val:03d}_{name}.c' if val >= 0 else 'default.c'
    open(os.path.join(out_dir, fn), 'w', encoding='utf-8').write('\n'.join(chunk) + '\n')
    index.append((val, name, len(chunk), calls))

with open(os.path.join(out_dir, 'INDEX.txt'), 'w', encoding='utf-8') as f:
    for val, name, n, calls in sorted(index):
        f.write(f'{val:4d} {name:<16} {n:5d} lines  {" ".join(calls)[:150]}\n')
print(f'{len(index)} cases from {func} (switch at line {sw}) -> {out_dir}')

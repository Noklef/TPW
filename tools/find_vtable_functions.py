"""List code addresses reachable only through vtables that Ghidra hasn't made functions.

Reads the vtable report from tools/vtables.py. For each vtable entry, emits the entry
address and, when the entry is a this-adjusting thunk (`jmp rel32`, optionally preceded
by `sub ecx,[ecx-4]`), the thunk's target too. Feed the output to tools/CreateFunctions.java.

Usage: python -I tools/find_vtable_functions.py <exe> <functions.tsv> <vtables.tsv> <out.txt>
"""
import csv
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pe_read import PE  # noqa: E402

exe, funcs_tsv, vt_tsv, out_path = sys.argv[1:5]
pe = PE(exe)
known = {int(r['address'], 16) for r in csv.DictReader(open(funcs_tsv), delimiter='\t')}
code = [(s[1], s[1] + s[2]) for s in pe.sections if s[0].lower() in ('.text', 'cseg')]


def is_code(v):
    return v is not None and any(lo <= v < hi for lo, hi in code)


def thunk_target(va):
    b = pe.bytes(va, 16) or b''
    for j in (0, 3):  # "jmp rel32" or "sub ecx,[ecx-4]; jmp rel32"
        if len(b) > j + 5 and b[j] == 0xE9:
            return va + j + 5 + struct.unpack('<i', b[j + 1:j + 5])[0]
    return None


vts = sorted(int(r['vtable'], 16) for r in csv.DictReader(open(vt_tsv, encoding='utf-8'), delimiter='\t'))
missing = set()
for i, vt in enumerate(vts):
    end = vts[i + 1] if i + 1 < len(vts) else vt + 0x400
    va = vt
    while va < end:
        p = pe.u32(va)
        if not is_code(p):
            break
        for a in (p, thunk_target(p)):
            if is_code(a) and a not in known:
                missing.add(a)
        va += 4

with open(out_path, 'w') as f:
    f.write('# Virtual methods / thunk targets reachable only via vtables (tools/find_vtable_functions.py)\n')
    f.write('\n'.join(f'{a:#x}' for a in sorted(missing)) + '\n')
print(f'{len(missing)} addresses -> {out_path}')

"""List the save-side field schema of the game's serializers, from the Ghidra decompile dump.

Every serializer announces each field through Save_FieldName (FUN_00416e10), called as
(size, "mName", isLoad). This prints, per function and in source order, the save-side
items only:
  F <size> <name>     a field written directly
  C <addr> (...)      a call to another serializer (base class / sub-object / typed helper)
  do { ... } while    loops (the counts come from the while condition)
Load-side calls (2nd argument literal 0) and the read-only helpers are skipped. Loops whose
condition itself calls helpers (e.g. the bank's 8 loans) are not shown; check the source. Use it as a
starting point for tools/tpsave.py schemas, then confirm against a real file: the world
parser checks it ends exactly on the module delimiter.

Usage:
  python -I tools/save_schema.py _re/decomp-raw [addr ...]   (addr like 004fb530; default all)
"""
import glob
import re
import sys

READ_HELPERS = {'004d37e0', '004fae10', '00424c20', '00510240', '005134d0'}


def load_functions(dump_dir):
    src = {}
    for path in sorted(glob.glob(dump_dir + '/decomp_*.c')):
        cur = None
        for line in open(path, encoding='utf-8', errors='replace'):
            m = re.match(r'// ==== (\S+) @ ([0-9a-f]+) ====', line)
            if m:
                cur = m.group(2)
                src[cur] = [m.group(1), '']
                continue
            if cur:
                src[cur][1] += line
    return src


def show(addr, src, serializers):
    name, body = src[addr]
    out = [f'=== {name} @ {addr}']
    body = re.sub(r'\s+', ' ', body)
    for stmt in re.split(r'(?<=[;{}])', body):
        s = stmt.strip()
        m = re.search(r'FUN_00416e10\((0x[0-9a-f]+|\d+),\s*(?:\(char \*\))?([^,]+?),\s*0\)', s)
        if m:
            out.append(f'  F {int(m.group(1), 0)} {m.group(2)}')
            continue
        if s == 'do {':
            out.append('  do {')
            continue
        m = re.match(r'\}? ?while \((.*)\);$', s)
        if m and 'FUN_' not in s:
            out.append(f'  }} while ({m.group(1)[:60]})')
            continue
        for m in re.finditer(r'FUN_(00[0-9a-f]{6})\(([^;]*?)\)', s):
            callee, args = m.group(1), [a.strip() for a in m.group(2).split(',')]
            if callee in READ_HELPERS or callee == addr or callee == '00416e10':
                continue
            if callee in serializers and not (len(args) > 1 and args[1] == '0'):
                out.append(f'  C {callee} ({m.group(2)[:60]})')
    return out


def main():
    dump_dir, wanted = sys.argv[1], sys.argv[2:]
    src = load_functions(dump_dir)
    serializers = {a for a, (_, body) in src.items() if 'FUN_00416e10(' in body}
    for addr in wanted or sorted(serializers):
        print('\n'.join(show(addr, src, serializers)))


if __name__ == '__main__':
    main()

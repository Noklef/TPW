# Finds Cd(0xADDR) calls in src/game whose target takes ECX or EDX: it reads the register before writing it, itself or through a direct call or
# jump it makes first (0x582170 passes ECX on to ModelList_Draw). A Cd leaves that register stale, and a cdecl recorder in a bench hides it: two
# crashes and a world without models in the user's runs came this way. Calls through envs and tables are not seen. Exit code 1 if any.
# usage: cdecx.py [--push]   (--push: also those whose first use is a PUSH, usually a stack slot, not an argument)
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
FAM = {'ECX': r'\b(ECX|CX|CL|CH)\b', 'EDX': r'\b(EDX|DX|DL|DH)\b'}
PRESERVING = {0x67b110}   # __chkstk keeps ECX and EDX for the code after it
ins = {}
order = []
for line in open(os.path.join(ROOT, '_re', 'text.asm'), errors='replace'):
    m = re.match(r'([0-9a-f]{8})\s+(.*)', line)
    if m:
        ins[int(m.group(1), 16)] = m.group(2).strip()
        order.append(int(m.group(1), 16))
index = {a: i for i, a in enumerate(order)}


def first_use(addr, reg, depth=0):
    i = index.get(addr)
    if i is None or depth > 3:
        return None
    for a in order[i:i + 40]:
        text = ins[a]
        op = text.split()[0] if text else ''
        args = text[len(op):].strip()
        dest, _, src = args.partition(',')
        target = re.fullmatch(r'0x([0-9a-f]+)', args)
        if op in ('CALL', 'JMP') and target:
            t = int(target.group(1), 16)
            if t in PRESERVING:
                continue
            return first_use(t, reg, depth + 1)
        if op == 'RET' or op in ('CALL', 'JMP') or op.startswith('J'):
            return ('read', text) if re.search(FAM[reg], text) else None
        if not re.search(FAM[reg], text):
            continue
        if op == 'XOR' and dest == src.strip():
            return None
        if op == 'OR' and re.fullmatch(FAM[reg], dest) and src.strip() == '0xffffffff':
            return None
        if op in ('MOV', 'LEA', 'MOVZX', 'MOVSX', 'POP') and re.fullmatch(FAM[reg], dest) and not re.search(FAM[reg], src):
            return None
        return ('push', text) if op == 'PUSH' else ('read', text)
    return None


seen = {}
for d, _, files in os.walk(os.path.join(ROOT, 'src', 'game')):
    for f in files:
        if f.endswith(('.cpp', '.h')):
            path = os.path.join(d, f)
            for n, l in enumerate(open(path, errors='replace'), 1):
                for m in re.finditer(r'\.Cd\(0x([0-9a-fA-F]+)', l):
                    seen.setdefault(int(m.group(1), 16), []).append('%s:%d' % (os.path.relpath(path, os.path.join(ROOT, 'src', 'game')).replace('\\', '/'), n))
found = 0
for a in sorted(seen):
    for reg in ('ECX', 'EDX'):
        r = first_use(a, reg)
        if r and (r[0] == 'read' or '--push' in sys.argv):
            print('%-4s %-4s %x  %-30s %s' % (r[0], reg, a, r[1][:30], ' '.join(seen[a][:3])))
            found += r[0] == 'read'
print('cdecx: %d targets, %d take a register' % (len(seen), found))
sys.exit(1 if found else 0)

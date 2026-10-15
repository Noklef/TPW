"""Finds hooks whose callers use EAX after the call while our port returns void.

A fncmp bench compares what a function writes, not what it leaves in EAX. When the original
returns a value by accident or by design (e.g. Matrix_Invert 0x470040 returns dst, and every
caller copies the result from that pointer) and the port is declared void, callers in tp.exe
read garbage. That only shows up in game.

usage: retcheck.py [LISTING]   (default _re/text.asm)

For each hooked address, scans each caller's next few instructions (skipping the cdecl
ADD ESP) for a read of EAX/AX/AL/AH before it is overwritten, and reports the hooks with such a
caller whose declaration in src/game returns void.
"""
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
listing = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, '_re', 'text.asm')

hooks = re.findall(r'\{"([A-Za-z0-9_]+)",\s*0x([0-9a-fA-F]+),',
                   open(os.path.join(ROOT, 'src', 'hook', 'hooks.cpp')).read())

# Declared return types: "<ret> [__conv] Name(" in the game headers.
decls = {}
for d, _, files in os.walk(os.path.join(ROOT, 'src', 'game')):
    for f in files:
        if f.endswith('.h'):
            for m in re.finditer(r'^\s*([A-Za-z_][\w\s\*]*?)\s*\b(?:__cdecl|__stdcall|__fastcall|__thiscall)?\s*\*?\s*'
                                 r'\b([A-Za-z0-9_]+)\s*\(', open(os.path.join(d, f)).read(), re.M):
                decls.setdefault(m.group(2), m.group(0))

lines = open(listing).read().splitlines()
calls = {}
for i, l in enumerate(lines):
    m = re.match(r'^([0-9a-f]{8})  CALL 0x([0-9a-f]{8})$', l)
    if m:
        calls.setdefault(m.group(2), []).append(i)

EAX = re.compile(r'\b(EAX|AX|AL|AH)\b')
# Instructions that overwrite EAX (or a part of it) without reading it first.
WRITES_ONLY = re.compile(r'^(MOV|LEA|MOVZX|MOVSX) (EAX|AX|AL|AH),(?!.*\b(EAX|AX|AL|AH)\b)'
                         r'|^(XOR|SUB) (EAX,EAX|AX,AX|AL,AL)$|^OR EAX,0xffffffff$|^POP EAX$|^FNSTSW AX$|^SET[A-Z]+ AL$')


def uses_eax(i):
    for l in lines[i + 1:i + 6]:
        ins = l[10:].strip()
        if re.match(r'^ADD ESP,', ins):
            continue
        if WRITES_ONLY.match(ins):
            return None
        if re.match(r'^(CALL|JMP|RET|J[A-Z]+ )', ins) and not EAX.search(ins):
            return None
        if EAX.search(ins):
            return l
    return None


# Checked by hand: they do return the value their callers read.
KNOWN_OK = {
    'App_GetWndProc': 'returns a function pointer; the declaration syntax defeats the parser',
}

found = 0
for name, addr in hooks:
    addr = addr.lower().rjust(8, '0')
    if name in KNOWN_OK:
        continue
    users = [u for u in (uses_eax(i) for i in calls.get(addr, [])) if u]
    if not users:
        continue
    decl = decls.get(name, '(declaration not found)')
    if re.match(r'^\s*void\b(?!\s*\*)', decl) or 'not found' in decl:
        found += 1
        print('%s %s  %d/%d callers read EAX, e.g. %s' % (addr, name, len(users), len(calls[addr]), users[0].strip()))
        print('    port: %s' % decl.strip())
print('%d suspect hooks' % found)
sys.exit(1 if found else 0)

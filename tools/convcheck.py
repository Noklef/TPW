"""Compares the calling convention each hook is declared with against the original function's RET.

A hook replaces the original at its address, so the port must pop exactly what the original popped:
`RET n` in the exe means the callee cleans n bytes (stdcall, or the stack arguments of a thiscall /
fastcall), plain `RET` means the caller does (cdecl). A benches' comparison cannot see this
(it compares what the function writes and returns, not the stack), and in game a wrong one drifts
the stack of every caller.

usage: convcheck.py [LISTING]   (default _re/text.asm)

For each hook row of src/hook/hooks.cpp, finds the declaration in src/game and the RETs of the
original (to the extent of tools/wave.py's extent()), and reports the hooks where the declared
number of stack bytes differs from the RET immediate (all RETs of the function must agree).
"""
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import wave  # noqa: E402

hooks = re.findall(r'\{"([A-Za-z0-9_]+)",\s*0x([0-9a-fA-F]+),', open(os.path.join(ROOT, 'src', 'hook', 'hooks.cpp')).read())

decls = {}
for d, _, files in os.walk(os.path.join(ROOT, 'src', 'game')):
    for f in files:
        if f.endswith('.h'):
            text = open(os.path.join(d, f), errors='replace').read()
            text = re.sub(r'//[^\n]*', '', text)
            for m in re.finditer(r'([A-Za-z_][\w \t\*:<>,]*?)\b(__cdecl|__stdcall|__fastcall|__thiscall)?\s*\*?\s*\b([A-Za-z0-9_]+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)\s*;', text):
                decls.setdefault(m.group(3), (m.group(2) or '', m.group(4)))

idx = wave.index()


def returns(start):
    size = wave.extent(start)
    rets = set()
    for ins in wave.body(start, size, idx):
        m = re.match(r'RET(?: 0x([0-9a-f]+))?$', ins)
        if m:
            rets.add(int(m.group(1), 16) if m.group(1) else 0)
    return rets


SIMPLE = re.compile(r'^(const\s+)?(u8|u16|u32|s8|s16|s32|int|unsigned|char|short|long|float|bool|void|size_t)|\*')


def stack_bytes(conv, params):
    """The bytes of stack the declaration takes, or None when a parameter's size is not known (a struct by value, a double)."""
    ps = [p.strip() for p in re.split(r',(?![^()]*\))', params) if p.strip() and p.strip() != 'void']
    if not all(SIMPLE.search(p) for p in ps):
        return None
    if conv == '__fastcall':
        return max(len(ps) - 2, 0) * 4
    if conv == '__stdcall' or conv == '__thiscall':
        return len(ps) * 4
    return 0 if conv in ('', '__cdecl') and False else None


bad = 0
for name, addr in hooks:
    if name not in decls:
        continue
    conv, params = decls[name]
    start = int(addr, 16)
    try:
        rets = returns(start)
    except KeyError:
        continue
    if not rets:
        continue
    if conv in ('', '__cdecl'):
        declared = 0
    else:
        declared = stack_bytes(conv, params)
    if declared is None or len(rets) != 1:  # (an unknown parameter size, or the extent ran into a neighbour with other RETs)
        continue
    if rets != {declared}:
        bad += 1
        print('%s %s: declared %s %s pops %d, original RET %s' % (addr, name, conv or 'cdecl', '(%s)' % params.strip()[:60], declared, sorted(rets)))
print('%d hooks with a different stack cleanup' % bad)
sys.exit(1 if bad else 0)

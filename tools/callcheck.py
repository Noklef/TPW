"""Compares each call by original address (AddrCall's Th / St / Cd, and the files' own Th / St / Cd helpers) with the original's RET.

A port calls an unported or hooked function through its original address with the arguments it names; the C++ call pops what its
declared convention says. Th (thiscall: callee pops its stack arguments) and St (stdcall) must pass as many dwords as the original's
`RET n` pops; Cd (cdecl) must call a function that pops nothing. A wrong count drifts the stack; two wrong ones can cancel while
nothing between them reads the stack (Game_Start 0x41a820 / 0x587380, found by tpw.exe only). A bench with the same wrong count
agrees with the port, so the benches miss it.

usage: callcheck.py   (reads _re/text.asm through tools/wave.py, src/game/**/*.cpp)
Reports file:line, the call and the original's pops; arguments of 8 bytes (double, u64) count as one here, so check those by hand.
"""
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import wave  # noqa: E402

idx = wave.index()
CALL = re.compile(r'\b(Th|St|Cd)\(\s*(0x[0-9a-fA-F]+)u?\s*[,)]')


def returns(start):
    rets = set()
    if start not in idx:   # (not an instruction start in the listing: reported by nothing here)
        return rets
    for ins in wave.body(start, wave.extent(start), idx):
        m = re.match(r'RET(?: 0x([0-9a-f]+))?$', ins)
        if m:
            rets.add(int(m.group(1), 16) if m.group(1) else 0)
    return rets


def arguments(text, at):
    """The top-level arguments of the call whose '(' is at text[at]; None when it does not close."""
    depth, args, start, i = 0, [], at + 1, at
    while i < len(text):
        c = text[i]
        if c in '"\'':
            j = i + 1
            while j < len(text) and text[j] != c:
                j += 2 if text[j] == '\\' else 1
            i = j
        elif c in '([{':
            depth += 1
        elif c in ')]}':
            depth -= 1
            if depth == 0:
                args.append(text[start:i].strip())
                return [a for a in args if a]
        elif c == ',' and depth == 1:
            args.append(text[start:i].strip())
            start = i + 1
        i += 1
    return None


def main():
    pops, bad, checked = {}, 0, 0
    for d, _, files in os.walk(os.path.join(ROOT, 'src', 'game')):
        for f in sorted(files):
            if not f.endswith('.cpp'):
                continue
            path = os.path.join(d, f)
            text = re.sub(r'//[^\n]*', lambda m: ' ' * len(m.group(0)), open(path, encoding='utf-8', errors='replace').read())
            for m in CALL.finditer(text):
                address = int(m.group(2), 16)
                if not 0x401000 <= address < 0x6fc800:
                    continue
                args = arguments(text, m.start() + len(m.group(1)))
                if args is None:
                    continue
                if address not in pops:
                    pops[address] = returns(address)
                rets = pops[address]
                if len(rets) != 1:
                    continue
                checked += 1
                popped = next(iter(rets))
                stack = len(args) - (2 if m.group(1) == 'Th' else 1)   # (the address, and Th's object)
                wanted = 0 if m.group(1) == 'Cd' else 4 * stack
                if popped != wanted:
                    bad += 1
                    line = text.count('\n', 0, m.start()) + 1
                    print('%s:%d %s(0x%x) with %d stack arguments; the original pops 0x%x' % (os.path.relpath(path, ROOT), line, m.group(1), address, stack, popped))
    print('%d of %d calls by address pop differently' % (bad, checked))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())

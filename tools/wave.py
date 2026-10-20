"""wave.py: find the next functions to port and read them.
usage:
  wave.py list [LO HI]     (env VCALLS=1 also takes functions with virtual calls) the unported functions of the range (default 401000 630000) whose callees are all hooked ("wave 0"), with their sizes; _re/wave0.json
                           and _re/r1.json (the portable closure: no x87, no indirect calls) are written. The function starts are the 16-byte aligned addresses after
                           0xCC padding in the exe, and Ghidra's starts. Env FPU=1 also takes x87 functions; SORT=size lists the biggest first.
  wave.py unlock [LO HI]   the unported callees (anywhere) that are the ONLY unported callee of unported functions in the range: each with the
                           functions and bytes its port would put in wave 0 (any x87 or virtual calls counted), the most bytes first. Port these to open the big ones.
  wave.py body ADDR...     the instructions of each function on one line (sizes from _re/r1.json, else 0x40)
  wave.py dec ADDR...      Ghidra's decompilation (_re/decomp-typed; env DECLIM = characters per function, default 6000)
  wave.py data ADDR:N...   N bytes of the exe at ADDR (jump tables)
Thunks (SUB ECX,[ECX-4]; JMP) are skipped by `list`."""
import glob
import json
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
LINES = None
ADDRS = None
TEXTS = None


def asm_lines():
    global LINES
    if LINES is None:
        LINES = open(os.path.join(ROOT, '_re', 'text.asm'), errors='replace').read().split('\n')
    return LINES


def index():
    idx = {}
    for i, line in enumerate(asm_lines()):
        try:
            idx[int(line[:8], 16)] = i
        except ValueError:
            pass
    return idx


def extent(start):
    """The size of the function at start: to the first RET / JMP that is past every forward jump target (as tools/extent.py)."""
    import bisect
    global ADDRS, TEXTS
    if ADDRS is None:
        ADDRS, TEXTS = [], []
        for line in asm_lines():
            m = re.match(r'([0-9a-f]{8})  (.*)', line)
            if m:
                ADDRS.append(int(m.group(1), 16))
                TEXTS.append(m.group(2))
    j = bisect.bisect_left(ADDRS, start)
    far = start
    while j < len(ADDRS):
        t = TEXTS[j]
        m = re.search(r'J\w+ 0x([0-9a-f]{8})', t)
        if m and not t.startswith('JMP dword'):
            target = int(m.group(1), 16)
            if far < target and target - start < 0x1000:
                far = target
        nxt = ADDRS[j + 1] if j + 1 < len(ADDRS) else ADDRS[j] + 1
        if (t.startswith('RET') or t.startswith('JMP')) and nxt > far:
            return (ADDRS[j] + (3 if t.startswith('RET') and len(t.split()) > 1 else 1) if t.startswith('RET') else min(nxt, ADDRS[j] + 6)) - start
        j += 1
    return 0x40


def body(start, size, idx=None):
    idx = idx or index()
    lines, i, out = asm_lines(), idx[start], []
    while True:
        try:
            va = int(lines[i][:8], 16)
        except ValueError:
            i += 1
            continue
        if va >= start + size:
            return out
        out.append(lines[i][8:].strip())
        i += 1


def cmd_list(lo, hi, unlock=False):
    import leafq
    import x87emu
    hooked = set(int(x, 16) for x in re.findall(r'0x00([0-9a-f]{6})', open(os.path.join(ROOT, 'src', 'hook', 'hooks.cpp')).read())) | leafq.PROVIDED
    if os.environ.get('FPU'):
        hooked |= {0x67a830, 0x67b280, 0x67b24a}  # __ftol, modf, _CIfmod: ports compute them inline and fncmp's emulator stubs them
    text, tbase = None, None
    for base, data in x87emu.exe_sections():
        if base <= 0x401000 < base + len(data):
            text, tbase = data, base
            break
    listing = set()
    for line in asm_lines():
        try:
            listing.add(int(line[:8], 16))
        except ValueError:
            pass
    starts = {va for va in listing if va % 16 == 0 and 0 < va - tbase < len(text) and text[va - tbase - 1] == 0xCC and text[va - tbase] != 0xCC}  # (aligned: x87 code ends instructions in CC bytes, FXCH ST4 = D9 CC)
    for f in glob.glob(os.path.join(ROOT, '_re', 'decomp-typed', 'decomp_*.c')):
        for m in re.finditer(r'^// ==== (\S+) @ ([0-9a-f]{8}) ====', open(f, errors='replace').read(), re.M):
            starts.add(int(m.group(2), 16))
    ks = sorted(starts)
    end = {a: ks[i + 1] for i, a in enumerate(ks[:-1])}
    info, cur, j = {}, None, 0
    for line in asm_lines():
        try:
            va = int(line[:8], 16)
        except ValueError:
            continue
        ins = line[8:].strip()
        while j < len(ks) and va >= ks[j]:
            cur, j = ks[j], j + 1
        if cur is None or not lo <= cur < hi or cur in hooked or va >= end.get(cur, cur):
            continue
        rec = info.setdefault(cur, {'fpu': 0, 'callees': set(), 'vcall': 0, 'size': end.get(cur, cur) - cur})
        m = re.match(r'(CALL|JMP) 0x([0-9a-f]+)$', ins)
        if m:
            t = int(m.group(2), 16)
            if m.group(1) == 'CALL' or not cur <= t < end.get(cur, cur):
                rec['callees'].add(t)
        elif ins.startswith('CALL'):
            rec['vcall'] += 1
        if re.match(r'F[A-Z]', ins):
            rec['fpu'] += 1
    cand = {a for a, r in info.items() if (r['fpu'] == 0 or os.environ.get('FPU')) and (r['vcall'] == 0 or os.environ.get('VCALLS'))}  # VCALLS=1: virtual and indirect calls allowed (recorder benches)
    closure, changed = set(), True
    while changed:
        changed = False
        for a in cand - closure:
            if all(t in hooked or t in closure for t in info[a]['callees']):
                closure.add(a)
                changed = True
    wave0 = sorted(a for a in closure if all(t in hooked for t in info[a]['callees']))
    for a in info:
        info[a]['size'] = extent(a)
    if unlock:
        gain = {}
        for a, r in info.items():
            missing = {t for t in r['callees'] if t not in hooked and 0x401000 <= t < 0x6e7000}
            if len(missing) == 1:
                g = gain.setdefault(missing.pop(), [0, 0])
                g[0], g[1] = g[0] + 1, g[1] + r['size']
        for t, (n, b) in sorted(gain.items(), key=lambda kv: -kv[1][1])[:40]:
            print('%x%s  unlocks %d (%d bytes)' % (t, '(%d)' % info[t]['size'] if t in info else ' (outside the range)', n, b))
        return
    json.dump({'%x' % a: [info[a]['size'], 0] for a in sorted(closure)}, open(os.path.join(ROOT, '_re', 'r1.json'), 'w'))
    json.dump(['%x' % a for a in wave0], open(os.path.join(ROOT, '_re', 'wave0.json'), 'w'))
    idx = index()
    shown = shown_bytes = 0
    for a in (sorted(wave0, key=lambda a: -info[a]['size']) if os.environ.get('SORT') == 'size' else wave0):
        b = body(a, info[a]['size'], idx)
        text_of = ' '.join(b)
        if b and (b[0].startswith('SUB ECX,dword ptr [ECX + -0x4]') or (len(b) <= 3 and b[-1].startswith('JMP')) or ('[EBP' in text_of and 'PUSH EBP' not in text_of)
                  or text_of.startswith('INC EBP') or re.search(r'CALL dword ptr \[0x006f[de]', text_of)):  # (imports: Win32 calls the driver does not serve)  # thunks, tail jumps and the pieces of exception handlers
            continue
        print('%x(%d)' % (a, info[a]['size']), end=' ')
        shown += 1
        shown_bytes += info[a]['size']
    print('\n%d unported, %d portable, %d in wave 0, %d shown (%d bytes)' % (len(info), len(closure), len(wave0), shown, shown_bytes))


def main():
    cmd, args = sys.argv[1], sys.argv[2:]
    if cmd in ('list', 'unlock'):
        cmd_list(int(args[0], 16) if args else 0x401000, int(args[1], 16) if len(args) > 1 else 0x630000, unlock=cmd == 'unlock')
    elif cmd == 'body':
        r1 = json.load(open(os.path.join(ROOT, '_re', 'r1.json'))) if os.path.exists(os.path.join(ROOT, '_re', 'r1.json')) else {}
        idx = index()
        for a in args:
            size = r1.get(a.lower().lstrip('0'), [0x40])[0]
            print('%s(%d): %s' % (a, size, ' ; '.join(body(int(a, 16), size, idx))))
    elif cmd == 'dec':
        want = {a.lower().lstrip('0') for a in args}
        limit = int(os.environ.get('DECLIM', '6000'))
        for f in sorted(glob.glob(os.path.join(ROOT, '_re', 'decomp-typed', 'decomp_*.c'))):
            for part in re.split(r'(?m)^// ==== ', open(f, errors='replace').read())[1:]:
                m = re.match(r'(\S+) @ (?:0x)?0*([0-9a-fA-F]+) ====', part)
                if m and m.group(2).lower() in want:
                    print('//==', part[:limit], '\n')
    elif cmd == 'data':
        import x87emu
        secs = x87emu.exe_sections()
        for a in args:
            va, n = a.split(':')
            va, n = int(va, 16), int(n, 0)
            for base, data in secs:
                if base <= va < base + len(data):
                    print(a, data[va - base:va - base + n].hex())
                    break
    else:
        print(__doc__)


if __name__ == '__main__':
    main()

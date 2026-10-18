"""Wires reimplemented functions into the hook table, symbols, build and tests.

usage: integrate.py SPEC [--no-build] [--bench tests/fncmp/x.py [ARGS...]]
SPEC has one function per line, "name|address|unit|description" (unit: under src/game, without
extension, e.g. coaster/spline); blank lines and lines starting with # are skipped.

For each function: adds its row to kHooks in src/hook/hooks.cpp (after the unit's last row,
with the original's first five bytes from the exe), and its row to re/symbols.csv (replacing
the row at that address if there is one, else after the unit's last row). Adds the unit's .cpp
to tpw_game in CMakeLists.txt and its header to hooks.cpp if missing. Then builds, runs
tpw_tests and, with --bench, the given fncmp suite. Functions already in kHooks are skipped,
so a rerun is harmless. Before touching anything it refuses names already used for another
address (in kHooks or re/symbols.csv) or declared in a header other than the unit's own.
"""
import csv
import io
import os
import re
import subprocess
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
EXE = os.path.join(ROOT, '.data', 'nocd', 'tp.exe')
HOOKS = os.path.join(ROOT, 'src', 'hook', 'hooks.cpp')
SYMBOLS = os.path.join(ROOT, 're', 'symbols.csv')
CMAKE = os.path.join(ROOT, 'CMakeLists.txt')
TEXT_DELTA = 0x400c00  # .text VA - file offset


def read(path):
    with open(path, newline='') as f:
        return f.read()


def write(path, text):
    with open(path, 'w', newline='') as f:
        f.write(text)


def eol(text):
    return '\r\n' if text.count('\r\n') * 2 > text.count('\n') else '\n'  # the majority


def parse_spec(path):
    out = []
    for line in read(path).splitlines():
        if not line.strip() or line.lstrip().startswith('#'):
            continue
        name, addr, unit, desc = (x.strip() for x in line.split('|', 3))
        base = unit.replace('\\', '/').rsplit('/', 1)[-1].removesuffix('.cpp')
        if re.match(r'^(small|tiny_?|misc)\d*[a-z]?\d*$', base):  # the repo names a module for its subject, not a bucket
            sys.exit('%s: unit %r is a bucket name; put %s in a module named for what it belongs to' % (path, unit, name))
        out.append((name, int(addr, 16), unit.replace('\\', '/').removesuffix('.cpp'), desc))
    return out


def unit_names(symbols_text, tag):
    """Names whose symbols.csv notes contain tag, e.g. '(src/game/coaster/spline)'."""
    return {row[1] for row in csv.reader(io.StringIO(symbols_text)) if len(row) > 4 and tag in row[4]}


def anchors(unit):
    """Where a unit's rows go: after its own, else after its folder's (a new unit)."""
    return ['(src/game/%s)' % unit, '(src/game/%s/' % unit.rsplit('/', 1)[0]]


def header_of(unit):
    """The unit's header without its extension (relative to src/game): its own, or the one its .cpp includes first (rsse_vm.cpp includes rsse.h)."""
    cpp = os.path.join(ROOT, 'src', 'game', unit + '.cpp')
    if not os.path.exists(os.path.join(ROOT, 'src', 'game', unit + '.h')) and os.path.exists(cpp):
        m = re.search(r'#include "([^"]+)\.h"', read(cpp))
        if m:
            return os.path.normpath(os.path.join(os.path.dirname(unit), m.group(1))).replace(os.sep, '/')
    return unit


def check_names(spec, symbols_text):
    """Exits, before any edit, on a name that is already something else."""
    hooks = read(HOOKS)
    bad = []
    for name, addr, unit, _ in spec:
        m = re.search(r'\{"%s",\s*0x([0-9a-f]{8}),' % re.escape(name), hooks)
        if m and int(m.group(1), 16) != addr:
            bad.append('%s is hooked at 0x%s' % (name, m.group(1)))
        for row in csv.reader(io.StringIO(symbols_text)):
            if len(row) > 1 and row[1] == name and int(row[0], 16) != addr:
                bad.append('%s is %s in re/symbols.csv' % (name, row[0]))
        own = os.path.normpath(os.path.join(ROOT, 'src', 'game', header_of(unit) + '.h'))
        decl = re.compile(r'\b%s\s*\(' % re.escape(name))
        for d, _, files in os.walk(os.path.join(ROOT, 'src', 'game')):
            for f in files:
                path = os.path.normpath(os.path.join(d, f))
                if f.endswith('.h') and path != own and decl.search(read(path)):
                    bad.append('%s is declared in %s' % (name, os.path.relpath(path, ROOT)))
    if bad:
        sys.exit('name clash, nothing changed:\n  ' + '\n  '.join(bad))


def add_hooks(spec, names_by_unit):
    text = read(HOOKS)
    nl = eol(text)
    lines = text.split(nl)
    start = next(i for i, l in enumerate(lines) if l.startswith('const Hook kHooks[]'))
    end = next(i for i in range(start, len(lines)) if lines[i] == '};')
    exe = open(EXE, 'rb').read()
    added = 0
    for name, addr, unit, _ in spec:
        if any(l.lstrip().startswith('{"%s",' % name) for l in lines[start:end]):
            print('  hooks.cpp: %s already there' % name)
            continue
        if any('0x%08x,' % addr in l for l in lines[start:end]):
            sys.exit('hooks.cpp: 0x%08x is already hooked under another name' % addr)
        b = exe[addr - TEXT_DELTA:addr - TEXT_DELTA + 5]
        row = '    {%-28s 0x%08x, (void *)&%s, {%s}},' % ('"%s",' % name, addr, name, ', '.join('0x%02x' % x for x in b))
        at = end
        for names in names_by_unit[unit]:
            for i in range(start + 1, end):
                m = re.match(r'\s*\{"(\w+)",', lines[i])
                if m and m.group(1) in names:
                    at = i + 1
            if at != end:
                break
        lines.insert(at, row)
        names_by_unit[unit][0].add(name)
        end += 1
        added += 1
    text = nl.join(lines)
    for unit in sorted({s[2] for s in spec}):
        header = header_of(unit)
        inc = '#include "../game/%s.h"' % header
        if inc not in text:
            # (the CR only looked at, not matched: an insert at a match's end must land before the line's CR, or that line gets CR CR LF)
            incs = [m for m in re.finditer(r'^#include "\.\./game/([^"]+)\.h"(?=\r?$)', text, re.M)]
            after = [m for m in incs if m.group(1).split('/')[0] == header.split('/')[0] and m.group(1) < header]
            same = [m for m in incs if m.group(1).split('/')[0] == header.split('/')[0]]
            anchor = (after or same or incs)[-1]
            pos = anchor.end() if after or not same else anchor.start()
            text = text[:pos] + (nl + inc if after or not same else inc + nl) + text[pos:]
            print('  hooks.cpp: included %s.h' % header)
    write(HOOKS, text)
    return added


def add_symbols(spec):
    text = read(SYMBOLS)
    nl = eol(text)
    lines = text.split(nl)
    for name, addr, unit, desc in spec:
        key = '0x%08x,' % addr
        hit = next((i for i, l in enumerate(lines) if l.startswith(key)), None)
        # A name is unconfirmed (low) unless its row already had it: then its confidence stays. port.py --known raises a name to high.
        old = next(csv.reader([lines[hit]])) if hit is not None else []
        confidence = old[3] if len(old) > 3 and old[1] == name else 'low'
        buf = io.StringIO()
        csv.writer(buf, lineterminator='').writerow(
            ['0x%08x' % addr, name, 'function', confidence, 'reimplemented (src/game/%s); %s' % (unit, desc)])
        row = buf.getvalue()
        if hit is not None:
            if lines[hit] != row:
                print('  symbols.csv: replaced %s' % lines[hit][:90])
            lines[hit] = row
            continue
        at = None
        for tag in anchors(unit):
            at = max((i + 1 for i, l in enumerate(lines) if tag in l), default=None)
            if at is not None:
                break
        if at is None:
            at = len(lines) - 1 if lines[-1] == '' else len(lines)
        lines.insert(at, row)
    write(SYMBOLS, nl.join(lines))


def add_sources(spec):
    text = read(CMAKE)
    nl = eol(text)
    for unit in sorted({s[2] for s in spec}):
        src = 'src/game/%s.cpp' % unit
        if not os.path.exists(os.path.join(ROOT, src)):
            sys.exit('%s does not exist' % src)
        if re.search(r'^\s*%s\s*$' % re.escape(src), text, re.M):
            continue
        folder = 'src/game/%s/' % unit.rsplit('/', 1)[0]
        lines = text.split(nl)
        game = next(i for i, l in enumerate(lines) if l.startswith('add_library(tpw_game'))
        close = next(i for i in range(game, len(lines)) if lines[i].strip() == ')')
        same = [i for i in range(game, close) if lines[i].strip().startswith(folder)]
        lines.insert(same[-1] + 1 if same else close, '    ' + src)
        text = nl.join(lines)
        print('  CMakeLists.txt: added %s' % src)
    write(CMAKE, text)


def run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, stdin=subprocess.DEVNULL, cwd=ROOT, **kw)


def main():
    a = sys.argv[1:]
    if not a:
        sys.exit(__doc__)
    spec = parse_spec(a[0])
    sym = read(SYMBOLS)
    check_names(spec, sym)
    names_by_unit = {u: [unit_names(sym, t) for t in anchors(u)] for u in {s[2] for s in spec}}
    n = add_hooks(spec, names_by_unit)
    add_symbols(spec)
    add_sources(spec)
    print('%d hook rows added' % n)
    if '--no-build' in a:
        return
    p = run(['cmd', '/c', os.path.join(ROOT, 'build.bat')])
    if p.returncode:
        errs = [l for l in (p.stdout + p.stderr).splitlines() if re.search(r'error|unresolved', l, re.I)]
        sys.exit('BUILD FAILED\n' + '\n'.join(errs[:20]))
    p = run([os.path.join(ROOT, 'build', 'tpw_tests.exe'), EXE])
    out = p.stdout + p.stderr
    matched = sum(1 for l in out.splitlines() if 'bytes match' in l.lower())
    passed = 'all tests passed' in out.lower()
    print('tests: %d hooks match, %s' % (matched, 'all passed' if passed else 'FAILED'))
    if not passed:
        print('\n'.join(l for l in out.splitlines() if re.search(r'fail|mismatch', l, re.I))[:3000])
    # Callers that use EAX after a call to a port declared void (benches can't see this).
    p = run([sys.executable, '-I', os.path.join(ROOT, 'tools', 'retcheck.py')])
    print('retcheck: ' + p.stdout.strip().splitlines()[-1])
    if p.returncode:
        print(p.stdout.strip()[-3000:])
        passed = False
    # The stack the port pops against the original's RET (benches cannot see a wrong calling convention).
    p = run([sys.executable, '-I', os.path.join(ROOT, 'tools', 'convcheck.py')])
    print('convcheck: ' + p.stdout.strip().splitlines()[-1])
    if p.returncode:
        print(p.stdout.strip()[-3000:])
        passed = False
    if '--bench' in a:
        i = a.index('--bench')
        p = run([sys.executable, '-I'] + a[i + 1:])
        print(p.stdout.strip()[-3000:] + p.stderr.strip()[-1000:])
        passed = passed and p.returncode == 0
    sys.exit(0 if passed else 1)


if __name__ == '__main__':
    main()

"""One porting cycle: integrate a spec, bench it, commit it, then run its mutants.

  python -I tools/port.py SPEC --bench SUITE.py [suite args...] [--msg FILE] [--mut FILE] [--also PATH...] [--unverified REASON] [--low NAME...]

--low NAME...: the spec's names that were invented get "low" confidence in re/symbols.csv (integrate.py writes "high"), in the same commit.

Runs tools/integrate.py SPEC --bench SUITE.py ARGS (hook rows, symbols, CMake, build, hook tests,
retcheck, bench). If all of that passes and --msg is given, commits by explicit path: each unit's
.h/.cpp from the spec, the suite, src/hook/hooks.cpp, re/symbols.csv, CMakeLists.txt and any
--also paths; the message file gets the Co-Authored-By trailer if it lacks one. Nothing else is
staged, so .data/, _re/ and build/ can't slip in.

Every function of the spec must be verified directly by the bench: MIN_CASES or more of its cases call the function's address and match
the C++ without a crash in either (fncmp's FNCMP_COVER records them), or nothing is committed. A function that cannot be compared
(imports, hardware, ...) is ported with --unverified "REASON" instead: it goes on re/unverified.csv with that reason, the commit
includes the list, and tools/coverage.py --check accepts it. The bench is also run as the coverage gate runs it (10 cases seed 5, 400
seed 6): a function the bench compares that gets no case there fails the port (deal the bench's ops: Rand.deal in tools/fncmp.py).

Mutants come after the commit (a surviving equivalent mutant shouldn't hold up a verified port).
The --mut file is Python defining MUTANTS = [(path, old, new), ...] and optionally OPS (the suite's
OPS environment filter) and N (cases per mutant, default 300).

Prints one line per stage; exits 1 if anything failed or a mutant survived.
"""
import csv
import importlib.util
import json
import os
import re
import subprocess
import sys
import tempfile
import time

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
AUTHOR = ['-c', 'user.name=Noklef', '-c', 'user.email=281545466+Noklef@users.noreply.github.com']
TRAILER = 'Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>'
FLAGS = ('--msg', '--mut', '--also', '--unverified', '--low')
MIN_CASES = 10


def take(a, flag):
    """Removes flag and the values after it (up to the next flag) from a; returns the values."""
    if flag not in a:
        return []
    i = a.index(flag)
    j = i + 1
    while j < len(a) and a[j] not in FLAGS:
        j += 1
    vals = a[i + 1:j]
    del a[i:j]
    return vals


def git(*args):
    return subprocess.run(['git'] + list(args), capture_output=True, text=True, stdin=subprocess.DEVNULL, cwd=ROOT)


def gate_cover(suite):
    """address -> direct, matching cases of the suite run as tools/coverage.py runs it (10 cases seed 5, then 400 seed 6)."""
    fd, cover = tempfile.mkstemp(suffix='.jsonl')
    os.close(fd)
    for n, seed in [(10, 5), (400, 6)]:
        subprocess.run([sys.executable, '-I', os.path.join(ROOT, suite) if not os.path.isabs(suite) else suite, str(n), str(seed)], capture_output=True,
                       stdin=subprocess.DEVNULL, cwd=ROOT, env=dict(os.environ, FNCMP_COVER=cover))
    out = {}
    for line in open(cover):
        if line.strip():
            for k, v in json.loads(line)['direct'].items():
                out[int(k, 16)] = out.get(int(k, 16), 0) + v
    os.remove(cover)
    return out


def main():
    a = sys.argv[1:]
    if len(a) < 3 or '--bench' not in a:
        sys.exit(__doc__)
    msg, mut, also, unverified = take(a, '--msg'), take(a, '--mut'), take(a, '--also'), ' '.join(take(a, '--unverified'))
    low = set(take(a, '--low'))
    spec = a[0]
    bench = a[a.index('--bench') + 1:]
    rows = []  # the spec's (name, address, unit)
    for line in open(os.path.join(ROOT, spec) if not os.path.isabs(spec) else spec):
        if line.strip() and not line.lstrip().startswith('#'):
            f = [x.strip() for x in line.split('|')]
            rows.append((f[0], int(f[1], 16), f[2].replace('\\', '/').removesuffix('.cpp')))
    if unverified and (unverified.lower().startswith('todo') or len(unverified) < 12):
        sys.exit('port: --unverified needs the reason the functions cannot be compared, not a placeholder')

    started = time.time()
    fd, cover = tempfile.mkstemp(suffix='.jsonl')
    os.close(fd)
    p = subprocess.run([sys.executable, '-I', os.path.join(ROOT, 'tools', 'integrate.py')] + a,
                       capture_output=True, text=True, stdin=subprocess.DEVNULL, cwd=ROOT, env=dict(os.environ, FNCMP_COVER=cover))
    out = (p.stdout + p.stderr).strip()
    seen = {}  # address -> the bench's cases that called it and matched without a crash
    for line in open(cover):
        if line.strip():
            for k, v in json.loads(line)['direct'].items():
                seen[int(k, 16)] = seen.get(int(k, 16), 0) + v
    os.remove(cover)
    if p.returncode:
        print(out[-2500:])
        sys.exit('port: integrate FAILED')
    keep = [l for l in out.splitlines() if l.startswith(('tests:', 'retcheck:')) or 'differ' in l or 'hook rows' in l]
    print(' | '.join(keep) + ' | %d s' % (time.time() - started))
    # A bench whose cases mostly fault in the original too tests little: say so (the commit still goes ahead, the decision is the porter's).
    for l in keep:
        m = re.match(r'(\d+) cases.*\((\d+) crash in both\)', l)
        if m and int(m.group(2)) * 10 > int(m.group(1)):
            print('WARNING: %s of %s cases crash in both: the bench barely exercises the port' % (m.group(2), m.group(1)))

    # Each function must have been compared: cases that call it and match without a crash in either.
    weak = ['%s %x (%d)' % (name, addr, seen.get(addr, 0)) for name, addr, _ in rows if seen.get(addr, 0) < MIN_CASES]
    if weak and not unverified:
        sys.exit('port: NOT VERIFIED, fewer than %d matching cases without a crash call: %s (bench them, or --unverified "REASON")'
                 % (MIN_CASES, ', '.join(weak)))
    # tools/coverage.py runs each bench at 10 cases (seed 5) and 400 (seed 6): every function this bench compares must get a case there too,
    # or ops added to a bench of many functions leave older ones without one and the gate fails after the merge (deal them: Rand.deal).
    gate = gate_cover(bench[0])
    starved = ['%x' % k for k in sorted(seen) if seen[k] >= MIN_CASES and not gate.get(k)]
    if starved:
        sys.exit('port: the coverage gate\'s cases (%s 10 5 + 400 6) call none of %s: deal or weight the bench\'s ops (r.deal in tools/fncmp.py)'
                 % (os.path.basename(bench[0]), ', '.join(starved)))

    if unverified:
        path = os.path.join(ROOT, 're', 'unverified.csv')
        with open(path, newline='') as f:
            listed = {r['address']: r for r in csv.DictReader(f)}
        for name, addr, unit in rows:
            if seen.get(addr, 0) < MIN_CASES:
                listed['%x' % addr] = {'address': '%x' % addr, 'name': name, 'module': unit, 'reason': unverified}
        with open(path, 'w', newline='') as f:
            w = csv.DictWriter(f, ['address', 'name', 'module', 'reason'], lineterminator='\n')
            w.writeheader()
            for k in sorted(listed, key=lambda k: int(k, 16)):
                w.writerow(listed[k])
        print('re/unverified.csv: %s listed as "%s"' % (', '.join(weak) or 'nothing', unverified))
        also = also + ['re/unverified.csv']

    if low:
        unknown = low - set(name for name, _, _ in rows)
        if unknown:
            sys.exit('port: --low names not in the spec: %s' % ', '.join(sorted(unknown)))
        path = os.path.join(ROOT, 're', 'symbols.csv')
        with open(path, newline='') as f:
            text = f.read()
        for name, addr, _ in rows:
            if name in low:
                key = '0x%08x,%s,function,high,' % (addr, name)
                if key not in text:
                    sys.exit('port: no symbols.csv row %s' % key)
                text = text.replace(key, '0x%08x,%s,function,low,' % (addr, name), 1)
        with open(path, 'w', newline='') as f:
            f.write(text)
        print('symbols.csv: low confidence for %s' % ', '.join(sorted(low)))

    if msg:
        units = set(unit for _, _, unit in rows)
        paths = ['src/hook/hooks.cpp', 're/symbols.csv', 'CMakeLists.txt', os.path.relpath(bench[0], ROOT)] + also
        paths += ['src/game/%s.%s' % (u, e) for u in sorted(units) for e in ('h', 'cpp')]
        paths = [x.replace('\\', '/') for x in paths if os.path.exists(os.path.join(ROOT, x))]
        bad = [x for x in paths if x.startswith(('.data', '_re', 'build'))]
        if bad:
            sys.exit('port: refusing to commit %s' % bad)
        text = open(msg[0], encoding='utf-8').read().rstrip('\n')
        if TRAILER not in text:
            text += '\n\n' + TRAILER
        text += '\n'
        tmp = msg[0] + '.port'
        open(tmp, 'w', encoding='utf-8', newline='\n').write(text)
        r = git('add', '--', *paths)
        r = r if r.returncode else git(*AUTHOR, 'commit', '-q', '-F', tmp)
        os.remove(tmp)
        if r.returncode:
            sys.exit('port: commit FAILED\n' + (r.stdout + r.stderr)[-1500:])
        print('committed ' + git('log', '-1', '--format=%h %s').stdout.strip())

    if mut:
        env = {}
        exec(open(mut[0], encoding='utf-8').read(), env)
        if 'OPS' in env:
            ops = env['OPS']   # (a string 'A,B' or a list of names)
            os.environ['OPS'] = ops if isinstance(ops, str) else ','.join(ops)
        suite = os.path.abspath(os.path.join(ROOT, bench[0]))
        sys.path.insert(0, os.path.dirname(suite))
        sys.argv = [os.path.basename(suite)]
        spec_ = importlib.util.spec_from_file_location(os.path.splitext(os.path.basename(suite))[0], suite)
        m = importlib.util.module_from_spec(spec_)
        spec_.loader.exec_module(m)
        caught = m.B.mutants(m.case, env['MUTANTS'], env.get('N', 300))
        print('mutants: %d/%d caught' % (sum(caught), len(caught)))
        if not all(caught):
            sys.exit(1)


if __name__ == '__main__':
    main()

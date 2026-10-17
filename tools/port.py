"""One porting cycle: integrate a spec, bench it, commit it, then run its mutants.

  python -I tools/port.py SPEC --bench SUITE.py [suite args...] [--msg FILE] [--mut FILE] [--also PATH...]

Runs tools/integrate.py SPEC --bench SUITE.py ARGS (hook rows, symbols, CMake, build, hook tests,
retcheck, bench). If all of that passes and --msg is given, commits by explicit path: each unit's
.h/.cpp from the spec, the suite, src/hook/hooks.cpp, re/symbols.csv, CMakeLists.txt and any
--also paths; the message file gets the Co-Authored-By trailer if it lacks one. Nothing else is
staged, so .data/, _re/ and build/ can't slip in.

Mutants come after the commit (a surviving equivalent mutant shouldn't hold up a verified port).
The --mut file is Python defining MUTANTS = [(path, old, new), ...] and optionally OPS (the suite's
OPS environment filter) and N (cases per mutant, default 300).

Prints one line per stage; exits 1 if anything failed or a mutant survived.
"""
import importlib.util
import os
import re
import subprocess
import sys
import time

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
AUTHOR = ['-c', 'user.name=Noklef', '-c', 'user.email=281545466+Noklef@users.noreply.github.com']
TRAILER = 'Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>'
FLAGS = ('--msg', '--mut', '--also')


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


def main():
    a = sys.argv[1:]
    if len(a) < 3 or '--bench' not in a:
        sys.exit(__doc__)
    msg, mut, also = take(a, '--msg'), take(a, '--mut'), take(a, '--also')
    spec = a[0]
    bench = a[a.index('--bench') + 1:]

    started = time.time()
    p = subprocess.run([sys.executable, '-I', os.path.join(ROOT, 'tools', 'integrate.py')] + a,
                       capture_output=True, text=True, stdin=subprocess.DEVNULL, cwd=ROOT)
    out = (p.stdout + p.stderr).strip()
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

    if msg:
        units = set()
        for line in open(os.path.join(ROOT, spec) if not os.path.isabs(spec) else spec):
            if line.strip() and not line.lstrip().startswith('#'):
                units.add(line.split('|')[2].strip().replace('\\', '/').removesuffix('.cpp'))
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
            os.environ['OPS'] = env['OPS']
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

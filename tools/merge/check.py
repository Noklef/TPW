"""Checks a resolved merge of a porting branch before it is committed, stopping at the first failure.

usage: python -I tools/merge/check.py BRANCH [--cases N] [--no-bench] [--no-build]

Run in the main tree after `git merge --no-ff --no-commit BRANCH` and resolving the conflicts (keepboth.py, then unionmerge.py).
In order:
  1. no conflicts left unresolved;
  2. kept: nothing the branch or main had is lost in the result. Per file: bench op names ('Name':) and hex addresses in
     tests/fncmp, hooks.cpp rows and includes, symbols.csv addresses, CMakeLists src/game entries. An item counts as lost when
     the branch dropped it from the merge base, or when either side has it and the result does not (unless main itself removed
     it); a bench range end that a widened range covers is not lost. Duplicates in the tables, CRs not ending a line in hooks.cpp, and a
     module-level name bound twice in a bench (the merge of two sides' constants, which git does not see as a conflict) also fail;
  3. hooks: rows the branch added (its own count, for checking an agent's report against);
  4. build, tpw_tests, retcheck, convcheck;
  5. each bench the merge touches, at N cases (default 1000) seed 5, --both: every run must show 0 differ.
"""
import collections
import os
import re
import subprocess
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
EXE = os.path.join(ROOT, '.data', 'nocd', 'tp.exe')
HOOKS = 'src/hook/hooks.cpp'
ROW = r'^\s+\{"\w+",\s+(0x[0-9a-f]+)'
TABLES = [(HOOKS, [ROW, r'^#include "(.*)"']), ('re/symbols.csv', [r'^(0x[0-9a-fA-F]+),']), ('CMakeLists.txt', [r'^\s+(src/game/\S+)'])]
BENCH = [r"'([A-Za-z_][A-Za-z0-9_]*)':", r'(0x[0-9a-fA-F]{5,8})']


def run(cmd):
    return subprocess.run(cmd, capture_output=True, text=True, stdin=subprocess.DEVNULL, cwd=ROOT)


def git(*a):
    return run(['git'] + list(a)).stdout


def show(ref, path):
    return git('show', '%s:%s' % (ref, path))


def worktree(path):
    p = os.path.join(ROOT, path)
    return open(p, encoding='utf-8', errors='replace').read() if os.path.exists(p) else ''


def fail(msg):
    sys.exit('check: FAILED: ' + msg)


def items(text, pat):
    found = re.findall(pat, text, re.M)
    return {int(x, 16) if x.startswith('0x') else x for x in found}, collections.Counter(found)


def ranges(text):
    return [(int(a, 16), int(b, 16)) for a, b in re.findall(r'\((0x[0-9a-fA-F]+),\s*(0x[0-9a-fA-F]+)\)', text)]


def top_names(text):
    """Module-level names a bench binds more than once (assignments, defs): one side's constant silently redefined by the other's."""
    import ast
    c = collections.Counter()
    try:
        body = ast.parse(text).body
    except SyntaxError:
        return set()
    for n in body:
        if isinstance(n, ast.Assign):
            for tg in n.targets:
                c.update(e.id for e in (tg.elts if isinstance(tg, ast.Tuple) else [tg]) if isinstance(e, ast.Name))
        elif isinstance(n, (ast.FunctionDef, ast.ClassDef)):
            c[n.name] += 1
    return {k for k, v in c.items() if v > 1}


def kept(path, pats, base, branch, table):
    problems = []
    texts = [show(r, path) for r in (base, 'HEAD', branch)]
    result = worktree(path)
    bench = path.startswith('tests/')
    if bench:
        twice = top_names(result) - top_names(texts[1]) - top_names(texts[2])
        if twice:
            problems.append('%s binds %s twice at module level (one side redefines the other\'s)' % (path, sorted(twice)))
    # A bench range's end that a widened range now covers is not lost (a branch that grows (lo, x) to (lo, y) drops x as text only).
    bounds = {x for t in texts for r in ranges(t) for x in r} if bench else set()
    spans = ranges(result) if bench else []
    for pat in pats:
        b, h, t = (items(x, pat)[0] for x in texts)
        res, counts = items(result, pat)
        dropped = (b - t) - res if bench else b - t
        lost = (h | t) - res - (b - h)
        bad = sorted((x for x in dropped | lost if not (x in bounds and any(lo <= x <= hi for lo, hi in spans))), key=str)
        if bad:
            problems.append('%s lost %s' % (path, ', '.join('%x' % x if isinstance(x, int) else x for x in bad[:8])))
        dups = [k for k, v in counts.items() if v > 1] if table else []
        if dups:
            problems.append('%s duplicates %s' % (path, dups[:8]))
    return problems


def main():
    a = sys.argv[1:]
    if not a or a[0].startswith('-'):
        sys.exit(__doc__)
    sys.stdout.reconfigure(line_buffering=True)  # (progress shows in a redirected log as it goes)
    branch = a[0]
    cases = a[a.index('--cases') + 1] if '--cases' in a else '1000'
    if not git('rev-parse', '-q', '--verify', 'MERGE_HEAD').strip():
        fail('no merge in progress (run git merge --no-ff --no-commit %s first)' % branch)
    base = git('merge-base', 'HEAD', branch).strip()

    unresolved = git('diff', '--name-only', '--diff-filter=U').split()
    marked = [f for f in git('diff', '--name-only', 'HEAD').split() if re.search(r'^<{7} |^>{7} ', worktree(f), re.M)]
    if unresolved or marked:
        fail('unresolved: %s' % sorted(set(unresolved + marked)))
    print('conflicts: none left')

    problems = []
    benches = sorted({f for f in git('diff', '--name-only', base, branch, '--', 'tests/fncmp/').split() + git('diff', '--name-only', 'HEAD', '--', 'tests/fncmp/').split()
                      if f.endswith('.py')})
    for f in benches:
        problems += kept(f, BENCH, base, branch, False)
    for f, pats in TABLES:
        problems += kept(f, pats, base, branch, True)
    lone = len(re.findall(rb'\r(?!\n)', open(os.path.join(ROOT, HOOKS), 'rb').read()))
    if lone:
        problems.append('%s has %d CRs not ending a line (CR CR LF, or a line split or joined)' % (HOOKS, lone))
    # The index stores text with LF only (.gitattributes); a CR there is a CR CR LF a branch committed, which the checkout shows as a
    # plain CR LF line, so the working file cannot show it.
    stored = subprocess.run(['git', 'show', ':' + HOOKS], capture_output=True, stdin=subprocess.DEVNULL, cwd=ROOT).stdout.count(b'\r')
    if stored:
        problems.append('%s has %d CRs in the index (stored lines must end in LF; git add the file to renormalise it)' % (HOOKS, stored))
    if problems:
        fail('kept:\n  ' + '\n  '.join(problems))
    print('kept: %d benches, 3 tables, nothing lost' % len(benches))

    rows = lambda text: set(re.findall(ROW, text, re.M))
    added = rows(show(branch, HOOKS)) - rows(show(base, HOOKS))
    print('hooks: branch added %d rows, %d in the result' % (len(added), len(rows(worktree(HOOKS)))))

    if '--no-build' in a:
        print('build, tests, benches: skipped (--no-build)')
        return
    p = run(['cmd', '/c', os.path.join(ROOT, 'build.bat')])
    if p.returncode:
        fail('build\n' + '\n'.join([l for l in (p.stdout + p.stderr).splitlines() if re.search(r'error|unresolved', l, re.I)][:20]))
    out = (lambda p: p.stdout + p.stderr)(run([os.path.join(ROOT, 'build', 'tpw_tests.exe'), EXE]))
    if 'all tests passed' not in out.lower():
        fail('tpw_tests\n' + '\n'.join(l for l in out.splitlines() if re.search(r'fail|mismatch', l, re.I))[:3000])
    for tool in ('retcheck', 'convcheck'):
        p = run([sys.executable, '-I', os.path.join(ROOT, 'tools', tool + '.py')])
        if p.returncode:
            fail(tool + '\n' + p.stdout.strip()[-3000:])
    print('build, tpw_tests, retcheck, convcheck: passed')

    if '--no-bench' in a:
        print('benches: skipped (--no-bench)')
        return
    for f in benches:
        if not os.path.exists(os.path.join(ROOT, f)):
            continue
        p = run([sys.executable, '-I', f, cases, '5', '--both'])
        res = [l.strip() for l in p.stdout.splitlines() if 'differ' in l]
        name = os.path.basename(f)[:-3]
        if p.returncode or not res or any(not re.search(r'\b0 differ', l) for l in res):
            fail('bench %s\n%s' % (name, (p.stdout + p.stderr).strip()[-3000:]))
        print('bench %-24s %s' % (name, ' | '.join(res)))
    print('check: all passed')


if __name__ == '__main__':
    main()

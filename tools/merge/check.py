"""Checks a resolved merge of a porting branch before it is committed, stopping at the first failure.

usage: python -I tools/merge/check.py BRANCH [--cases N] [--no-bench] [--no-build]

Run in the main tree after `git merge --no-ff --no-commit BRANCH` and resolving the conflicts (keepboth.py, then unionmerge.py).
In order:
  1. no conflicts left unresolved;
  2. kept: nothing the branch or main had is lost in the result. Per file: bench op names ('Name':) and hex addresses in
     tests/fncmp, hooks.cpp rows and includes, symbols.csv addresses, CMakeLists src/game entries. An item counts as lost when
     the branch dropped it from the merge base, or when either side has it and the result does not (unless main itself removed
     it); a bench range end that a widened range covers is not lost. Duplicates in the tables, CRs not ending a line in hooks.cpp, a
     module-level name bound twice in a bench (the merge of two sides' constants, which git does not see as a conflict) and an op name
     twice in a bench's table (a dict literal's key, or KINDS['x'] = for a key it has: the first op's cases are never made) also fail.
     --moved BENCH (repeatable): a bench the branch empties by moving its cases to other benches (a bucket clean-up); its op names
     are not checked, and an address it lost is kept when another bench has it (or, for a range's end, the range's start).
     --accept-lost PATH:ITEM (repeatable): one lost item checked by hand; printed, and to be named in the merge commit;
  3. hooks: rows the branch added (its own count, for checking an agent's report against);
  4. build, tpw_tests, retcheck, convcheck;
  5. each bench the merge touches, at N cases (default 1000) seed 5, --both: every run must show 0 differ. Every bench importing a
     touched module runs too (a bench's case() reused by another, or a helper module with no main, e.g. thingenv.py, which needs one).
"""
import collections
import glob
import os
import re
import subprocess
import sys
import time

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
EXE = os.path.join(ROOT, '.data', 'nocd', 'tp.exe')
HOOKS = 'src/hook/hooks.cpp'
ROW = r'^\s+\{"\w+",\s+(0x[0-9a-f]+)'
TABLES = [(HOOKS, [ROW, r'^#include "(.*)"']), ('re/symbols.csv', [r'^(0x[0-9a-fA-F]+),']), ('CMakeLists.txt', [r'^\s+(src/game/\S+)'])]
# Op names: dict keys and `op == 'x':`, and each name in `op in ('x', 'y'):` (a branch that folds one op's case into a shared one).
BENCH = [r"'([A-Za-z_][A-Za-z0-9_]*)'(?=(?:,\s*'[A-Za-z_][A-Za-z0-9_]*')*\)?:)", r'(0x[0-9a-fA-F]{5,8})']


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


def doubled_keys(text):
    """Op names a bench's table holds twice: a key repeated in a dict literal, or NAME['k'] = ... at module level for a key NAME already
    has. Python keeps the last silently, so the first op's cases are never made (two sides, or one branch, reusing a name)."""
    import ast
    try:
        tree = ast.parse(text)
    except SyntaxError:
        return []
    out, tables = [], {}
    for n in ast.walk(tree):
        if isinstance(n, ast.Dict):
            seen = set()
            for k in n.keys:
                if isinstance(k, ast.Constant):
                    if k.value in seen:
                        out.append('%r (line %d)' % (k.value, k.lineno))
                    seen.add(k.value)
    for n in tree.body:
        if isinstance(n, ast.Assign) and len(n.targets) == 1:
            tg = n.targets[0]
            if isinstance(tg, ast.Name) and isinstance(n.value, ast.Dict):
                tables[tg.id] = {k.value for k in n.value.keys if isinstance(k, ast.Constant)}
            elif isinstance(tg, ast.Subscript) and isinstance(tg.value, ast.Name) and isinstance(tg.slice, ast.Constant) and tg.value.id in tables:
                if tg.slice.value in tables[tg.value.id]:
                    out.append('%s[%r] (line %d)' % (tg.value.id, tg.slice.value, n.lineno))
                tables[tg.value.id].add(tg.slice.value)
    return out


def elsewhere(path):
    """Every hex address the working tree's other benches mention."""
    out = set()
    for f in glob.glob(os.path.join(ROOT, 'tests', 'fncmp', '*.py')):
        if os.path.normcase(os.path.relpath(f, ROOT)) != os.path.normcase(path):
            out |= items(open(f, encoding='utf-8', errors='replace').read(), BENCH[1])[0]
    return out


def kept(path, pats, base, branch, table, moved=False):
    problems = []
    texts = [show(r, path) for r in (base, 'HEAD', branch)]
    result = worktree(path)
    bench = path.startswith('tests/')
    if bench:
        twice = top_names(result) - top_names(texts[1]) - top_names(texts[2])
        if twice:
            problems.append('%s binds %s twice at module level (one side redefines the other\'s)' % (path, sorted(twice)))
        doubled = doubled_keys(result)
        if doubled:
            problems.append('%s has op names twice in a table (the last wins, the first op never runs): %s' % (path, ', '.join(doubled[:8])))
    # A bench range's end that a widened range now covers is not lost (a branch that grows (lo, x) to (lo, y) drops x as text only).
    bounds = {x for t in texts for r in ranges(t) for x in r} if bench else set()
    spans = ranges(result) if bench else []
    # A bench emptied by moves (--moved): its cases went to the destination modules' benches, under new op names, so its op names are
    # not checked, and an address it lost counts as kept when another bench has it (or, for a range's end, the range's start).
    there = elsewhere(path) if moved else set()
    ends = {hi: lo for t in texts for lo, hi in ranges(t)}
    for pat in pats:
        if moved and pat == BENCH[0]:
            continue
        b, h, t = (items(x, pat)[0] for x in texts)
        res, counts = items(result, pat)
        dropped = (b - t) - res if bench else b - t
        lost = (h | t) - res - (b - h)
        bad = sorted((x for x in dropped | lost if not (x in bounds and any(lo <= x <= hi for lo, hi in spans))
                      and not (moved and (x in there or ends.get(x) in there))), key=str)
        if bad:
            problems.append('%s lost %s' % (path, ', '.join('%x' % x if isinstance(x, int) else x for x in bad[:8])))
        dups = [k for k, v in counts.items() if v > 1] if table else []
        if dups:
            problems.append('%s duplicates %s' % (path, dups[:8]))
    return problems


def main():
    sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    import lowprio
    lowprio.below_normal()   # (the builds and benches it starts inherit it)
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
    moved = {m.replace('\\', '/') for i, m in enumerate(a) if i and a[i - 1] == '--moved'}
    for f in benches:
        problems += kept(f, BENCH, base, branch, False, f in moved)
    # --accept-lost PATH:ITEM: one item the merger checked by hand (say, a moved case's region now starting a few bytes later); printed,
    # and to be named in the merge commit.
    accepted = [m for i, m in enumerate(a) if i and a[i - 1] == '--accept-lost']
    for acc in accepted:
        path, item = acc.split(':', 1)
        hit = [p for p in problems if p.startswith(path + ' lost ') and item in p.split(' lost ', 1)[1].split(', ')]
        if not hit:
            fail('--accept-lost %s: nothing like that was lost' % acc)
        for p in hit:
            rest = [x for x in p.split(' lost ', 1)[1].split(', ') if x != item]
            problems[problems.index(p)] = '%s lost %s' % (path, ', '.join(rest)) if rest else None
        problems = [p for p in problems if p]
        print('kept: ACCEPTED as lost by hand: %s' % acc)
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
    # A changed module also runs as every bench that imports it (a bench's case() reused as another's world, or a helper module with
    # no main of its own, e.g. thingenv.py, which must have one), and so on for theirs.
    runs, todo = [], [f for f in benches if os.path.exists(os.path.join(ROOT, f))]
    while todo:
        f = todo.pop()
        if f in runs:
            continue
        name = os.path.basename(f)[:-3]
        users = sorted(os.path.relpath(u, ROOT).replace('\\', '/') for u in glob.glob(os.path.join(ROOT, 'tests', 'fncmp', '*.py'))
                       if re.search(r'^\s*(from %s import|import %s\b)' % (name, name), open(u, encoding='utf-8', errors='replace').read(), re.M))
        if '.main(' in worktree(f):
            runs.append(f)
            if users:
                print('bench %s: also through %s' % (name, ', '.join(os.path.basename(u)[:-3] for u in users)))
        else:
            print('helper %s: through %s' % (name, ', '.join(os.path.basename(u)[:-3] for u in users) or 'no bench (FAILED)'))
            if not users:
                fail('helper %s has no bench importing it' % name)
            runs.append(f)   # (seen; not run itself)
        todo += users
    runs = [f for f in runs if '.main(' in worktree(f)]
    for f in sorted(set(runs)):
        t0 = time.time()
        p = run([sys.executable, '-I', f, cases, '5', '--both'])
        res = [l.strip() for l in p.stdout.splitlines() if 'differ' in l]
        name = os.path.basename(f)[:-3]
        if p.returncode or not res or any(not re.search(r'\b0 differ', l) for l in res):
            fail('bench %s\n%s' % (name, (p.stdout + p.stderr).strip()[-3000:]))
        print('bench %-24s %s  [%d s]' % (name, ' | '.join(res), time.time() - t0))   # (the time: a slow bench is a merge's cost)
    print('check: all passed')


if __name__ == '__main__':
    main()

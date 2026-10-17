"""Which hooks a bench actually verifies: runs every tests/fncmp suite with fncmp's FNCMP_COVER and sorts the hooks of src/hook/hooks.cpp into
  direct    some case called the hook's address itself and matched the C++ without a crash in either (the guarantee a port needs),
  indirect  the original only ran inside other functions' matching cases (weaker: the C++ caller may not call the port),
  none      no matching case reached it.

  python -I tools/coverage.py [--cases N] [--budget S] [--jobs J] [--only a.py,b.py] [--check] [--write-baseline]

Each suite runs 10 cases first, then up to --cases (default 400, seed 6) as its --budget seconds (default 90) allow from that timing.
The result goes to _re/coverage.json, a summary by module to stdout.

re/unverified.csv lists the hooks allowed to lack direct coverage, each with a reason (todo: no bench yet; or why it cannot be compared).
--check fails when a hook lacks direct coverage and is not listed (a hook added without a bench), and names the listed ones that are now
covered (to take off the list). --write-baseline rewrites the list from this run, keeping the reasons already given.
"""
import concurrent.futures
import csv
import json
import os
import re
import subprocess
import sys
import tempfile
import time

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from fncmp import hooked_addresses  # noqa: E402

SUITES = os.path.join(ROOT, 'tests', 'fncmp')
BASELINE = os.path.join(ROOT, 're', 'unverified.csv')
OUT = os.path.join(ROOT, '_re', 'coverage.json')


def arg(a, flag, default):
    return a[a.index(flag) + 1] if flag in a else default


def suites(only):
    names = sorted(f for f in os.listdir(SUITES) if f.endswith('.py') and '.main(' in open(os.path.join(SUITES, f)).read())
    return [f for f in names if f in only] if only else names


def run(name, cases, budget):
    """Runs one suite with coverage on; returns (name, its JSON lines, seconds, an error or None)."""
    fd, path = tempfile.mkstemp(suffix='.jsonl')
    os.close(fd)
    env = dict(os.environ, FNCMP_COVER=path)
    started, err = time.time(), None
    try:
        for n, seed in [(10, 5), (None, 6)]:
            if n is None:
                per_case = (time.time() - started) / 10
                n = max(0, min(cases, int(budget / max(per_case, 1e-3)) - 10))
                if n == 0:
                    break
            p = subprocess.run([sys.executable, '-I', os.path.join(SUITES, name), str(n), str(seed)], env=env, capture_output=True, text=True,
                               stdin=subprocess.DEVNULL, cwd=ROOT, timeout=max(600, budget * 4))
            if p.returncode and 'differ' not in p.stdout:
                err = (p.stdout + p.stderr).strip().splitlines()[-1:] or ['exit %d' % p.returncode]
                err = err[0][:200]
                break
        lines = [json.loads(line) for line in open(path) if line.strip()]
    except subprocess.TimeoutExpired:
        lines, err = [json.loads(line) for line in open(path) if line.strip()], 'timed out'
    finally:
        os.remove(path)
    return name, lines, time.time() - started, err


def modules():
    """address -> the module re/symbols.csv names (or the symbol's name)."""
    out = {}
    for row in open(os.path.join(ROOT, 're', 'symbols.csv'), errors='replace'):
        p = row.split(',', 4)
        if p[0].startswith('0x'):
            m = re.search(r'src/game/([a-z0-9_/]+)', p[4] if len(p) > 4 else '')
            out[int(p[0], 16)] = (p[1], m.group(1) if m else '?')
    return out


def read_baseline():
    if not os.path.exists(BASELINE):
        return {}
    with open(BASELINE, newline='') as f:
        return {int(r['address'], 16): r for r in csv.DictReader(f)}


def write_baseline(rows):
    with open(BASELINE, 'w', newline='') as f:
        w = csv.DictWriter(f, ['address', 'name', 'module', 'reason'], lineterminator='\n')
        w.writeheader()
        for a in sorted(rows):
            w.writerow(rows[a])


def main():
    a = sys.argv[1:]
    cases, budget, jobs = int(arg(a, '--cases', 400)), float(arg(a, '--budget', 90)), int(arg(a, '--jobs', 2))   # (each job is a driver process: kept low by default, see --jobs)
    only = [x for x in arg(a, '--only', '').split(',') if x]
    hooks, names = hooked_addresses(), modules()
    direct, indirect, by_suite, failed = {}, set(), {}, []
    started = time.time()
    with concurrent.futures.ThreadPoolExecutor(jobs) as pool:
        for name, lines, secs, err in pool.map(lambda s: run(s, cases, budget), suites(only)):
            if err:
                failed.append('%s: %s' % (name, err))
            mine = set()
            for line in lines:
                if line['differ']:
                    failed.append('%s: %d differ' % (name, line['differ']))
                for k, v in line['direct'].items():
                    direct[int(k, 16)] = direct.get(int(k, 16), 0) + v
                    mine.add(int(k, 16))
                indirect |= set(int(k, 16) for k in line['indirect'])
            by_suite[name] = {'seconds': round(secs, 1), 'direct': sorted('%x' % k for k in mine)}
    covered = set(k for k in direct if k in hooks)
    only_indirect = (indirect & hooks) - covered
    none = hooks - covered - only_indirect
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    json.dump({'direct': {'%x' % k: direct[k] for k in sorted(covered)}, 'indirect': sorted('%x' % k for k in only_indirect),
               'none': sorted('%x' % k for k in none), 'suites': by_suite, 'failed': failed}, open(OUT, 'w'), indent=1)

    print('%d hooks: %d direct, %d indirect only, %d none (%d suites, %.0f s)' % (len(hooks), len(covered), len(only_indirect), len(none), len(by_suite),
                                                                                time.time() - started))
    for f in failed:
        print('suite FAILED', f)
    tally = {}
    for k in only_indirect | none:
        tally.setdefault(names.get(k, ('?', '?'))[1], [0, 0])[k in none] += 1
    for mod, (ind, no) in sorted(tally.items(), key=lambda t: -sum(t[1]))[:30]:
        print('  %-24s %3d indirect only, %3d none' % (mod, ind, no))

    if only:
        return 1 if failed else 0
    base = read_baseline()
    missing = sorted(k for k in hooks - covered if k not in base)
    done = sorted(k for k in base if k in covered)
    if '--write-baseline' in a:
        rows = {}
        for k in hooks - covered:
            old = base.get(k)
            name, mod = names.get(k, ('?', '?'))
            rows[k] = {'address': '%x' % k, 'name': name, 'module': mod, 'reason': old['reason'] if old else ('todo: indirect only' if k in only_indirect else 'todo')}
        write_baseline(rows)
        print('wrote %s: %d rows' % (os.path.relpath(BASELINE, ROOT), len(rows)))
    elif '--check' in a:
        for k in missing:
            print('NOT VERIFIED %x %s (%s): no direct coverage and not in re/unverified.csv' % (k, *names.get(k, ('?', '?'))))
        for k in done:
            print('now covered: %x %s, take it off re/unverified.csv (--write-baseline)' % (k, base[k]['name']))
        return 1 if missing or failed else 0
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())

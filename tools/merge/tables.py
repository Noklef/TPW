# Resolves the merge's conflicts in the shared tables in one go, after `git merge --no-ff --no-commit`: re/symbols.csv, src/hook/hooks.cpp
# and CMakeLists.txt. Each hunk becomes ours' lines, then theirs' lines whose key ours lacks (the key: symbols.csv a row's address, hooks.cpp
# a kHooks row's name and address, any other line itself). A key on both sides with different lines: symbols.csv keeps the row that says
# "reimplemented" (ours when both or neither do, listed for a look); hooks.cpp and CMakeLists.txt leave that hunk for a hand. Each file left
# without markers is `git add`ed; then the tables' duplicate addresses and names are listed. Other conflicted files are named, by hand.
# usage: python -I tools/merge/tables.py   (from anywhere in the tree being merged; it works from that tree's top)
import os
import re
import subprocess
import sys

TABLES = ('re/symbols.csv', 'src/hook/hooks.cpp', 'CMakeLists.txt')
ROW = re.compile(rb'\s*\{"([^"]+)",\s*(0x[0-9a-fA-F]+)')


def keys(path, line):
    s = line.strip()
    if path.endswith('.csv'):
        return {s.split(b',', 1)[0].lower()}
    m = ROW.match(line) if path.endswith('hooks.cpp') else None
    return {b'name:' + m.group(1), b'addr:' + m.group(2).lower()} if m else {s}


def resolve(path):
    lines = open(path, 'rb').read().split(b'\n')   # (LF only: a lone CR stays inside its line)
    lines = [l + b'\n' for l in lines[:-1]] + ([lines[-1]] if lines[-1] else [])
    out, left, review, i, hunks = [], 0, [], 0, 0
    while i < len(lines):
        if not lines[i].startswith(b'<<<<<<< '):
            out.append(lines[i])
            i += 1
            continue
        j = next(k for k in range(i, len(lines)) if lines[k].rstrip(b'\r\n') == b'=======')
        k = next(k for k in range(j, len(lines)) if lines[k].startswith(b'>>>>>>> '))
        ours, theirs = [l for l in lines[i + 1:j] if l.strip()], [l for l in lines[j + 1:k] if l.strip()]
        owned = {}
        for n, l in enumerate(ours):
            for key in keys(path, l):
                owned[key] = n
        merged, clash = list(ours), False
        for l in theirs:
            hit = {owned[key] for key in keys(path, l) if key in owned}
            if not hit:
                merged.append(l)
            elif any(ours[n].strip() != l.strip() for n in hit):
                if path.endswith('.csv') and len(hit) == 1:
                    n = hit.pop()
                    if b'reimplemented' in l and b'reimplemented' not in ours[n]:
                        merged[n] = l
                    elif (b'reimplemented' in l) == (b'reimplemented' in ours[n]):
                        review.append(ours[n].split(b',', 2)[:2])
                else:
                    clash = True
        if clash:
            out += lines[i:k + 1]
            left += 1
        else:
            out += merged
            hunks += 1
        i = k + 1
    open(path, 'wb').write(b''.join(out))
    print('%s: %d hunks resolved%s' % (path, hunks, ', %d left (a key on both sides with different lines)' % left if left else ''))
    for addr, name in review:
        print('  kept ours (both or neither reimplemented): %s %s' % (addr.decode(), name.decode()))
    return not left


def duplicates():
    rows = open('re/symbols.csv', 'rb').read().split(b'\n')[1:]
    seen, dup = set(), set()
    for r in rows:
        a = r.split(b',', 1)[0].strip().lower()
        if a:
            (dup if a in seen else seen).add(a)
    for a in sorted(dup):
        print('re/symbols.csv: address %s twice' % a.decode())
    names, addrs = {}, {}
    for m in ROW.finditer(open('src/hook/hooks.cpp', 'rb').read()):
        names[m.group(1)] = names.get(m.group(1), 0) + 1
        addrs[m.group(2).lower()] = addrs.get(m.group(2).lower(), 0) + 1
    for d, what in ((names, 'name'), (addrs, 'address')):
        for k in sorted(k for k, n in d.items() if n > 1):
            print('src/hook/hooks.cpp: %s %s twice' % (what, k.decode()))
    return not dup and all(n == 1 for d in (names, addrs) for n in d.values())


os.chdir(subprocess.run(['git', 'rev-parse', '--show-toplevel'], capture_output=True, text=True, check=True).stdout.strip())
conflicted = subprocess.run(['git', 'diff', '--name-only', '--diff-filter=U'], capture_output=True, text=True, check=True).stdout.split()
done = [p for p in conflicted if p in TABLES and resolve(p)]
if done:
    subprocess.run(['git', 'add', '--'] + done, check=True)
rest = [p for p in conflicted if p not in done]
for p in rest:
    print('by hand: %s' % p)
ok = duplicates()
sys.exit(0 if ok and not rest else 1)

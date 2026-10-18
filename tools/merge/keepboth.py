# Resolves add/add conflicts in table files by keeping both sides (ours, then theirs). Refuses a hunk whose sides share a key (the
# first comma/quote field), which would be a real conflict. usage: python -I tools/merge/keepboth.py FILE...
import re
import sys

for path in sys.argv[1:]:
    data = open(path, 'rb').read().decode('utf-8')
    out, i, hunks = [], 0, 0
    pat = re.compile(r'^<<<<<<< [^\n]*\r?\n(.*?)^=======\r?\n(.*?)^>>>>>>> [^\n]*\r?\n', re.S | re.M)
    for m in pat.finditer(data):
        ours, theirs = m.group(1), m.group(2)
        key = lambda l: re.split(r'[,"]', l.strip().lstrip('{').strip())[0:2]
        ka = {tuple(key(l)) for l in ours.splitlines() if l.strip()}
        kb = {tuple(key(l)) for l in theirs.splitlines() if l.strip()}
        if ka & kb:
            sys.exit('%s: both sides have %s' % (path, ka & kb))
        out.append(data[i:m.start()])
        out.append(ours + theirs)
        i, hunks = m.end(), hunks + 1
    out.append(data[i:])
    open(path, 'wb').write(''.join(out).encode('utf-8'))
    print(path, hunks, 'hunks kept both')

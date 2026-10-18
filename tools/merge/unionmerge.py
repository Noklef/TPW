# Resolves conflict hunks as a union: ours' lines, then theirs' lines not already in ours (compared without line endings or edge
# whitespace). Works on bytes so mixed CR LF / LF endings survive. usage: python -I tools/merge/unionmerge.py FILE...
import sys

for path in sys.argv[1:]:
    # split on LF only: bytes.splitlines() also splits at a lone CR, which cut hooks.cpp's CR CR LF lines in two (and the empty half
    # was then dropped as a duplicate, joining two lines)
    lines = open(path, 'rb').read().split(b'\n')
    lines = [l + b'\n' for l in lines[:-1]] + ([lines[-1]] if lines[-1] else [])
    out, i, hunks = [], 0, 0
    while i < len(lines):
        if not lines[i].startswith(b'<<<<<<< '):
            out.append(lines[i])
            i += 1
            continue
        j = next(k for k in range(i, len(lines)) if lines[k].rstrip(b'\r\n') == b'=======')
        k = next(k for k in range(j, len(lines)) if lines[k].startswith(b'>>>>>>> '))
        ours, theirs = lines[i + 1:j], lines[j + 1:k]
        seen = {l.strip() for l in ours}
        out += ours + [l for l in theirs if not l.strip() or l.strip() not in seen]
        hunks += 1
        i = k + 1
    open(path, 'wb').write(b''.join(out))
    print(path, hunks, 'hunks unioned')

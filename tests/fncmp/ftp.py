# The FTP client's listing line reader (src/game/online/ftp: 0x5d2720) against the original: "ls -l" lines of 0-7 fields (perms, links,
# owner, group, size, ...) split by runs of spaces, tabs and the other isspace chars, fields of letters, digits, signs and high bytes
# (sign-extended for isspace), the size as atoi takes it; a short line's NUL is read past, into junk that has a space somewhere.
# usage: ftp.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x5d2720, 0x5d2842), (0x67a400, 0x6902c0)])
LINE = RAM + 0x100
SPACES = [b' ', b' ', b'  ', b'\t', b' \t ', b'\r\n', b'\x0b\x0c']
WORD = b'rwx-dlAZaz019+.\x80\xa0\xff'


def field(r, size=False):
    if size:
        return r.choice([b'', b'-', b'+']) + str(r.choice([0, 7, r.getrandbits(16), r.getrandbits(32), r.getrandbits(36)])).encode() + \
            r.choice([b'', b'', b'k', b'.5'])
    return bytes(r.choice(WORD) for _ in range(r.choice([1, 1, 2, 4, 10])))


def case(r):
    n = r.choice([5, 5, 5, 6, 7, 4, 3, 1, 0])
    parts = [field(r, i == 4) for i in range(n)]
    line = r.choice([b'', b'', b' ']) + b''.join(p + r.choice(SPACES) for p in parts)
    if r.random() < 0.5:
        line = line.rstrip()
    junk = bytes(r.choice(WORD + b'   ') for _ in range(r.randrange(40))) + b' 12 x 34 y 56 z 78 w 9 '
    return Call(0x5d2720, [r.getrandbits(32), LINE], {LINE: line + b'\0' + junk + b'\0'}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

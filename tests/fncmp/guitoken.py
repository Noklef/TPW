# The GUI library's text tokens (src/game/ui/guitoken: 0x66dbfa, 0x66dc2c, 0x66dcf7, 0x66dd68, 0x66ddd8) against the originals, with
# the game's CRT (strchr, strcmp, atoi, strncpy) emulated on one side and tpw_fncmp's stand-ins on the other. The text is a few tokens
# (numbers with or without signs, words, odd bytes) split by spaces and tabs, leading blanks now and then, sometimes a single token or
# none, its NUL then random bytes. The out buffer is random bytes, or none now and then. ReadString's count is 0-0x80 mostly (a negative word rarely:
# strncpy's zeros run past the buffer, a fault in both). ReadKind: the first token mostly one of the game's kind names (as it is, in
# lower case, cut short or run on), the names' count [0x77c59c] the game's 11 now and then fewer (or a large word: the table's end read
# on, a fault in both).
# usage: guitoken.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SkipBlanks,ReadKind,ReadWord,ReadDword,ReadString)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'SkipBlanks': 0x66dbfa, 'ReadKind': 0x66dc2c, 'ReadWord': 0x66dcf7, 'ReadDword': 0x66dd68, 'ReadString': 0x66ddd8}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x66dbfa, 0x66de4c), (0x67a400, 0x6902c0)])
KINDS = [b'WINDOW', b'BUTTON', b'SCROLL', b'LISTBOX', b'EDITBOX', b'IMAGE', b'LISTCONTROL', b'CHECKBUTTONS', b'BAR', b'TEXTBOX', b'KNOB']
TEXT, OUT = RAM, RAM + 0x200


def token(r):
    kind = r.random()
    if kind < 0.5:
        digits = str(r.choice([r.randrange(0, 100), r.randrange(0, 70000), r.getrandbits(32), r.getrandbits(40)]))
        return (r.choice(['', '', '-', '+']) + digits).encode()
    if kind < 0.8:
        return bytes(r.choice(b'abcxyzABCXYZ_.,0123456789') for _ in range(r.randrange(1, 12)))
    return bytes(r.choice([0x80, 0xa0, 0xff, 0x09, 0x0a, r.randrange(1, 256)]) for _ in range(r.randrange(1, 6)))


def kind(r):
    name = r.choice(KINDS)
    return r.choice([name] * 4 + [name.lower(), name[:-1], name + b'S', name + b'\t1', name[:1]])


def text(r, kinds=False):
    lead = bytes(r.choice(b'  \t') for _ in range(r.choice([0, 0, 0, 1, 2, 5]))) if not kinds or r.random() < 0.1 else b''
    parts = [token(r) for _ in range(r.choice([0, 1, 1, 2, 3, 5]))]
    if kinds and r.random() < 0.85:
        parts = [kind(r)] + parts
    body = b''
    for i, p in enumerate(parts):
        body += p
        if i + 1 < len(parts):
            body += bytes(r.choice(b'  \t') for _ in range(r.choice([1, 1, 1, 2])))
    body = lead + body + (b' ' if r.random() < 0.1 else b'')
    return body[:0x180] + b'\0' + r.bytes(0x200 - len(body[:0x180]) - 1)


def case(r):
    op = r.deal(OPS)
    regions = {TEXT: text(r, op == 'ReadKind'), OUT: r.bytes(0x100)}
    if op == 'ReadKind' and r.random() < 0.3:
        regions[0x77c59c] = r.choice([0, 1, 5, 10, r.randrange(0, 11), r.randrange(0, 11), 0x8000, 0x7fff]).to_bytes(2, 'little')
    if op == 'SkipBlanks':
        return Call(CALLS[op], [TEXT + r.choice([0, 0, 0, 1, 2])], regions, ret='a')
    out = OUT if r.random() < 0.85 else 0
    if op == 'ReadString':
        count = r.choice([r.randrange(0, 0x40)] * 6 + [0, 1, 0x80, r.randrange(0, 0x81) | r.getrandbits(16) << 16])
        if r.random() < 0.02:
            count = r.choice([0xffff, 0x8000])
        return Call(CALLS[op], [TEXT, out, count], regions, ret='a')
    return Call(CALLS[op], [TEXT, out], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

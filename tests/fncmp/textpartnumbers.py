# The number text parts' SetLayouts (src/game/render/textparts: slot 3 of the vtables 0x6ff3b0, 0x6ff3e0, 0x6ff410, 0x6ff440 and 0x6ff4a0;
# 0x4903e0, 0x490650, 0x490940, 0x490b80, 0x490ff0) against the originals. rendererpart.py's TextLayout world (the part's recording vtable,
# its info and record); mostly the flag 0x80 or'd in with the info's dword +0 a number about each one's bounds (0, 0x18 / 0x30, 0xeeeeeee,
# negative, the extremes, any), else as TextLayout (0x48fc40, which they call). The formatter 0x6acd60 the log stub with its number
# argument hashed (text_args(); its buffer an out argument) and the 8 hex digits it writes what slot 9 (the recorder 9) is sent: hashed
# (blob_bytes(), its first character alone for an empty one).
# usage: textpartnumbers.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=L3b0,L3e0,L410,L440,L4a0)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7cc5f4  # noqa: E402
import rendererpart  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _cdecl_log4, blob_bytes, put, strings, text_args  # noqa: E402

CALLS = {'L3b0': 0x4903e0, 'L3e0': 0x490650, 'L410': 0x490940, 'L440': 0x490b80, 'L4a0': 0x490ff0}
B = Bench(dialog7cc5f4.merged(rendererpart.RANGES + [(0x4903e0, 0x4904b9), (0x490650, 0x4907ac), (0x490940, 0x4909ef), (0x490b80, 0x490c23),
                                                     (0x490ff0, 0x491166)]), hooks={0x6acd60: _cdecl_log4})
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
INFO = RAM + 0x500   # (rendererpart.layout()'s)


def value(r):
    return r.choice([0, 0, 1, -1, 0x18, 0x19, 0x30, 0x31, 47, 48, 49, 72, 0xeeeeeee, 0xeeeeeef, -0xeeeeeee, -0xeeeeeef, 0x7fffffff, -0x7fffffff,
                     -0x80000000, r.randrange(-100, 101), r.randrange(-100000, 100001), r.getrandbits(32) - (1 << 31),
                     0xeeeeeee + r.randrange(-3, 4), -0xeeeeeee + r.randrange(-3, 4)])


def case(r):
    op = r.deal(OPS)
    c = rendererpart.layout(r, True)
    part, flags, info = c.args
    if r.random() < 0.8:
        n = value(r)
        put(c.regions[INFO], 0, 'i', n)
        flags, info = flags | 0x80, INFO
        log = c.regions[LOG]
        text_args(log, 1)
        strings(log, 0x3d, 0, 2)
        blob_bytes(log, 9, 0, 3 if op == 'L410' and not n else 0x3ffff)
    else:
        flags &= ~0x80
    return Call(CALLS[op], [part, flags, info], c.regions, conv='t', ret='v', cw=c.cw)


if __name__ == '__main__':
    B.main(case)

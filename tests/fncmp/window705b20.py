# The GUI library's 0x705b20 window (src/game/ui/window705b20: 0x66b47d) against the original. SetRange: any window bytes, the range
# +0x138 / +0x13c now and then the one given, the step +0x144 0, 1, -1, the sign bit or any; from / to about each other (equal, one
# apart, the sign bit) or any.
# usage: window705b20.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetRange)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

CALLS = {'SetRange': 0x66b47d}
B = Bench([(0x66b47d, 0x66b508)])


def case(r):
    r.deal(OPS)
    w = bytearray(r.bytes(0x160))
    a = r.choice([0, 1, 0x3ff, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])
    b = r.choice([a, (a + 1) & 0xffffffff, (a - 1) & 0xffffffff, 0, 0x3ff, 0x80000000, 0x7fffffff, r.getrandbits(32)])
    if r.random() < 0.5:
        a, b = b, a
    put(w, 0x144, 'I', r.choice([0, 1, 2, 0xffffffff, 0xfffffffe, 0x80000000, 0x7fffffff, r.getrandbits(32)]))
    if r.random() < 0.25:
        put(w, 0x138, 'II', b, a)
    elif r.random() < 0.2:
        put(w, 0x138, 'I', b)
    elif r.random() < 0.2:
        put(w, 0x13c, 'I', a)
    return Call(CALLS['SetRange'], [RAM, a, b], {RAM: w}, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

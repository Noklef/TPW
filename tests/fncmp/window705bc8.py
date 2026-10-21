# The GUI library's kind 11 window (vtable 0x705bc8, two ranges) against the originals (src/game/ui/window705bc8: 0x66cc90, 0x66cd05).
# FirstToSecond / SecondToFirst: the ranges' ends small or any (the second's mostly 0 to 1 or small), the spans their differences (now and
# then any; never 0, nor -1 under a dividend of -0x80000000: a fault in both), the value +0x144 / +0x158 an end of its range, in it or any.
# usage: window705bc8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FirstToSecond,SecondToFirst)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

CALLS = {'FirstToSecond': 0x66cc90, 'SecondToFirst': 0x66cd05}
B = Bench([(0x66cc90, 0x66cd05), (0x66cd05, 0x66cd83)])
WINDOW = RAM


def s32(v):
    return (v + 0x80000000 & 0xffffffff) - 0x80000000


def rng(r, big):
    low = r.choice([0, 0, 1, -5, r.randrange(-1000, 1000), s32(r.getrandbits(32))])
    high = r.choice([low + big, low + r.randrange(1, 50), low + 1, s32(r.getrandbits(32))])
    span = r.choice([high - low] * 6 + [r.randrange(1, 100), -3, s32(r.getrandbits(32))])
    return s32(low), s32(high), s32(span) or 1


def case(r):
    op = r.deal(OPS)
    w = bytearray(r.bytes(0x160))
    lo1, hi1, span1 = rng(r, 0xfff)
    lo2, hi2, span2 = rng(r, 1)
    value = r.choice([lo1, hi1, lo1 + r.randrange(0x1000), s32(r.getrandbits(32))]) if op == 'FirstToSecond' else         r.choice([lo2, hi2, lo2 + r.randrange(-2, 50), s32(r.getrandbits(32))])
    put(w, 0x138, 'iii', lo1, hi1, span1)
    put(w, 0x14c, 'iii', lo2, hi2, span2)
    put(w, 0x144 if op == 'FirstToSecond' else 0x158, 'i', s32(value))
    if op == 'FirstToSecond':   # (the product wraps in both; only its quotient by -1 overflows)
        product = s32(span2 * s32(value))
        if span1 == -1 and product == -0x80000000:
            put(w, 0x140, 'i', 1)
    else:
        product = s32(s32(s32(value) - lo2) * span1)
        if span2 == -1 and product == -0x80000000:
            put(w, 0x154, 'i', 1)
    return Call(CALLS[op], [WINDOW], {WINDOW: w}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

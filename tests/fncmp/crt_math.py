# The C library's numeric routines (src/game/core/crt) against the originals: _isnan's bits around the exponent and mantissa edges, __alldiv's signs, widths and the quotient
# estimate's correction, a zero divisor faulting in both.
# usage: crt_math.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x67c7f0, 0x67c824), (0x67b410, 0x67b440), (0x67c320, 0x67c3cd)])
U = 1 << 64


def isnan(r):
    high = r.choice([0x7ff00000, 0xfff00000, 0x7ff80000, 0xfff80000, 0x7ff7ffff, 0x7ff00001, 0x7fefffff, 0x3ff00000, 0, r.getrandbits(32)])
    low = r.choice([0, 0, 1, 0x80000000, r.getrandbits(32)])
    return Call(0x67b410, [low, high], {}, conv='c', ret='a')


def wide(r):
    bits = r.choice([1, 8, 16, 31, 32, 33, 40, 48, 63, 64])
    v = r.getrandbits(bits)
    if r.random() < 0.15:
        v = r.choice([0, 1, 0xffffffff, 0x100000000, 1 << 63, U - 1, (1 << 63) - 1])
    return (-v if r.random() < 0.4 else v) % U


def alldiv(r):
    a, b = wide(r), wide(r)
    if not b and r.random() < 0.9:
        b = 1
    return Call(0x67c320, [a & 0xffffffff, a >> 32, b & 0xffffffff, b >> 32], {}, conv='s', ret='i')


def allmul(r):
    a, b = wide(r), wide(r)
    if r.random() < 0.3:
        a &= 0xffffffff
        b &= 0xffffffff
    return Call(0x67c7f0, [a & 0xffffffff, a >> 32, b & 0xffffffff, b >> 32], {}, conv='s', ret='i')


KINDS = {'isnan': isnan, 'alldiv': alldiv, 'allmul': allmul}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    return KINDS[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)

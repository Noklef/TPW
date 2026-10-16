# The text formatter's argument constructors (src/game/core/textarg: 0x4a77a0, 0x4ea220, 0x50aef0)
# against the originals: values about 0 and INT_MIN, signed or not.
# usage: textarg.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x4a77a0, 0x4a77b9), (0x4ea220, 0x4ea240), (0x50aef0, 0x50af32)])


def case(r):
    arg = RAM + r.randrange(16)
    regions = {RAM: r.bytes(0x40)}
    n = r.choice([0, 1, 2, 4, r.getrandbits(32)])
    op = r.randrange(3)
    if op == 0:
        return Call(0x4a77a0, [arg, n], regions, conv='t', ret='a')
    if op == 1:
        return Call(0x4ea220, [arg, n, r.getrandbits(32)], regions, conv='t', ret='a')
    value = r.choice([0, 1, 0xffffffff, 0x80000000, 0x7fffffff, r.getrandbits(32)])
    return Call(0x50aef0, [arg, n, value, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

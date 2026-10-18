# The directory entry tests (src/game/files/direntry) against the original: 0x5c5230 on entries with
# the directory attribute 0x10 set or not (alone or with others) and names starting with '.', a letter
# or empty.
# usage: direntry.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5c5230, 0x5c524b)])


def case(r):
    obj = bytearray(r.bytes(0x400))
    put(obj, 0x20, 'I', RAM + 0x100)
    obj[4] = r.choice([0, 0x10, 0x10, 0x30, 0x20])
    obj[0x100] = r.choice([0x2e, 0x2e, 0x41, 0])
    return Call(0x5c5230, [RAM], {RAM: obj}, ret='a')


if __name__ == '__main__':
    B.main(case)

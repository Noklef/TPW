# The language object's font set and id (src/game/core/language: 0x41a5a0, 0x41a820-0x41a850)
# against the originals: widths about the 512 / 640 / 800 steps and negative, ids Hebrew or not.
# usage: language.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x41a5a0, 0x41a5df), (0x41a820, 0x41a85f)])
LANG = RAM + 0x100


def case(r):
    mem = put(r.bytes(0x400), 0x108, 'I', r.choice([0x40d, 0x409, 0x40d ^ (1 << r.randrange(32)), r.getrandbits(32)]))
    op = r.randrange(5)
    if op == 0:
        w = r.choice([0x200, 0x280, 0x320]) + r.randrange(-2, 3)
        w = r.choice([w, r.getrandbits(32), -w & 0xffffffff])
        return Call(0x41a5a0, [LANG, w], {RAM: mem}, conv='t')
    if op == 3:
        return Call(0x41a840, [LANG, r.getrandbits(32)], {RAM: mem}, conv='t')
    return Call({1: 0x41a820, 2: 0x41a830, 4: 0x41a850}[op], [LANG], {RAM: mem}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

# The language object's font set and id (src/game/core/language: 0x41a5a0, 0x41a820-0x41a850),
# its font lookup and Hebrew alignment (0x485a70, 0x4925a0, on the object at 0x78b530) against
# the originals: widths about the 512 / 640 / 800 steps and negative, ids Hebrew or not, font
# indexes about 13 and font sets beyond 3, alignments 0-3 and wild.
# usage: language.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x41a5a0, 0x41a5df), (0x41a820, 0x41a85f), (0x485a70, 0x485a94), (0x4925a0, 0x4925cb)])
LANG = RAM + 0x100


def case(r):
    mem = put(r.bytes(0x400), 0x108, 'I', r.choice([0x40d, 0x409, 0x40d ^ (1 << r.randrange(32)), r.getrandbits(32)]))
    op = r.randrange(7)
    if op >= 5:
        put(mem, 4, 'I', r.choice([0, 1, 2, 3, r.randrange(100), r.getrandbits(32)]))
        put(mem, 8, 'I', r.choice([0x40d, 0x40d, 0x409, 0x40d ^ (1 << r.randrange(32))]))
        if op == 5:
            return Call(0x485a70, [r.choice([0, 1, 12, 13, 14, r.randrange(13), r.getrandbits(32)])], {0x78b530: mem}, ret='a')
        put(mem, 0x200, 'I', r.choice([0, 1, 2, 3, 2, 0, r.getrandbits(32)]))
        return Call(0x4925a0, [RAM + 0x200], {0x78b530: mem[:0x10], RAM: mem})
    if op == 0:
        w = r.choice([0x200, 0x280, 0x320]) + r.randrange(-2, 3)
        w = r.choice([w, r.getrandbits(32), -w & 0xffffffff])
        return Call(0x41a5a0, [LANG, w], {RAM: mem}, conv='t')
    if op == 3:
        return Call(0x41a840, [LANG, r.getrandbits(32)], {RAM: mem}, conv='t')
    return Call({1: 0x41a820, 2: 0x41a830, 4: 0x41a850}[op], [LANG], {RAM: mem}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

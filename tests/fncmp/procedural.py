# The procedural terrain texture's column extents (src/game/terrain/procedural: 0x56e780) against the original: low and high bounds about the
# signed byte range and past it, a column range of 1 to 6 (now and then empty), the stored extents (+0x20 / +0x24) around the new ones.
# usage: procedural.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x56e780, 0x56e7de)])


def case(r):
    obj = bytearray(r.bytes(0x400))
    arg = lambda: r.choice([0, 1, 5, 9, -1, -5, 100, 0x7f, 0x80, -0x80, 0x1ff, -0x1ff, r.randrange(-300, 300)]) & 0xffffffff
    first = r.randrange(0, 10)
    last = first + r.randrange(0, 6) - (2 if r.random() < 0.1 else 0)
    put(obj, 0x20, 'i', first + r.choice([-1, 0, 1, r.randrange(-20, 20)]))  # the stored extents about the new ones
    put(obj, 0x24, 'i', last + r.choice([-1, 0, 1, r.randrange(-20, 20)]))
    return Call(0x56e780, [RAM, arg(), first, arg(), last], {RAM: obj}, conv='t')


if __name__ == '__main__':
    B.main(case)

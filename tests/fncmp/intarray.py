# The dword array SetSize of src/game/core/intarray (0x598630) against the original: arrays without data, with data of every size / max combination, new sizes 0 to 40
# (below, inside and past the max), grow steps of 0, -1, small and large.
# usage: intarray.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x598630, 0x598770), (0x67acb0, 0x67acb1)])


def case(r):
    size = r.choice([0, 1, 3, 4, 8, 12, 20])
    mx = size + r.choice([0, 0, 1, 4, 10])
    arr = bytearray(r.bytes(0x10))
    has = r.random() < 0.85
    put(arr, 0, 'III', HEAP + 0x40 if has else 0, size, mx)
    put(arr, 0xc, 'I', r.choice([0, 0, 1, 4, 10, 0x7fffffff & r.getrandbits(8)]))
    new = r.choice([0, 1, 2, size, size + 1, mx, mx + 1, mx + 5, r.randrange(41), 40])
    grow = r.choice([-1, -1, 0, 3, 16, 0x1000])
    regions = {RAM: arr, HEAP: heap()}
    return Call(0x598630, [RAM, new, grow & 0xffffffff], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

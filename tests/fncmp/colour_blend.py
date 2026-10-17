# The three colour blend of src/game/core/colour_blend (0x5f09a0) against the original: positions below 0 and above 100 and every value between, records of random bytes.
# usage: colour_blend.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x5f09a0, 0x5f0ae0)])


def case(r):
    regions = {RAM: r.bytes(0x40), RAM + 0x100: r.bytes(0x40), RAM + 0x200: r.bytes(0x40), RAM + 0x300: r.bytes(0x40)}
    position = r.choice([-5, 0, 1, 25, 49, 50, 51, 75, 99, 100, 101, 1000, r.randrange(-10, 120), -0x80000000])
    return Call(0x5f09a0, [RAM, position & 0xffffffff, RAM + 0x100, RAM + 0x200, RAM + 0x300], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

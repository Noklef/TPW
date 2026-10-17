# The navigation block's constructor of src/game/world/nav (0x50ffe0 Nav_Ctor) against the original: an object of random bytes (the constructor sets the block and its five
# integer pairs).
# usage: navctor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x50ffe0, 0x5100f0), (0x50f1f0, 0x50f200), (0x5d1750, 0x5d1751), (0x67acf0, 0x67ada0), (0x67b050, 0x67b0d0)])


def case(r):
    return Call(0x50ffe0, [RAM], {RAM: r.bytes(0x100)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

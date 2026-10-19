# The game's sine table build (src/game/math/sintable: 0x4708d0, n) against the original, at the precision the game runs it at shutdown (24 bits, the
# emulator's): n from 0 up, mostly small, now and then the game's 4096 or past the table's 4096 entries (the values run over the count at 0x7b4968).
# usage: sintable.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x4708d0, 0x470937)])
SCALE, VALUES, COUNT = 0x7b095c, 0x7b0968, 0x7b4968


def case(r):
    k = r.random()
    n = r.randrange(0, 40) if k < 0.6 else r.randrange(40, 600) if k < 0.92 else r.choice([4095, 4096, 4096, 4097, 4100])
    if VALUES + 4 * n <= COUNT:
        regions = {SCALE: r.bytes(0xc + 4 * n), COUNT: r.bytes(4)}
    else:
        regions = {SCALE: r.bytes(0xc + 4 * n)}
    return Call(0x4708d0, [n], regions)


if __name__ == '__main__':
    B.main(case)

# The IME support's constructor (src/game/core/ime: 0x478b60) against the original, on random
# bytes at addresses with different top bytes (the allocator bytes copy it, sic).
# usage: ime.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x478b60, 0x478bc0)])


def case(r):
    at = r.choice([RAM, RAM + 0x1000 * r.randrange(16) + 4 * r.randrange(16), 0x7a0000 + 0x100 * r.randrange(8)])
    return Call(0x478b60, [at], {at: r.bytes(0x70)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

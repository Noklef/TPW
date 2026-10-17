# Format_BuildChannelTable (0x591270, src/game/gfx/color) against the original: the table of 256 records of four channel values for four masks (the usual 565, 555, 4444,
# 8888 and BGR masks, empty masks, masks with gaps, wider than 8 bits, a random word), the rest of the by-value format (a DDPIXELFORMAT: size, flags, FourCC, bit count, then red, green, blue, alpha) random.
# usage: channel_table.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x591270, 0x591400)])
SETS = [(0xf800, 0x7e0, 0x1f, 0), (0x7c00, 0x3e0, 0x1f, 0x8000), (0xf00, 0xf0, 0xf, 0xf000), (0xff0000, 0xff00, 0xff, 0xff000000), (0xff, 0xff00, 0xff0000, 0xff000000),
        (0, 0, 0, 0), (0xffffffff, 1, 0x80000000, 0x55555555)]


def mask(r):
    return r.choice([0, 1, 0xff, 0x1ff, 0xffffffff, r.getrandbits(32), r.getrandbits(32) & r.getrandbits(32), (0xff << r.randrange(25)) & 0xffffffff, 0x3 << r.randrange(31)])


def case(r):
    masks = list(r.choice(SETS)) if r.random() < 0.6 else [mask(r) for _ in range(4)]
    args = [RAM] + [r.getrandbits(32) for _ in range(3)] + masks + [r.getrandbits(32)]
    return Call(0x591270, args, {RAM: bytes(0x1000)}, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)

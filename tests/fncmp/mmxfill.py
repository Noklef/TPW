# The screen buffer's fill (src/game/gfx/mmxfill: 0x678d30, MMX in the original) against the original: widths -3 to 40 (at least one
# 16-byte store a row), 1-6 rows, pitches of the width in pixels times 2 or more (or less: rows overlapping), the pattern at 0x77e580 any
# 8 bytes.
# usage: mmxfill.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x678d30, 0x678d66)])
BUF = RAM + 0x100


def case(r):
    width = r.choice([r.randrange(-3, 41), 8, 16, 1, 9])
    rows = r.randrange(1, 7)
    pitch = r.choice([2 * max(width, 0), 2 * max(width, 0) + 16, r.randrange(0, 0x60)])
    regions = {BUF: r.bytes(0x60 * 7 + 0x60), 0x77e580: r.bytes(8)}
    return Call(0x678d30, [width & 0xffffffff, rows, BUF, pitch], regions)


if __name__ == '__main__':
    B.main(case)

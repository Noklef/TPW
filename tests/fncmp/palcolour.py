# PalColour_Set (src/game/gfx/palette: 0x47ddc0) against the original: colours in, near and off
# a random game palette (0xfa71f8), with junk in the arguments' high bits; and the colour ramps set from it (0x409db0) over junk.
# usage: palcolour.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x47ddc0, 0x47ddf2), (0x5f73a0, 0x5f76b7), (0x409db0, 0x409f56)])


def case(r):
    pal = r.bytes(0x400)
    if r.deal(['set', 'ramps']) == 'ramps':
        return Call(0x409db0, [], {0x7b0480: r.bytes(0x118), 0xfa71f8: pal}, conv='c')
    k = r.randrange(256)
    rgb = [pal[k * 4 + i] for i in range(3)] if r.random() < 0.5 else [r.getrandbits(8) for _ in range(3)]
    rgb = [(v + r.choice([0, 0, 1, -1])) & 0xff | (r.getrandbits(24) << 8 if r.random() < 0.3 else 0) for v in rgb]
    return Call(0x47ddc0, [RAM + 0x10] + rgb, {RAM: r.bytes(0x100), 0xfa71f8: pal}, conv='t')


if __name__ == '__main__':
    B.main(case)

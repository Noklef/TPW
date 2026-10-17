# The panel reset of src/game/core/panel (0x5d48d0) against the original: a panel of seven images (small sources of up to 4 x 3 pixels with alphas around the ends, and the
# target image large enough for the positions the blends use, so the case is heavy: use few cases).
# usage: panel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5d48d0, 0x5d4970), (0x5d4170, 0x5d4270)])
PANEL, PIX = RAM, RAM + 0x1000
W, H = 0x228, 0xe0


def case(r):
    panel = bytearray(0x80)
    regions = {PANEL: panel}
    at = PIX
    for k in range(7):
        base = 0x10 * k
        if k == 5:  # the target image
            w, h = W, H
        else:
            w, h = r.randrange(0, 5), r.randrange(0, 4)
        put(panel, base, 'III', w, h, 4)
        put(panel, base + 0xc, 'I', at)
        size = w * h * 4 + 8
        pixels = bytearray(r.bytes(size)) if k != 5 else bytearray(r.bytes(W * H * 4 + 8))
        if k != 5:
            for i in range(0, len(pixels) - 4, 4):
                pixels[i] = r.choice([0, 1, 0x7f, 0x80, 0xfe, 0xff, pixels[i]])
        regions[at] = pixels
        at = (at + len(pixels) + 0xfff) & ~0xfff
    return Call(0x5d48d0, [PANEL], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

# The palette helpers (src/game/gfx/palette) against the originals: PalColour_Set (0x47ddc0) with colours in, near and off a random
# game palette (0xfa71f8), with junk in the arguments' high bits; the colour ramps set from it (0x409db0) over junk; Palette_Nearest
# (0x5f73a0) called directly, over a random palette or one of ties (entries at the colour plus deltas of equal squares, of equal
# sums, duplicated: each tie-break in turn); Palette_ExpandRgb (0x5f7360) over 0 to 0x100 triples.
# usage: palcolour.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Set,Ramps,Nearest,Expand)
import itertools
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x47ddc0, 0x47ddf2), (0x5f7360, 0x5f76b7), (0x409db0, 0x409f56)])
DELTAS = sorted({p for a in itertools.product(range(4), repeat=3) for p in itertools.permutations(a)})


def ties(r, rgb):
    """A palette whose entries sit at rgb plus deltas of a few sizes, signs flipped, some entries repeated."""
    sizes = [r.choice(DELTAS) for _ in range(r.choice([1, 2, 3]))]
    pal = bytearray()
    for _ in range(256):
        d = r.choice(sizes)
        pal += bytes([min(255, max(0, c + v * r.choice([1, -1]))) for c, v in zip(rgb, r.sample(d, 3))]) + r.bytes(1)
    for _ in range(r.randrange(4) if r.random() < 0.2 else 0):   # (an exact match now and then)
        k = r.randrange(256)
        pal[4 * k:4 * k + 3] = bytes(rgb)
    return bytes(pal)


def case(r):
    op = r.deal(OPS)
    pal = r.bytes(0x400)
    if op == 'Expand':
        n = r.choice([0, 1, 2, 3, r.randrange(0x101)])
        return Call(0x5f7360, [RAM + 0x400, RAM, n], {RAM: r.bytes(0x300), RAM + 0x400: r.bytes(0x404)}, conv='c', ret='v')
    if op == 'Nearest':
        rgb = [r.getrandbits(8) for _ in range(3)]
        if r.random() < 0.6:
            pal = ties(r, rgb)
        args = [v | (r.getrandbits(24) << 8 if r.random() < 0.3 else 0) for v in rgb]
        return Call(0x5f73a0, [RAM] + args, {RAM: pal}, conv='c', ret='b')
    if op == 'Ramps':
        return Call(0x409db0, [], {0x7b0480: r.bytes(0x118), 0xfa71f8: pal}, conv='c')
    k = r.randrange(256)
    rgb = [pal[k * 4 + i] for i in range(3)] if r.random() < 0.5 else [r.getrandbits(8) for _ in range(3)]
    rgb = [(v + r.choice([0, 0, 1, -1])) & 0xff | (r.getrandbits(24) << 8 if r.random() < 0.3 else 0) for v in rgb]
    return Call(0x47ddc0, [RAM + 0x10] + rgb, {RAM: r.bytes(0x100), 0xfa71f8: pal}, conv='t')


OPS = [k for k in os.environ.get('OPS', 'Set,Ramps,Nearest,Expand').split(',') if k]


if __name__ == '__main__':
    B.main(case)

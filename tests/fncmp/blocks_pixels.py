# The seven-array constructor and the image blend of src/game/core/blocks_pixels against the originals: images of a few pixels blended into a larger one at any
# position inside it, with any alpha, channels and bytes per pixel.
# usage: blocks_pixels.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Blend)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5d4170, 0x5d4260), (0x5d45b0, 0x5d46d0)])
SRC, DEST, SPIX, DPIX = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x2000


def case(r):
    if r.random() < 0.2:
        return Call(0x5d45b0, [RAM], {RAM: r.bytes(0x100)}, conv='t', ret='a')
    sw, sh = r.randrange(0, 5), r.randrange(0, 4)
    dw, dh, bpp = r.randrange(max(sw, 1), 9), 8, r.choice([4, 4, 4, 3, 5])
    x, y = r.randrange(0, dw - sw + 1) if dw >= sw else 0, r.randrange(0, 4)
    src, dest = bytearray(0x20), bytearray(0x20)
    put(src, 0, 'III', sw, sh, 4)
    put(src, 0xc, 'I', SPIX)
    put(dest, 0, 'III', dw, dh, bpp)
    put(dest, 0xc, 'I', DPIX)
    pixels = bytearray(r.bytes(4 * 4 * 4 + 8))
    for i in range(0, len(pixels), 4):  # alphas around the ends
        pixels[i] = r.choice([0, 1, 0x7f, 0x80, 0xfe, 0xff, pixels[i]])
    regions = {SRC: src, DEST: dest, SPIX: pixels, DPIX: r.bytes(0x400)}
    return Call(0x5d4170, [SRC, DEST, x, y], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

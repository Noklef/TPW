# The matrix that puts a sprite on the screen (src/game/sprite/screenmatrix: 0x476c50) against
# the original: sizes and positions of any 16 bits (high words garbage), spans as floats, the
# aspect globals 0x74f888 / 0x74f88c, angles 0 or not, through Matrix_Identity / Matrix_Mul and
# a 4096-entry sine table.
# usage: screenmatrix.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb  # noqa: E402

B = Bench([(0x476c50, 0x476e80), (0x46f150, 0x46f180), (0x4700f0, 0x4702d0)], sines=4096)


def word(r, small):
    v = r.choice([r.randrange(-small, small + 1), r.randrange(1, small + 1), r.getrandbits(16), 0])
    return (v & 0xffff) | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0)


def case(r):
    aspect = r.floats(bytearray(8), range(0, 8, 4), r.choice([1, 2, None]))
    if r.random() < 0.5:
        aspect[0:8] = fb(1.0).to_bytes(4, 'little') + fb(r.choice([1.0, 0.75, 1.3333334])).to_bytes(4, 'little')
    angle = r.choice([0, 0, r.randrange(-4096, 4097), r.getrandbits(16), 1024, 2048])
    args = [RAM, word(r, 2048), word(r, 1536), word(r, 640), word(r, 480),
            r.f(1000), r.f(1000), r.f(2048), r.f(1536), word(r, 2048), word(r, 1536),
            (angle & 0xffff) | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)]
    return Call(0x476c50, args, {RAM: bytearray(r.bytes(0x40)), 0x74f888: aspect})


if __name__ == '__main__':
    B.main(case)

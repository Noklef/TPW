# An animated object put down on the grid (src/game/motion/placement: 0x467030) against the original: an animator, its holder, model and transform
# block of random content; grid sizes and origins of random floats; angles right and odd, cells and footprints small and wide; flags 2 and 0x20
# each set or not, with random other bits.
# usage: placement.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x467030, 0x4671f0), (0x46f650, 0x46f700)], sines=256)
ANIM, HOLDER, MODEL, XF, WORLD, GRID = (RAM + 0x1000 * i for i in range(6))


def small(r):
    return r.choice([0, 1, 2, 3, r.randrange(-5, 300), r.getrandbits(32) - (1 << 31)])


def case(r):
    anim = put(r.bytes(0xc8), 8, 'I', HOLDER)
    holder = put(r.bytes(0x14), 4, 'I', MODEL)
    put(holder, 0x10, 'i', r.choice([0, 90, 359, r.randrange(-720, 720), r.getrandbits(32) - (1 << 31)]))
    model = put(r.bytes(0xac), 0x78, 'I', XF)
    grid = r.floats(r.bytes(0x18), range(8, 0x18, 4))
    angle = r.choice([0, 0xb4, 90, 270, -90, r.randrange(-1000, 1000), r.getrandbits(32) - (1 << 31)])
    flags = r.choice([0, 2, 0x20, 0x22]) | (r.getrandbits(8) & ~0x22 if r.random() < 0.5 else 0)
    if r.random() < 0.2:
        flags |= r.getrandbits(32) & ~0xff
    args = [ANIM, small(r), small(r), small(r), small(r), angle, r.f(), flags, small(r), small(r)]
    regs = {ANIM: anim, HOLDER: holder, MODEL: model, XF: r.bytes(0x50), WORLD: put(r.bytes(0x70), 0x6c, 'I', GRID), GRID: grid,
            0x7a0854: put(bytearray(4), 0, 'I', WORLD)}
    return Call(0x467030, args, regs)


if __name__ == '__main__':
    B.main(case)

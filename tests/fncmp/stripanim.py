# The strip animation (src/game/model/stripanim: 0x4543c0) against the original: an object
# (+4 flags, +0xb0 -> count, +0xb4 -> shape) with grouped vertices and texture coordinates, t
# across the ramp (and wild), counts from none up.
# usage: stripanim.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402

B = Bench([(0x4543c0, 0x45453b)])
OBJECT, SHAPE, CHAIN, VERTS, COORDS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x1000, RAM + 0x2000


def case(r):
    ram = bytearray(r.bytes(0x3000))
    n = r.choice([0, 1, 2, 3, 8, r.randrange(-3, 40)])
    put(ram, 4, 'I', r.getrandbits(32) | r.choice([2, 2, 2, 0]) if r.random() < 0.9 else r.getrandbits(32) & ~2)
    put(ram, 0xb0, 'I', CHAIN + 0x80)
    put(ram, 0x280, 'i', n)
    put(ram, 0xb4, 'I', SHAPE)
    put(ram, 0x110, 'I', VERTS)
    put(ram, 0x124, 'I', CHAIN)          # holder: +4 model
    put(ram, 0x204, 'I', CHAIN + 0x20)   # model: +0x78 root
    put(ram, 0x298, 'I', CHAIN + 0x40)
    put(ram, 0x2a8, 'I', COORDS)         # root: +0x68 coordinates
    r.floats(ram, range(VERTS - RAM, VERTS - RAM + 0x30 * 40, 4), r.choice([1, 10, 100, None]))
    t = r.choice([r.f(2), r.f(), fb(r.uniform(0, 1.25)), fb(0.25), fb(1.0), fb(0.0), r.f()])
    return Call(0x4543c0, [OBJECT, t], {RAM: ram})


if __name__ == '__main__':
    B.main(case)

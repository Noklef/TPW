# The screen surfaces source (src/game/gfx/screensurfaces: 0x47e2f0, 0x47e390, 0x47e570) against
# the original: the constructor (through SurfaceInfo_Ctor) and the base destructor on random
# bytes, and the bounds from any screen size.
# usage: screensurfaces.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x47e2f0, 0x47e385), (0x47e390, 0x47e397), (0x47e570, 0x47e59e), (0x5fa680, 0x5fa697)])
CALLS = ('Ctor', 'Dtor', 'Bounds')


def case(r):
    op = r.choice(OPS)
    regs = {RAM: r.bytes(0xc0)}
    if op == 'Ctor':
        return Call(0x47e2f0, [RAM], regs, conv='t', ret='a')
    if op == 'Dtor':
        return Call(0x47e390, [RAM], regs, conv='t')
    size = lambda: r.choice([640, 800, 1024, 0, 1, r.getrandbits(32)]).to_bytes(4, 'little')  # noqa: E731
    regs.update({0x7a1968: size(), 0x7a1930: size()})
    return Call(0x47e570, [RAM, RAM + r.choice([0, 0x10, 0x80])], regs, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

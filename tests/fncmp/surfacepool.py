# The surface pool's side index and turn (src/game/gfx/surfacepool: 0x5848b0, 0x58b1f0) against the originals: sides every power of
# two to 0x100, around them, 0, negative and any; turns 0 and 1 mostly, else 2-5 or a little negative (reads near the pool).
# usage: surfacepool.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Index,Next)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x5848b0, 0x584a21), (0x58b1f0, 0x58b21e)])
CALLS = {'Index': 0x5848b0, 'Next': 0x58b1f0}
POOL = RAM + 0x100


def side(r):
    p = 1 << r.randrange(9)
    return r.choice([p, p, p, p - 1, p + 1, p + 0x10, 0, -1, -p, 0x200, r.randrange(0x111), r.getrandbits(32)]) & 0xffffffff


def case(r):
    op = r.deal(OPS)
    if op == 'Index':
        return Call(0x5848b0, [side(r)], {}, ret='a')
    area = bytearray(r.bytes(0x300))
    for k in range(-1, 9):
        put(area, 0x180 + 4 * k, 'i', r.choice([0, 1, 0, 1, 2, 5, -1, -3]))
    return Call(0x58b1f0, [POOL, side(r), r.getrandbits(32)], {RAM: area}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

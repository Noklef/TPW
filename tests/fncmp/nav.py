# The nav's position and velocity accessors, its world-to-local transform, predicted position and path progress
# (src/game/world/nav: 0x5100e0, 0x510160, 0x50f280, 0x50f870, 0x50fd40 and the jump 0x510180 to it; 0x510190, its speed limits) against the originals: outs apart, the same,
# or over the position; 16.16 values small and of any size; path totals that fit and, now and then, ones whose IDIV
# faults in both.
# usage: nav.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetPosVel,GetPos,WorldToLocal,PredictPos,PathProgress,SetSpeed)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x5100e0, 0x5100ff), (0x510160, 0x510175), (0x50f280, 0x50f30e), (0x50f870, 0x50f8dd), (0x50fd40, 0x50fde2), (0x510180, 0x510186), (0x510190, 0x5101e9),
           (0x50cd80, 0x50cdaa), (0x50df70, 0x50dfb6), (0x50ece0, 0x50ed0a)])
NAV = RAM + 0x100
OPS = os.environ.get('OPS', 'SetPosVel,GetPos,WorldToLocal,PredictPos,PathProgress,SetSpeed').split(',')


def fixed(r):
    """A 16.16 value: small, about one, or any."""
    return r.choice([0, 0x10000, -0x10000, r.randrange(-0x80000, 0x80000), r.randrange(-0x10000, 0x10000), r.getrandbits(32) - (1 << 31)])


def case(r):
    mem = bytearray(r.bytes(0x200))
    op = r.choice(OPS)
    if op == 'SetPosVel':
        return Call(0x5100e0, [NAV] + [r.getrandbits(32) for _ in range(4)], {RAM: mem}, conv='t')
    if op == 'GetPos':
        x = RAM + r.choice([0x10, 0x108, 0x10c])
        return Call(0x510160, [NAV, x, r.choice([x, RAM + 0x20, RAM + 0x108])], {RAM: mem}, conv='t')
    for off in range(0x8, 0x30, 4):  # pos, vel, max_force/speed, the local axes
        struct.pack_into('<i', mem, 0x100 + off, fixed(r))
    if op == 'WorldToLocal':
        struct.pack_into('<ii', mem, 0x20, fixed(r), fixed(r))
        point = r.choice([RAM + 0x20, NAV + 8, NAV + 0x10])
        out = r.choice([RAM + 0x40, point, NAV + 8, NAV + 0x24])
        return Call(0x50f280, [NAV, out, point], {RAM: mem}, conv='t', ret='a')
    if op == 'PredictPos':
        out = r.choice([RAM + 0x40, NAV + 8, NAV + 0xc, NAV + 0x10])
        return Call(0x50f870, [NAV, out, fixed(r) & 0xffffffff], {RAM: mem}, conv='t', ret='a')
    if op == 'SetSpeed':   # the speed about 0, 2 and the floor's 0x28f, negative, huge, NaN and infinite
        v = r.choice([0.0, -0.0, 0.01, 0.025, 0.0249, 0.05, 1.0, 1.5, 2.0, 2.0000002, 3.0, -1.0, 1e-40, 1e30, -1e30, float('nan'), float('inf'),
                      float('-inf'), r.uniform(0, 2.5), r.uniform(-1, 3)])
        bits = struct.unpack('<I', struct.pack('<f', v))[0] if r.random() < 0.95 else r.getrandbits(32)
        return Call(0x510190, [NAV, bits], {RAM: mem}, conv='t', ret='v')
    # PathProgress
    mem[0x160] = r.choice([0, 1, 1, 2, r.getrandbits(8)])
    struct.pack_into('<I', mem, 0x15c, r.choice([0, 1, 2, 3, 4, r.randrange(5)]))
    for off in range(0x164, 0x18c, 4):
        struct.pack_into('<i', mem, off, fixed(r) if r.random() < 0.3 else r.randrange(-0x80000, 0x80000))
    struct.pack_into('<ii', mem, 0x1a4, r.randrange(0, 0x400000), r.randrange(0, 0x400000))
    if r.wild and r.random() < 0.3:
        struct.pack_into('<ii', mem, 0x1a4, r.getrandbits(32) - (1 << 31), r.getrandbits(32) - (1 << 31))
    total = r.choice([0, 0, r.randrange(0x1000, 0x2000000), r.randrange(0x1000, 0x2000000), -r.randrange(0x1000, 0x2000000),
                      r.randrange(0x400000, 0x10000000), r.randrange(1, 0x100) if r.random() < 0.15 else 0x100000])
    struct.pack_into('<i', mem, 0x1a0, total)
    return Call(r.choice([0x50fd40, 0x510180]), [NAV], {RAM: mem}, conv='t', ret='a')   # (0x510180: the jump to it)


if __name__ == '__main__':
    B.main(case)

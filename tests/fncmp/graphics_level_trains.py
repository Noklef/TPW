# The graphics level and the car count of src/game/core/small34 against the originals: the render mode words (the size and depth tests), the quality level, coasters of up
# to 4 trains (coaster.py's unload environment: the trains have cars, riders and seat parts) with the flags and counts either side.
# usage: small34.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Level,Count)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import coaster  # noqa: E402
from fncmp import Bench, Call, put  # noqa: E402

CALLS = {'Level': 0x41a9d0, 'Count': 0x43ba20}
B = Bench([(0x41a9d0, 0x41aa30), (0x43ba20, 0x43ba90), (0x44a590, 0x44a600), (0x43b480, 0x43b5ff), (0x44b4c0, 0x44b505), (0x475550, 0x47558b), (0x5da3c0, 0x5da3d0)] + [(a, b) for (a, b) in [(0x412e90, 0x412eea)]])
TRAINS = coaster.CARS


def case(r):
    name = r.choice(OPS)
    if name == 'Level':
        return Call(CALLS[name], [], {0x79d86c: put(bytearray(4), 0, 'I', r.choice([0x4000, 0x2000, 0, 0x1000 | 0x4000, 0x4000 | 0x10, 0x4000 | 8, 0x4000 | 0x20, r.getrandbits(15)])),
                                      0x7858d0: put(bytearray(4), 0, 'I', r.choice([0, 1, 2, 3, 4, r.getrandbits(32)]))}, ret='a')
    call = coaster.queue(r, 'Unload')
    regs = dict(call.regions)
    old = bytearray(regs[TRAINS])
    n = r.choice([1, 2, 3, 4])
    arena = bytearray(0x100 * 8)
    train, cars = old[:0x100], old[0x100:]
    for k in range(n):
        arena[0x80 * k:0x80 * k + 0x80] = train[:0x80]
        put(arena, 0x80 * k + 8, 'III', r.randrange(3), r.choice([0, 1, 2, 3]), r.getrandbits(32))
        put(arena, 0x80 * k + 0x28, 'I', TRAINS + 0x400)
    regs[TRAINS] = arena + bytearray(0x200) + cars
    c = bytearray(regs[coaster.COAST])
    put(c, 8, 'I', r.choice([0, 4, 8, 0xc, 1, 0x1c, r.getrandbits(32)]))
    put(c, 0xf4, 'I', r.choice([0, 1, 2, 3, 4, 4]))
    put(c, 0x128, 'I', TRAINS)
    regs[coaster.COAST] = c
    return Call(CALLS[name], [coaster.COAST, r.choice([0, 0, 1, 2, 3, 4])], regs, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

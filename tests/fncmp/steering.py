# The steering behaviours (src/game/world/steering) against the originals: the constructors, base destructor and exit
# thunks (0x50ce50-0x50d140) on any bytes; seek, arrival and avoid_obstacle's steering (0x50dd80,
# 0x50ddf0, 0x50e350), their per-neighbour / per-obstacle callbacks (0x50e190, 0x50e2b0) and the circle hits
# (0x50cba0, 0x50cc20) on navs with 16.16 values near each other (so that every branch is taken), at times of any size; the
# globals (0x7cf048, 0x7cf100, 0x7cf32c) over any bytes.
# usage: steering.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Dtor,Destroy,Seek,Arrival,Neighbour,Hit,Ray,Obstacle,Avoid)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x50ce50, 0x50d14a), (0x4d49a0, 0x4d49e0), (0x50cba0, 0x50cdc0), (0x50dd80, 0x50dfd0), (0x50e190, 0x50e4b0),
           (0x50ece0, 0x50ed20), (0x50f280, 0x50f3c0), (0x50f870, 0x50f8f0), (0x5da3c0, 0x5da3c1)])
CTORS = [0x50ce50, 0x50ced0, 0x50cf40, 0x50cfb0, 0x50d020, 0x50d090, 0x50d100]
DESTROYS = [0x50ce90, 0x50cf10, 0x50cf80, 0x50cff0, 0x50d060, 0x50d0d0, 0x50d140]
GLOBALS = 0x7cf040  # the seven objects, 0x7cf050-0x7cf370, and the force, reach and nearest hit
OPS = os.environ.get('OPS', 'Ctor,Dtor,Destroy,Seek,Arrival,Neighbour,Hit,Ray,Obstacle,Avoid').split(',')
NAV, OTHER, OUT = RAM + 0x100, RAM + 0x20, RAM + 0x40


def near(r, scale=0x80000):
    """A 16.16 value: mostly within scale of 0, at times any."""
    if r.random() < (0.3 if r.wild else 0.05):
        return r.getrandbits(32) - (1 << 31)
    return r.choice([0, r.randrange(-scale, scale), r.randrange(-0x100, 0x100), r.randrange(-0x1000000, 0x1000000)])


def put(mem, at, *values):
    struct.pack_into('<%dI' % len(values), mem, at, *(v & 0xffffffff for v in values))


def case(r):
    op = r.choice(OPS)
    if op == 'Ctor':
        return Call(r.choice(CTORS), [RAM + r.choice([0x10, 0x11, 0x13])], {RAM: r.bytes(0x40)}, conv='t', ret='a')
    if op == 'Dtor':
        return Call(0x50cea0, [RAM + r.choice([0x10, 0x12])], {RAM: r.bytes(0x40)}, conv='t')
    glob = bytearray(r.bytes(0x340))
    if op == 'Destroy':
        return Call(r.choice(DESTROYS), [], {GLOBALS: glob})
    mem = bytearray(r.bytes(0x200))
    put(mem, 0x100, *(near(r) for _ in range(0x30 // 4)))  # mass, radius, pos, vel, max_force/speed, the axes
    put(mem, 0x104, r.randrange(0, 0x40000))  # radius
    put(mem, 0x11c, r.choice([0x3333, 0x10000, r.randrange(0, 0x100000), near(r)]))  # max_speed
    put(mem, 0x120, *(r.choice([0x10000, -0x10000, 0, r.randrange(-0x10000, 0x10000)]) for _ in range(4)))
    put(mem, 0x130, r.choice([OTHER, NAV + 8, NAV + 0x10]))  # the target
    pos = struct.unpack_from('<ii', mem, 0x108)
    if r.random() < 0.8:  # the other / the target near the nav
        scale = r.choice([0x40000, 0x40000, 0x2000, 0x400000])  # (the length's tiers: under 0x1000, 0x400000 and beyond)
        put(mem, 0x20, pos[0] + near(r, scale), pos[1] + near(r, scale))
    else:
        put(mem, 0x20, near(r), near(r))
    put(mem, 0x28, r.randrange(0, 0x40000) if r.random() < 0.9 else near(r))  # the other's radius
    if op == 'Seek':
        return Call(0x50dd80, [RAM, r.choice([OUT, NAV + 8, NAV + 0x10]), NAV], {RAM: mem}, conv='t', ret='a')
    if op == 'Arrival':
        return Call(0x50ddf0, [RAM, r.choice([OUT, NAV + 8, NAV + 0x10]), NAV], {RAM: mem}, conv='t', ret='a')
    if op == 'Avoid':
        return Call(0x50e350, [RAM, r.choice([OUT, NAV + 8, NAV + 0x10]), NAV], {RAM: mem, GLOBALS: glob}, conv='t', ret='a')
    if op == 'Neighbour':
        return Call(0x50e190, [NAV, OTHER], {RAM: mem, GLOBALS: glob})
    if op == 'Hit':
        if r.random() < 0.2:  # a zero discriminant: x 0, y under 2^18, a radius whose square is 0
            put(mem, 0x20, 0, r.randrange(0, 0x40000))
            return Call(0x50cba0, [OTHER, r.randrange(0, 0x100)], {RAM: mem}, ret='a')
        return Call(0x50cba0, [OTHER, r.choice([r.randrange(0, 0x40000), near(r)]) & 0xffffffff], {RAM: mem}, ret='a')
    if op == 'Ray':  # from (the other), a direction and a centre
        scale = r.choice([0x10000, 0x40000, 0x400000])
        put(mem, 0x30, near(r, 0x10000), near(r, 0x10000), near(r, scale), near(r, scale))
        return Call(0x50cc20, [OTHER, RAM + 0x30, r.choice([RAM + 0x38, OTHER]), r.choice([r.randrange(0, 0x40000), r.randrange(0x100), near(r)]) & 0xffffffff],
                    {RAM: mem}, ret='a')
    # Obstacle: the reach and the nearest hit so far
    put(glob, 0x8, r.choice([r.randrange(0, 0x100000), near(r)]))
    put(glob, 0x2ec, r.choice([0x7fffffff, r.randrange(0, 0x100000), near(r)]))
    return Call(0x50e2b0, [NAV, OTHER], {RAM: mem, GLOBALS: glob})


if __name__ == '__main__':
    B.main(case)

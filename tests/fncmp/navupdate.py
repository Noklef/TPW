# The nav's step of src/game/world/nav (0x50f3b0, and the jump 0x510150 to it) against the original: a behaviour list of up to three entries (two objects whose force is a recorder's
# that writes the case's force; weights about 1.0 and any), navs whose 16.16 values are near each other (so that the truncations and the path progress take
# each branch), and for the move test (the pointer at 0x762190, laid out here) the cdecl logger of the two map ids, passable or not (the native driver
# cannot call the game's code through a pointer: its own code is at those addresses).
# usage: navupdate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG2, LOG, Bench, Call, RAM, echo, fill, log_region, recorder, set_return, strings  # noqa: E402

B = Bench([(0x50f3b0, 0x50f5f5), (0x510150, 0x510156), (0x50dd80, 0x50dfd0), (0x50cba0, 0x50cdc0), (0x4d49a0, 0x4d49e0), (0x50e080, 0x50e0d4), (0x50ece0, 0x50ed20),
           (0x50f1a0, 0x50f202), (0x50f600, 0x50f6f9), (0x50f700, 0x50f84a), (0x50f850, 0x50f86f), (0x4d99c0, 0x4d99d3), (0x4d97b0, 0x4d97d9),
           (0x5104b0, 0x510502), (0x510870, 0x51089d), (0x4d8300, 0x4d8328), (0x50fd40, 0x50fde2), (0x5da3c0, 0x5da3c1)])
NAV, TARGET, OBJECTS, LIST, VTABLES = RAM + 0x100, RAM + 0x20, RAM + 0x1c0, RAM + 0x1000, RAM + 0x2000


def near(r, scale=0x80000):
    """A 16.16 value: mostly within scale of 0, at times any."""
    if r.random() < (0.3 if r.wild else 0.05):
        return r.getrandbits(32) - (1 << 31)
    return r.choice([0, r.randrange(-scale, scale), r.randrange(-0x100, 0x100), r.randrange(-0x1000000, 0x1000000)])


def put(mem, at, *values):
    struct.pack_into('<%dI' % len(values), mem, at, *(v & 0xffffffff for v in values))


def case(r):
    mem = bytearray(r.bytes(0x200))
    nav = NAV - RAM
    # pos about the first row (cells 0-2 and the edges), vel, max_force, max_speed; the target near the pos
    cells = [r.choice([0, 1, 2, 125, 126, 127, 128, r.randrange(128)]), r.choice([0, 0, 0, 1, 2, 127, r.randrange(128)])]
    if r.random() < 0.1:
        cells[r.randrange(2)] = r.choice([-1, -2, 0x8000])
    pos = [(c << 16) + r.randrange(0x10000) for c in cells]
    put(mem, nav + 4, r.randrange(0, 0x40000))
    put(mem, nav + 8, *pos)
    put(mem, nav + 0x10, *(r.choice([0, near(r, 0x10000), near(r, 0x40000), r.randrange(-0x20000, 0x20000)]) for _ in range(2)))
    put(mem, nav + 0x18, r.choice([0x6666, 0x10000, 0x3333, r.randrange(0, 0x40000), near(r)]))
    put(mem, nav + 0x1c, r.choice([0x3333, 0x10000, r.randrange(0, 0x40000), near(r)]))
    put(mem, nav + 0x30, TARGET)
    put(mem, TARGET - RAM, *(p + r.choice([0, near(r, 0x40000), near(r, 0x2000), near(r, 0x400000)]) for p in pos))
    # the path progress: the done flag, the point, the five points, the distances and the total; the last progress and the history
    mem[nav + 0x60] = r.choice([0, 1, 1, 2])
    put(mem, nav + 0x5c, r.randrange(5))
    for k in range(5):
        put(mem, nav + 0x64 + 8 * k, *(p + r.randrange(-0x80000, 0x80000) for p in pos))
    put(mem, nav + 0xa4, r.randrange(0, 0x400000), r.randrange(0, 0x400000))
    put(mem, nav + 0xa0, r.choice([0, r.randrange(0x1000, 0x2000000), r.randrange(0x400000, 0x10000000), 0x100000]))
    put(mem, nav + 0xac, r.choice([0, 0x10000, -0x10000, 0x8000, r.randrange(-0x20000, 0x20000), r.getrandbits(32)]))
    put(mem, nav + 0xb0, r.getrandbits(32))
    put(mem, nav + 0xb4, r.getrandbits(32))
    # the behaviour objects, whose force (vtable slot 0, thiscall (out, nav), returning out) is a recorder's: each object its own, which writes the
    # case's force through out and returns it; the list
    log = log_region()
    for k in range(2):
        put(mem, OBJECTS - RAM + 0x20 * k, VTABLES + 4 * k)
        strings(log, k, 0, 1)  # (out: a stack address)
        echo(log, k, 0)
        fill(log, k, 0, struct.pack('<2I', *(near(r, 0x40000) & 0xffffffff for _ in range(2))), k)
    # the move test: the cdecl logger of two arguments (the map ids; the flags go unlogged), passable or not
    set_return(log, 0x3e, r.choice([0, 0, 1, 1, r.getrandbits(32)]))
    n = r.choice([0, 1, 2, 3])
    entries = bytearray(r.bytes(0x20))
    for k in range(n):
        put(entries, 8 * k, r.choice([0x10000, 0x8000, 0xe666, 0x1999, 0, -0x10000, r.getrandbits(32)]), OBJECTS + 0x20 * r.randrange(2))
    regions = {RAM: mem, LIST: entries, 0x7cf078: struct.pack('<II', LIST, LIST + 8 * n), 0x762190: struct.pack('<I', CDECL_LOG2), LOG: log,
               VTABLES: struct.pack('<2I', recorder(0, 2), recorder(1, 2))}
    return Call(r.choice([0x50f3b0, 0x510150]), [NAV], regions, conv='t', ret='v')   # (0x510150: the jump to it)


if __name__ == '__main__':
    B.main(case)

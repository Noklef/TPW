# The staff constructors' common tail (src/game/world/staff: 0x504b90) against the original: a staff member of random bytes with types about 3 (and wild), the world's
# calendar clock and its imports as funnytime.py serves them (the hire date: the funny time now), the message centre (thing_ctors.py's, made: its maps of 0 to 4 keys; making it
# here would copy the funny time's stack leftovers), the words at 0x75c7f0 / 0x75c7f8 / 0x75c7fc random, and the park the world's +0x1da720 names in the thing table placed so that its time stamp lands in OUT.
# usage: staffinit.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import funnytime  # noqa: E402
import thing_ctors  # noqa: E402
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

B = Bench([(0x504b90, 0x504c70), (0x519510, 0x519550), (0x50b350, 0x50b370), (0x5da3c0, 0x5da3c1)] + funnytime.RANGES + thing_ctors.CENTRE_RANGES,
          hooks=thing_ctors.CENTRE_HOOKS)
STAFF, OUT = RAM + 0x1000, RAM + 0x2000
WORLD = funnytime.CLOCK - 0x2a0   # (the world's clock at CLOCK)


def case(r):
    regions = {funnytime.CLOCK: bytearray(r.bytes(0x40)), HEAP: heap(), OUT: r.bytes(0x10)}
    funnytime.clock_regions(r, regions)
    ticks = bytearray(regions.pop(funnytime.TICKS)) + bytearray(8)
    slot = r.randrange(1, 0x100)
    put(ticks, 0x14, 'H', slot)   # (the world's +0x1da720: the park's handle)
    regions[0x80239c] = struct.pack('<I', WORLD)
    regions[WORLD + 0x1da70c] = ticks
    regions[0x7cfb90 + 20 * slot] = struct.pack('<I', OUT - 0x21c38)
    regions[0x75c7f0] = r.bytes(0x10)
    staff = bytearray(r.bytes(0x220))
    put(staff, 0x1e4, 'i', r.choice([0, 1, 2, 3, 4, 5, -1, 0x7fffffff, -0x80000000, r.getrandbits(32) - (1 << 31)]))
    regions[STAFF] = staff
    while regions.get(0x788d3c, b'') in (b'', bytes(4)):   # (the centre made already: making it copies uninitialised stack bytes into each slot (sic), which
        thing_ctors.centre(r, regions)                   # here hold what the funny time's calls left, so the two differ)
    return Call(0x504b90, [STAFF], regions, conv='t')


if __name__ == '__main__':
    B.main(case)

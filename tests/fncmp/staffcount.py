# The staff count of src/game/world/staff (0x508000) against the original: a park of up to six things (thingenv.py) of the staff kinds 4-8 and others, counts in the
# object reset first, and limits (0x7852e8-0x7852f8, in .bss: laid out here) of 0 to 3 and negative, so that each type's flag is set or not.
# usage: staffcount.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM  # noqa: E402
from thingenv import park  # noqa: E402

B = Bench([(0x508000, 0x508153), (0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1)])
POOLOBJ = RAM + 0x40000


def case(r):
    regions, _handles, _things = park(r, kinds=(4, 5, 6, 7, 8, 4, 5, 6, 7, 8, 3, 1, 9), defs=False)
    regions[POOLOBJ] = r.bytes(0x300)
    regions[0x7852e8] = b''.join(struct.pack('<i', r.choice([0, 1, 1, 2, 3, -1, 0x7fffffff])) for _ in range(5))
    return Call(0x508000, [POOLOBJ], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

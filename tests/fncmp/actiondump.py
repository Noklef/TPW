# The action history's debug dump (src/game/world/person: 0x4faa60) against the original: a person whose history (+0x30: 32 (action, handle)
# word pairs from +4, the count at +0x88) names thingenv's park (objects, people, staff), handle 0 or a slot not in use (its thing pointer random:
# mostly 0, in a few cases random: a fault in both), the actions 0..0x19 and others; the count mostly within the 32, now and then past them (the entries from
# +0x84 on: still within the person). Only the reads remain, so the result (-1) and the person compare.
# usage: actiondump.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM  # noqa: E402
from thingenv import SLOTS, park  # noqa: E402

B = Bench([(0x4faa60, 0x4fada1), (0x50c190, 0x50c1d0), (0x5da3c0, 0x5da3c1)])
PERSON = RAM + 0x20000


def case(r):
    regions, handles, things = park(r, kinds=(3, 3, 1, 2, 4, 5, 0))
    slots = bytearray(regions[SLOTS])
    wild = r.random() < 0.05   # (now and then a slot not in use keeps its random thing pointer)
    for h in range(12):
        if h not in handles and not (wild and r.random() < 0.3):
            struct.pack_into('<I', slots, 20 * h, 0)   # (a slot not in use: no thing)
    regions[SLOTS] = slots
    person = bytearray(r.bytes(0x200))
    for i in range(32):
        handle = r.choice(handles * 3 + [0, 0, r.randrange(12)]) if handles else r.choice([0, r.randrange(12)])
        struct.pack_into('<HH', person, 0x34 + 4 * i, r.choice(list(range(0x1a)) + [0x1a, 0xffff, r.getrandbits(16)]), handle)
    struct.pack_into('<I', person, 0xb8, r.choice([r.randrange(32), r.randrange(32), 0, 31, 32, 33, 0x40]))
    regions[PERSON] = person
    return Call(0x4faa60, [PERSON], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

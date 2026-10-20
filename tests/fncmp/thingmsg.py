# The thing messages' constructors and Type functions (src/game/world/thingmsg: 0x40f240-0x40f920; message 15's Type and deleting destructor at 0x405530 / 0x405540)
# against the originals, each built over junk with junk arguments (the narrow ones' high bits too); message 05's 0x40f300 with a record to copy.
# usage: thingmsg.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap  # noqa: E402

B = Bench([(0x40f240, 0x40f926), (0x59b550, 0x59b590), (0x405530, 0x405560)])
MSG = RAM + 0x100
# address: argument count
CTORS = {0x40f290: 1, 0x40f240: 2, 0x40f330: 1, 0x40f380: 1, 0x40f3d0: 1, 0x40f420: 1, 0x40f470: 1, 0x40f4c0: 0,
         0x40f500: 0, 0x40f540: 0, 0x40f580: 2, 0x40f5e0: 1, 0x40f630: 1, 0x40f680: 1, 0x40f6d0: 1, 0x40f6f0: 2,
         0x40f740: 4, 0x40f7a0: 11, 0x40f840: 4, 0x40f900: 1, 0x40f8b0: 1}
TYPES = [0x40f260, 0x40f2d0, 0x40f350, 0x40f3a0, 0x40f3f0, 0x40f440, 0x40f490, 0x40f4d0, 0x40f510, 0x40f550,
         0x40f5b0, 0x40f600, 0x40f650, 0x40f6a0, 0x40f710, 0x40f770, 0x40f810, 0x40f880, 0x40f8d0, 0x40f920, 0x405530]
OPS = list(CTORS) + TYPES + [0x40f300, 0x405540]


def case(r):
    at = r.deal(OPS)
    if at == 0x40f300:   # message 05, its record at +8 copied from the one the argument points at
        return Call(0x40f300, [MSG, RAM + 0x200 + r.choice([0, 4, 0x20])], {RAM: r.bytes(0x400)}, conv='t', ret='a')
    if at == 0x405540:   # message 15's deleting destructor: freed when bit 0 of the flags (a byte) is set
        flags = r.choice([0, 1, 2, 3, 0x100, 0x101, r.getrandbits(32)])
        return Call(at, [MSG, flags], {RAM: r.bytes(0x400), HEAP: heap(0x100)}, conv='t', ret='a')
    if at in TYPES:
        return Call(at, [MSG], {RAM: r.bytes(0x400)}, conv='t', ret='a')
    return Call(at, [MSG] + [r.getrandbits(32) for _ in range(CTORS[at])], {RAM: r.bytes(0x400)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

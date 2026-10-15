# The thing messages' constructors and Type functions (src/game/world/thingmsg: 0x40f240-0x40f920)
# against the originals, each built over junk with junk arguments (the narrow ones' high bits too).
# usage: thingmsg.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x40f240, 0x40f926)])
MSG = RAM + 0x100
# address: argument count
CTORS = {0x40f290: 1, 0x40f240: 2, 0x40f330: 1, 0x40f380: 1, 0x40f3d0: 1, 0x40f420: 1, 0x40f470: 1, 0x40f4c0: 0,
         0x40f500: 0, 0x40f540: 0, 0x40f580: 2, 0x40f5e0: 1, 0x40f630: 1, 0x40f680: 1, 0x40f6d0: 1, 0x40f6f0: 2,
         0x40f740: 4, 0x40f7a0: 11, 0x40f840: 4, 0x40f900: 1, 0x40f8b0: 1}
TYPES = [0x40f260, 0x40f2d0, 0x40f350, 0x40f3a0, 0x40f3f0, 0x40f440, 0x40f490, 0x40f4d0, 0x40f510, 0x40f550,
         0x40f5b0, 0x40f600, 0x40f650, 0x40f6a0, 0x40f710, 0x40f770, 0x40f810, 0x40f880, 0x40f8d0, 0x40f920]


def case(r):
    if r.random() < 0.3:
        return Call(r.choice(TYPES), [MSG], {RAM: r.bytes(0x400)}, conv='t', ret='a')
    at = r.choice(list(CTORS))
    return Call(at, [MSG] + [r.getrandbits(32) for _ in range(CTORS[at])], {RAM: r.bytes(0x400)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

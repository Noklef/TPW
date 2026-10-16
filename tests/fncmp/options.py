# The game options' revert / keep and setters (src/game/core/options: 0x423ad0-0x423ba0) against the
# originals, over junk with +0x94 either sign and values equal to the field or not.
# usage: options.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x423ad0, 0x423afc), (0x423b00, 0x423b36), (0x423b40, 0x423b4a), (0x423b60, 0x423b71),
           (0x423b80, 0x423b91), (0x423ba0, 0x423bb1)])
OPT = RAM + 0x100
SETS = {0x423b40: 0x30, 0x423b60: 4, 0x423ba0: 8, 0x423b80: 0xc}


def case(r):
    mem = put(r.bytes(0x400), 0x194, 'i', r.choice([-2, -1, 0, 1, 2, r.getrandbits(31), -r.getrandbits(31)]))
    at = r.choice([0x423ad0, 0x423b00] + list(SETS))
    if at in SETS:
        cur = struct.unpack_from('<I', mem, 0x100 + SETS[at])[0]
        return Call(at, [OPT, r.choice([cur, r.getrandbits(32)])], {RAM: mem}, conv='t')
    return Call(at, [OPT], {RAM: mem}, conv='t')


if __name__ == '__main__':
    B.main(case)

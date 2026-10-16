# The texture cache surface getters (src/game/gfx/texsurface: 0x58c2c0, 0x58c2e0, 0x58c330, 0x58c8a0,
# 0x58c8b0) against the originals: any object bytes, the vbtable pointers set to a table whose
# offset reaches back and forward.
# usage: texsurface.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x58c2c0, 0x58c2c4), (0x58c2e0, 0x58c2f2), (0x58c330, 0x58c334), (0x58c8a0, 0x58c8a4), (0x58c8b0, 0x58c8c2)])
CALLS = {'A38': 0x58c2c0, 'AArea': 0x58c2e0, 'A10': 0x58c330, 'BC': 0x58c8a0, 'BArea': 0x58c8b0}
SUB = RAM + 0x100
VB = RAM + 0x1000


def case(r):
    op = r.choice(list(CALLS))
    obj = bytearray(r.bytes(0x200))
    put(obj, 0x100 - 0x3c, 'I', VB)
    put(obj, 0x100 - 0x10, 'I', VB)
    vb = struct.pack('<ii', 0, r.choice([0, 0x10, 0x30, 0x38, 0x3c, 0x40, 0x50, -0x10, 0x80]))
    return Call(CALLS[op], [SUB], {RAM: obj, VB: vb}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

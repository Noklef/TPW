# The constructor of src/game/core/misc's class 0x701378 (0x5667d0 Class701378_Ctor) against the original: an object whose vtable gives the virtual base offset, with the flag set
# or clear (the base class constructor needs the driver quirk globals).
# usage: class701378.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5651c0, 0x565220), (0x5667d0, 0x566850)])
VT, OBJ = RAM + 0x1000, RAM


def case(r):
    offset = r.choice([0x2c, 0x30, 0x34, 0x40, 0x20])
    obj = bytearray(r.bytes(0x100))
    put(obj, 0, 'I', VT)
    regions = {OBJ: obj, VT: put(bytearray(r.bytes(0x10)), 4, 'I', offset), 0x879a20: put(bytearray(r.bytes(0x20)), 4, 'I', RAM + 0x2000), 0x879ab0: r.bytes(0x10),
               0x8bd520: r.bytes(4) + struct.pack('<I', r.choice([1, 1, 0, 4])), RAM + 0x2000: put(bytearray(r.bytes(0x500)), 0x428, 'I', r.choice([0x10de, 0x10de, 0x1002, 0]))}
    return Call(0x5667d0, [OBJ, r.choice([0, 1, 1, 0xffffff00, 7])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

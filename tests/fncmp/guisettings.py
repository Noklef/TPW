# The GUI library's small setters (src/game/ui/guisettings) against the original. The cursor's slots recorders, Fn_669c30 real.
# usage: guisettings.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, vtable  # noqa: E402

B = Bench([(0x65889c, 0x6588b6), (0x6590a5, 0x6590c3), (0x6590d0, 0x6590ea), (0x6591d4, 0x65921f), (0x6595b3, 0x6595cd), (0x659674, 0x65968e), (0x669c30, 0x669c41)])
CURSOR, KVTABLE, LIST = RAM + 0x1000, RAM + 0x1100, RAM + 0x1200


def case(r):
    regions = {LOG: log_region(), KVTABLE: vtable({2: 0, 3: 0}), CURSOR: struct.pack('<I', KVTABLE), LIST: r.bytes(0x20)}
    block = bytearray(r.bytes(0xd0))
    struct.pack_into('<I', block, 0xb0, r.choice([LIST, LIST, 0]))
    struct.pack_into('<I', block, 0xc8, r.choice([CURSOR, CURSOR, 0]))
    regions[0xfaa540] = block
    regions[0x77c48c] = r.bytes(8)
    value = r.choice([0, r.getrandbits(32), r.getrandbits(32) & 0xffff0000])
    other = r.getrandbits(32)
    function, args, ret = r.choice([(0x65889c, [value, other], 'v'), (0x6590a5, [], 'a'), (0x6590d0, [value, other], 'v'), (0x6591d4, [value], 'v'), (0x6591e3, [], 'v'), (0x659201, [], 'v'),
                                    (0x6595b3, [value], 'v'), (0x659674, [value], 'v')])
    return Call(function, args, regions, ret=ret)


if __name__ == '__main__':
    B.main(case)

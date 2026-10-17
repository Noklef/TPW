# The constructor of src/game/core/vdtor's class 0x702bc8 (0x5dc580 Class702bc8_Ctor) against the original: an object whose base constructor registers it with the global object
# through a recorder; the heap hands out the two helper blocks, so a failed allocation is not covered.
# usage: class702bc8.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, vtable  # noqa: E402

B = Bench([(0x5dc580, 0x5dc660), (0x5e3b90, 0x5e3bc0), (0x67acb0, 0x67acb1)])
OTHER, VT = RAM + 0x200, RAM + 0x1000


def case(r):
    regions = {HEAP: heap(), LOG: log_region(), RAM: r.bytes(0x100), 0xf85614: struct.pack('<I', OTHER), OTHER: struct.pack('<I', VT) + r.bytes(0x20), VT: vtable({6: 1, 7: 1})}
    return Call(0x5dc580, [RAM], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

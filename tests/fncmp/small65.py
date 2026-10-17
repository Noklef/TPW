# The constructors and factory of src/game/core/small65 (0x5dc580, 0x617ff0, 0x50ffe0) against the originals: an object whose base constructor registers it with the global
# object through a recorder (Ctor); an out pair of words with a null or live object pointer (Create); an object of random bytes for the constants constructor (Consts). The heap hands out blocks, so the failed allocation is not covered.
# usage: small65.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, vtable  # noqa: E402

B = Bench([(0x5dc580, 0x5dc660), (0x5e3b90, 0x5e3bc0), (0x617ff0, 0x6180b0), (0x6338b0, 0x6338d0), (0x67acb0, 0x67acb1), (0x50ffe0, 0x5100f0), (0x50f1f0, 0x50f200), (0x5d1750, 0x5d1751), (0x67acf0, 0x67ada0), (0x67b050, 0x67b0d0)])
OBJ, OTHER, VT, OUT, SLOT = RAM, RAM + 0x200, RAM + 0x1000, RAM + 0x2000, RAM + 0x2100


def case(r):
    regions = {HEAP: heap(), LOG: log_region()}
    if r.random() < 0.3:
        return Call(0x50ffe0, [OBJ], {OBJ: r.bytes(0x100)}, conv='t', ret='a')
    if r.random() < 0.5:
        regions.update({OBJ: r.bytes(0x100), 0xf85614: struct.pack('<I', OTHER), OTHER: struct.pack('<I', VT) + r.bytes(0x20), VT: vtable({6: 1, 7: 1})})
        return Call(0x5dc580, [OBJ], regions, conv='t', ret='a')
    regions.update({OUT: r.bytes(8), SLOT: r.bytes(8)})
    return Call(0x617ff0, [OUT, r.choice([SLOT, SLOT, 0])], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

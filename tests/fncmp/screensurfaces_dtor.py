# The screen surfaces' destructor of src/game/gfx/screensurfaces (0x47e3e0 ScreenSurfaces_Dtor) against the original: an object holding two COM objects (+0xac, +0xb0) through
# recorders, either of them null.
# usage: screensurfaces_dtor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, put, vtable  # noqa: E402

B = Bench([(0x47e3e0, 0x47e460), (0x5da3c0, 0x5da3c1)])
SUB1, SUB2, VTC = RAM + 0x2200, RAM + 0x2300, RAM + 0x2400


def case(r):
    obj = bytearray(r.bytes(0x100))
    put(obj, 0xac, 'I', r.choice([SUB1, 0]))
    put(obj, 0xb0, 'I', r.choice([SUB2, 0]))
    regions = {RAM: obj, SUB1: struct.pack('<I', VTC) + r.bytes(0x20), SUB2: struct.pack('<I', VTC) + r.bytes(0x20), VTC: vtable({}, com={2: 1}), LOG: log_region()}
    return Call(0x47e3e0, [RAM], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

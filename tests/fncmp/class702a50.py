# The class 0x702a50's destructor (src/game/frontend/lobbyscene: 0x5d80c0) against the original: the registry [0xf83774] a recorder object (slot 8), Ride_Destroy (k 1) and String_Dtor (k 2) recorders.
# usage: class702a50.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, vtable  # noqa: E402

B = Bench([(0x5d80c0, 0x5d8130)], hooks={0x463c80: _recorder(1, 1, cdecl=True), 0x5f61e0: _recorder(2, 0)})
OBJECT, REGISTRY, VT = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    regions = {LOG: log_region(), OBJECT: r.bytes(0x40), 0xf83774: struct.pack('<I', REGISTRY), REGISTRY: struct.pack('<I', VT), VT: vtable({8: 1})}
    return Call(0x5d80c0, [OBJECT], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

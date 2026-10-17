# The 0x703780 class destructors of src/game/core/class703780_dtors against the originals: a member object whose slot 0 is a recorder, or none.
# usage: class703780_dtors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=A,B,C)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, put, vtable  # noqa: E402

CALLS = {'A': 0x617a80, 'B': 0x617b80, 'C': 0x6180c0}
B = Bench([(a, a + 0x50) for a in CALLS.values()])
VT = RAM + 0x1000


def case(r):
    address = CALLS[r.choice(OPS)]
    obj = bytearray(r.bytes(0x40))
    put(obj, 8, 'I', RAM + 0x200 if r.random() < 0.8 else 0)
    regions = {RAM: obj, LOG: log_region(), RAM + 0x200: struct.pack('<I', VT) + r.bytes(0x20), VT: vtable({0: 0})}
    return Call(address, [RAM], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The mouse handlers of src/game/input/mouse (0x46c210, 0x46c2a0) against the originals: the mouse object at [0x7b05d8] is a recorder (the log shows which slot is called with which
# arguments), the last position and the button flags are random.
# usage: mouse.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Button,Moved)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, put, vtable  # noqa: E402

B = Bench([(0x46c210, 0x46c2a0), (0x46c2a0, 0x46c330), (0x5da3c0, 0x5da3c1)])
MOUSE, VT = RAM + 0x2000, RAM + 0x2100


def case(r):
    name = r.choice(OPS)
    glob = bytearray(struct.pack('<IIIIIIII', *[r.choice([0, 1, 5, 0x80000000]) for _ in range(8)]))  # 0x7b05d4 .. 0x7b05f4: the last position, the mouse object, the flags
    put(glob, 4, 'I', MOUSE)
    put(glob, 0x10, 'I', r.choice([0, 1, 5]))
    put(glob, 0x1c, 'I', r.choice([0, 1, 5]))
    regions = {MOUSE: struct.pack('<I', VT) + r.bytes(0x20), VT: vtable({1: 2, 2: 2, 3: 2, 4: 2, 6: 4, 7: 2, 8: 2}), LOG: log_region(), 0x7b05d4: glob}
    if name == 'Button':
        return Call(0x46c210, [r.choice([0xf0, 0xf1, 0xf0, 0xf1, 0, 0xef]), r.choice([1, 0, 2, 1]), r.getrandbits(32), r.getrandbits(32)], regions, conv='c', ret='a')
    return Call(0x46c2a0, [r.choice([0, 1, 5, 0x80000000, 0xffffffff]), r.choice([0, 1, 5, 7])], regions, conv='s', ret='v')


OPS = [k for k in os.environ.get('OPS', 'Button,Moved').split(',') if k]

if __name__ == '__main__':
    B.main(case)

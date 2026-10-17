# The holder teardown, mouse handlers and COM release of src/game/core/small67 (0x45bc60, 0x46c210, 0x46c2a0, 0x47e3e0) against the originals: small58.py's holder in the renderer
# with a part whose vtable (a recorder) gives the virtual base offset (Holder); the mouse object at [0x7b05d8] as a recorder (Button, Moved); an object holding two COM
# objects through recorders (Release).
# usage: small67.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Holder,Button,Moved,Release)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import small58  # noqa: E402
from fncmp import Bench, Call, LOG, RAM, log_region, put, vtable  # noqa: E402

B = Bench([(0x45bc60, 0x45bcb0), (0x45bdd0, 0x45be90), (0x46c210, 0x46c2a0), (0x46c2a0, 0x46c330), (0x47e3e0, 0x47e460), (0x576540, 0x5765d0), (0x5da3c0, 0x5da3c1)])
MOUSE, VT, SUB1, SUB2, VTC, OWNER = RAM + 0x2000, RAM + 0x2100, RAM + 0x2200, RAM + 0x2300, RAM + 0x2400, RAM + 0x2500


def holder(r):
    c = small58.sleep(r)
    obj = c.regions[RAM]
    offset = struct.unpack_from('<I', obj, 0x304)[0]
    vt = vtable({0: 1})
    put(vt, 4, 'I', offset)
    obj[0x300:0x300 + len(vt)] = vt
    put(obj, 0x100 + offset, 'I', RAM + 0x300)
    put(obj, 0x8, 'I', r.choice([0, OWNER]))
    c.regions[OWNER] = r.bytes(0x40)
    c.regions[LOG] = log_region()
    return Call(0x45bc60, [RAM], c.regions, conv='c', ret='v')


def mouse(r, name):
    glob = bytearray(struct.pack('<IIIIIIII', *[r.choice([0, 1, 5, 0x80000000]) for _ in range(8)]))  # 0x7b05d4 .. 0x7b05f4: the last position, the mouse object, the flags
    put(glob, 4, 'I', MOUSE)
    put(glob, 0x10, 'I', r.choice([0, 1, 5]))
    put(glob, 0x1c, 'I', r.choice([0, 1, 5]))
    regions = {MOUSE: struct.pack('<I', VT) + r.bytes(0x20), VT: vtable({1: 2, 2: 2, 3: 2, 4: 2, 6: 4, 7: 2, 8: 2}), LOG: log_region(), 0x7b05d4: glob}
    if name == 'Button':
        return Call(0x46c210, [r.choice([0xf0, 0xf1, 0xf0, 0xf1, 0, 0xef]), r.choice([1, 0, 2, 1]), r.getrandbits(32), r.getrandbits(32)], regions, conv='c', ret='a')
    return Call(0x46c2a0, [r.choice([0, 1, 5, 0x80000000, 0xffffffff]), r.choice([0, 1, 5, 7])], regions, conv='s', ret='v')


def release(r):
    obj = bytearray(r.bytes(0x100))
    put(obj, 0xac, 'I', r.choice([SUB1, 0]))
    put(obj, 0xb0, 'I', r.choice([SUB2, 0]))
    regions = {RAM: obj, SUB1: struct.pack('<I', VTC) + r.bytes(0x20), SUB2: struct.pack('<I', VTC) + r.bytes(0x20), VTC: vtable({}, com={2: 1}), LOG: log_region()}
    return Call(0x47e3e0, [RAM], regions, conv='t', ret='v')


def case(r):
    name = r.choice(OPS)
    if name == 'Holder':
        return holder(r)
    if name == 'Release':
        return release(r)
    return mouse(r, name)


OPS = [k for k in os.environ.get('OPS', 'Holder,Button,Moved,Release').split(',') if k]

if __name__ == '__main__':
    B.main(case)

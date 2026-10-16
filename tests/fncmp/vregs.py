# The register shadows, seek check and list drains (src/game/core/vregs) against the originals: a member object with a vtable of COM-style recorders, shadows around the
# written values, whence values around 0 to 2 (see tools/fncmp.py recorder()).
# usage: vregs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, set_return, vtable  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'B': (0x5679b0, 't', 'v', 0x60), 'S2': (0x567a10, 't', 'v', 0x60), 'S3': (0x567a70, 't', 'v', 0x60), 'Sk': (0x5c4fa0, 'c', 'a', 0x60),
         'D0c': (0x5d8a10, 't', 'v', 0x60), 'D04': (0x5da3e0, 't', 'v', 0x60)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x67acb0, 0x67acb1)])
OBJ, MEM, VTO, VTM, LST, NODES, VT = RAM, RAM + 0x300, RAM + 0x1000, RAM + 0x1200, RAM + 0x400, RAM + 0x500, RAM + 0x1400
VALUES = [0, 1, 0x0f, 0x10, 0x55, 0xaa, 0xff, 0x100, 0x1234, 0x80000000, 0xffffffff]


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    log = log_region()
    regions = {OBJ: obj, LOG: log, HEAP: heap()}
    if name in ('B', 'S2', 'S3'):
        off = r.choice([0, 0x40, 0x60])
        put(obj, 0, 'I', VTO)
        put(obj, 0x24, 'I', MEM)
        put(obj, off + 0x14, 'I', r.choice(VALUES))
        regions[VTO] = bytearray(vtable({}))
        struct.pack_into('<I', regions[VTO], 4, off)
        regions[MEM] = struct.pack('<I', VTM) + r.bytes(0x20)
        regions[VTM] = vtable({}, size=48, com={22: 3, 40: 4})
        if name == 'B':
            return Call(address, [OBJ, r.choice(VALUES)], regions, conv=conv, ret=ret)
        if name == 'S2':
            return Call(address, [OBJ, r.choice(VALUES), r.choice(VALUES), r.choice([0x13, 0x14, 7]), r.getrandbits(32), r.getrandbits(32)], regions, conv=conv, ret=ret)
        return Call(address, [OBJ, r.choice(VALUES), r.getrandbits(32), r.choice(VALUES), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'Sk':
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({4: 2})
        if r.random() < 0.6:
            set_return(log, 4, r.choice([0, 1, 0xffffffff, 0x80000000, 0x7fffffff]))
        return Call(address, [OBJ, r.getrandbits(32), r.choice([0, 1, 2, 3, 0xffffffff, 7])], regions, conv=conv, ret=ret)
    field = 0xc if name == 'D0c' else 4
    n = r.choice([0, 1, 2, 3])
    put(obj, field, 'I', LST)
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'II', NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0)
    put(lst, 0xc, 'I', n)
    nodes = bytearray(r.bytes(0x60))
    for k in range(n):
        put(nodes, 0x10 * k, 'III', RAM + 0x700 + 0x40 * k if r.random() < 0.85 else 0, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
        regions[RAM + 0x700 + 0x40 * k] = struct.pack('<I', VT) + r.bytes(0x20)
    regions.update({LST: lst, NODES: nodes, VT: vtable({0: 1})})
    return Call(address, [OBJ], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

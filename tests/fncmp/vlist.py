# The loops that call a virtual method of each element of an array or list (src/game/core/vlist) against the originals: objects whose vtables point at the recorders
# (see tools/fncmp.py recorder()), so the comparison covers the order of the calls, the objects and arguments (the log) and the results used.
# usage: vlist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, put, vtable  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'C': (0x5ed460, 's', 'v', 0x30), 'F': (0x5ed500, 'c', 'v', 0x30), 'B': (0x5ed530, 'c', 'a', 0x30), 'E8': (0x5e40e0, 't', 'v', 0x30),
         'Ec': (0x5e4110, 't', 'v', 0x30), 'Lc': (0x5d5cc0, 't', 'a', 0x30)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()])
OBJ, SENT, NODES, ARR, DEST, VT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x800, RAM + 0x900, RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    regions = {LOG: log_region()}
    if name in ('C', 'F', 'B'):
        n = r.choice([0, 1, 2, 3])
        arr = bytearray(r.bytes(0x14 * 4))
        dest = bytearray(r.bytes(0x14 * 4))
        for k in range(4):
            put(arr, 0x14 * k, 'I', VT)
            put(dest, 0x14 * k, 'I', VT)
        regions.update({ARR: arr, DEST: dest, VT: vtable({3: 1, 6: 1})})
        first, last = ARR, ARR + 0x14 * n
        if name == 'C':
            regions[VT] = vtable({3: 1})
            return Call(address, [first, last], regions, conv=conv, ret=ret)
        if name == 'F':
            return Call(address, [first, last, RAM + 0x400], regions, conv=conv, ret=ret)
        return Call(address, [first, last, DEST + 0x14 * n], regions, conv=conv, ret=ret)
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x18, 'I', SENT)
    put(obj, 0x14, 'I', r.getrandbits(32))
    n = r.choice([0, 1, 2, 3])
    nodes = bytearray(r.bytes(0x40))
    for k in range(n):
        put(nodes, 8 * k, 'II', RAM + 0x300 + 0x40 * k, NODES + 8 * (k + 1) if k + 1 < n else 0)
        regions[RAM + 0x300 + 0x40 * k] = struct.pack('<I', VT) + r.bytes(0x20)
    regions.update({OBJ: obj, SENT: struct.pack('<II', 0, NODES if n else 0), NODES: nodes, VT: vtable({9: 0, 2: 0, 3: 0})})
    return Call(address, [OBJ], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

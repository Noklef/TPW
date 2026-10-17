# The constructors and destructors of src/game/core/list_class_dtors against the originals: the global objects they register with (recorder vtables), lists of 0 to 3 objects with
# deleting-destructor recorders, the allocator the heap region.
# usage: list_class_dtors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, vtable  # noqa: E402

# name: (address, list offset or None for a constructor, size)
CALLS = {'CtorA': (0x5da200, None, 0x70), 'CtorB': (0x5e3c20, None, 0x70), 'DtorA': (0x5da2f0, 4, 0xb0), 'DtorB': (0x5e03b0, 8, 0xb0), 'DtorC': (0x5e3d10, 0x18, 0xb0)}
B = Bench([(a, a + n) for (a, _o, n) in CALLS.values()] + [(0x5d5700, 0x5d5720), (0x5d5750, 0x5d5770), (0x5e3c00, 0x5e3c20)])
LIST, NODES, OBJS, VT, VTL, OTHER, OTHER2 = RAM + 0x400, RAM + 0x500, RAM + 0x1000, RAM + 0x3000, RAM + 0x3100, RAM + 0x200, RAM + 0x300


def case(r):
    name = r.choice(OPS)
    address, off, _n = CALLS[name]
    obj = bytearray(r.bytes(0x40))
    regions = {RAM: obj, LOG: log_region(), HEAP: heap(), 0xf82884: struct.pack('<I', OTHER), OTHER: struct.pack('<I', VT) + r.bytes(0x20), VT: vtable({0: 1, 1: 1, 6: 1, 7: 1}),
               0xf85614: struct.pack('<I', OTHER2), OTHER2: struct.pack('<I', VT) + r.bytes(0x20)}
    if off is not None:
        n = r.choice([0, 1, 2, 3])
        put(obj, off, 'I', LIST)
        lst = bytearray(r.bytes(0x20))
        put(lst, 0, 'I', VTL)
        put(lst, 4, 'II', NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0)
        put(lst, 0xc, 'I', n)
        nodes = bytearray(r.bytes(0x50))
        for k in range(n):
            put(nodes, 0x10 * k, 'III', OBJS + 0x40 * k if r.random() < 0.85 else 0, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
            regions[OBJS + 0x40 * k] = struct.pack('<I', VT) + r.bytes(0x20)
        regions.update({LIST: lst, NODES: nodes, VTL: vtable({0: 1})})
    return Call(address, [RAM], regions, conv='t', ret='a' if off is None else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

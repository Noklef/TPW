# The counted buffer release, byte switch, ring lookup, virtual base deleting destructors and list splice of src/game/core/small8 against
# the originals: any object bytes, indexes and flags, nodes of any shape (the allocator is the heap region).
# usage: small8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Rel': (0x4f75e0, 't', 'v', 0x50), 'Sw': (0x4fe980, 's', 'b', 0x50), 'One': (0x5012b0, 't', 'v', 0x40), 'Ring': (0x50c190, 't', 'a', 0x50),
         'V1': (0x579020, 't', 'a', 0x50), 'V2': (0x5790c0, 't', 'a', 0x50), 'V3': (0x579150, 't', 'a', 0x50), 'Splice': (0x5d0a50, 's', 'a', 0x50)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x5da3c0, 0x5da3c1), (0x50b350, 0x50b360)])
BUFS = RAM + 0x1000
COUNTS = [0, 1, 2, 5, 0x7f, 0xfe, 0xff]


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x400))
    regions = {RAM: obj, HEAP: heap(), 0x785058: r.bytes(0x10)}
    if name == 'Rel':
        data = bytearray(r.bytes(0x40))
        data[2] = r.choice(COUNTS)
        put(obj, 0x80 + 4, 'I', r.choice([0, BUFS + 3, BUFS + 3]))
        regions[BUFS] = data
        return Call(address, [RAM + 0x80], regions, conv='t', ret=ret)
    if name == 'Sw':
        return Call(address, [r.choice([0, 1, 2, 3, 4, 0xffffffff, r.getrandbits(32)])], regions, conv='s', ret=ret)
    if name == 'One':
        return Call(address, [RAM + 0x80], regions, conv='t', ret=ret)
    if name == 'Ring':
        put(obj, 0x80 + 0x88, 'I', r.choice([0, 1, 5, 0x1f, 0x20, 0x21, 100, r.getrandbits(32)]))
        return Call(address, [RAM + 0x80, r.choice([0, 1, 5, 0x1f, 0x20, 0x21, 100, 0xffffffff])], regions, conv='t', ret=ret)
    if name in ('V1', 'V2', 'V3'):
        base = RAM + 0x100  # the object is 8 above; the base's vtable at RAM + 0x40, its offset dword at +4
        put(obj, 0x100, 'I', RAM + 0x40)
        put(obj, 0x44, 'I', r.choice([0, 4, 8, 0xc, 0x10, 0x20, 0x40, 0x80]))
        return Call(address, [base + 8, r.choice([0, 1, 2, 3, 0xffffffff, r.getrandbits(32)])], regions, conv='t', ret=ret)
    nodes = [RAM + 0x10 * i for i in range(5)]  # {next, prev} each
    for a in nodes:
        put(obj, a - RAM, 'II', r.choice(nodes), r.choice(nodes))
    return Call(address, [r.choice(nodes), r.getrandbits(32), r.choice(nodes), r.choice(nodes)], regions, conv='s', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

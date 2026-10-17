# The map insert users and the list freeing destructor of src/game/core/small48 (0x5b1650, 0x5af740, 0x617f40) against the originals: small38.py's trees of the 16-bit key
# map (the object keeps its tree at +0x28 for 0x5af740); lists of 0 to 5 nodes whose blocks are freed.
# usage: small48.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Copy,Add,Dtor)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import small38  # noqa: E402
import stdtree  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

CALLS = {'Copy': 0x5b1650, 'Add': 0x5af740, 'Dtor': 0x617f40}
B = Bench([(0x5b1650, 0x5b1690), (0x5af740, 0x5af7a0), (0x617f40, 0x617ff0), (0x5b1930, 0x5b1a20), (0x5b2ba0, 0x5b2ed0), (0x5b3130, 0x5b31f0), (0x5b3210, 0x5b3230),
           (0x67acb0, 0x67acb1), (0x45b340, 0x45b9a0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)], hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
OBJ = RAM + 0x20000


def case(r):
    name = r.choice(OPS)
    if name == 'Dtor':
        n = r.choice([0, 1, 2, 5])
        obj = bytearray(r.bytes(0x20))
        head, nodes = RAM + 0x100, RAM + 0x200
        ring = [head] + [nodes + 0x20 * k for k in range(n)]
        mem = {head: bytearray(0x20), nodes: bytearray(0x20 * 6)}
        for i, addr in enumerate(ring):
            buf, off = (mem[head], 0) if addr == head else (mem[nodes], addr - nodes)
            put(buf, off, 'II', ring[(i + 1) % len(ring)], ring[i - 1])
            put(buf, off + 8, 'I', r.choice([HEAP + 0x100, HEAP + 0x200, 0, nodes + 0x10]))
        put(obj, 8, 'II', head, n)
        return Call(CALLS[name], [RAM], {RAM: obj, HEAP: heap(), **mem}, conv='t', ret='v')
    saved = small38.OPS
    small38.OPS = ['G']
    try:
        c = small38.case(r)
    finally:
        small38.OPS = saved
    if name == 'Copy':
        c.address, c.ret = CALLS[name], 'a'
        return c
    # an object with the tree at +0x28
    tree = bytearray(c.regions.pop(stdtree.TREE))
    obj = bytearray(r.bytes(0x60))
    obj[0x28:0x28 + len(tree)] = tree
    c.regions[OBJ] = obj
    key = struct.unpack_from('<H', c.regions[RAM + 0x900], 0)[0]
    return Call(CALLS[name], [OBJ, key | r.choice([0, 0, 0x10000]), r.choice([0, 1, 7, 0xffffffff])], c.regions, conv='t', ret='b')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

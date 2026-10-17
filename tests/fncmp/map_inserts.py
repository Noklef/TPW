# The map insert of src/game/core/map_inserts (0x47cd00) against the original: valid red-black trees (stdtree.py's) of 0 to 11 nodes with three-dword values, keys that are new or
# already there, with the multi flag set or clear.
# usage: map_inserts.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import stdtree  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap  # noqa: E402

B = Bench([(0x47cd00, 0x47cde0), (0x5b1930, 0x5b1a20), (0x5b2ba0, 0x5b2ed0), (0x5b3130, 0x5b31f0), (0x5b3210, 0x5b3230), (0x47d140, 0x47d470), (0x47d4b0, 0x47d570), (0x67acb0, 0x67acb1), (0x45b340, 0x45b9a0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
TREE, HEAD, NIL, NODES = stdtree.TREE, stdtree.HEAD, stdtree.NIL, stdtree.NODES
KINDS = {'A': (0x47cd00, 0x7b4a54, 0x18), 'G': (0x5b1930, 0xf7c970, 0x10)}


def case(r):
    name = r.choice(OPS)
    address, nil_at, COLOUR = KINDS[name]
    n = r.choice([0, 1, 2, 3, 5, 8, 11])
    nodes, root = stdtree.build(r, n)
    keys = sorted(nodes)
    new = r.choice(keys) if keys and r.random() < 0.4 else r.choice([k for k in range(100) if k not in nodes])
    addr = {k: NODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: NIL if k is None else addr[k]  # noqa: E731
    blob = bytearray(0x4000)
    for k in keys:
        base = addr[k] - NODES
        struct.pack_into('<III', blob, base, ptr(nodes[k][0]), HEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        struct.pack_into('<I', blob, base + 0xc, k)
        struct.pack_into('<I', blob, base + COLOUR, nodes[k][3])
    head = struct.pack('<III', addr[keys[0]] if keys else HEAD, addr[root] if keys else NIL, addr[keys[-1]] if keys else HEAD) + bytes(0x20)
    nil = bytearray(0x40)
    struct.pack_into('<I', nil, COLOUR, stdtree.BLACK)
    tree = bytearray(0x20)
    struct.pack_into('<I', tree, 4, HEAD)
    tree[8] = r.choice([0, 0, 1])
    struct.pack_into('<I', tree, 0xc, n)
    value = struct.pack('<III', new, r.getrandbits(32), r.getrandbits(32))
    regions = {TREE: tree, HEAD: head, NIL: nil, NODES: blob, nil_at: struct.pack('<I', NIL), RAM + 0x800: bytes(0x10), RAM + 0x900: value, HEAP: heap()}
    return Call(address, [TREE, RAM + 0x800, RAM + 0x900], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The map finds of src/game/core/small26 against the originals: valid red-black trees of stdtree.py (keys 0 to 99; the searched key present, between or outside), the map of
# 0x5af780 inside an object.
# usage: small26.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import stdtree  # noqa: E402
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, size, nil global, colour offset, key width, returns)
CALLS = {'A': (0x410610, 0x60, 0x785de4, 0x10, 2, 'a'), 'B': (0x47c6a0, 0x60, 0x7b4a54, 0x18, 4, 'a'), 'C': (0x5e76c0, 0x60, 0xf7c970, 0x10, 2, 'a'),
         'D': (0x5af780, 0x70, 0xf7c970, 0x10, 2, 'b')}
B = Bench([(a, a + n) for (a, n, *_r) in CALLS.values()] + [(0x410ac0, 0x410b20), (0x47d460, 0x47d4b0), (0x5b30b0, 0x5b3110), (0x5da3c0, 0x5da3c1)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})


def case(r):
    name = r.choice(OPS)
    address, _n, nil_at, colour, width, ret = CALLS[name]
    nodes, root = stdtree.build(r, r.choice([0, 1, 2, 3, 5, 8, 11]) or 1)
    keys = sorted(nodes)
    addr = {k: stdtree.NODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: stdtree.NIL if k is None else addr[k]  # noqa: E731
    blob = bytearray(0x40 * len(keys))
    for k in keys:
        base = addr[k] - stdtree.NODES
        struct.pack_into('<III', blob, base, ptr(nodes[k][0]), stdtree.HEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        struct.pack_into('<II', blob, base + 0xc, k, 0)
        struct.pack_into('<I', blob, base + colour, nodes[k][3])
    nil = bytearray(0x40)
    struct.pack_into('<I', nil, colour, 1)
    head = struct.pack('<III', addr[keys[0]], addr[root], addr[keys[-1]]) + bytes(0x20)
    key = r.choice(keys + [0, 3, 50, 99, 100, 0xffff, 0x10000 if width == 4 else 7])
    regions = {stdtree.HEAD: head, stdtree.NIL: nil, stdtree.NODES: blob, nil_at: struct.pack('<I', stdtree.NIL), RAM + 0x300: bytes(0x10)}
    if name == 'D':
        obj = bytearray(r.bytes(0x40))
        struct.pack_into('<II', obj, 0x28, 0, stdtree.HEAD)
        regions[RAM] = obj
        return Call(address, [RAM, key | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0)], regions, conv='t', ret=ret)
    tree = bytearray(0x20)
    struct.pack_into('<II', tree, 4, stdtree.HEAD, 0)
    regions[RAM] = tree
    regions[RAM + 0x400] = struct.pack('<I', key) + bytes(0xc)
    return Call(address, [RAM, RAM + 0x300, RAM + 0x400], regions, conv='t', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

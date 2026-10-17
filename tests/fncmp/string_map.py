# The string map walks of src/game/core/string_map (0x5b2f40, 0x5b0c40) against the originals: valid red-black trees (stdtree.py's) of 0 to 12 nodes whose keys are game strings
# (gamestring.py's: a small alphabet so that equal, prefix and ordered keys come up) and whose mapped pointers lead to objects with the byte at +0xb9 set or clear.
# usage: string_map.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Lower,All)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
import stdtree  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap  # noqa: E402

CALLS = {'Lower': 0x5b2f40, 'All': 0x5b0c40, 'Find': 0x5b20c0}
B = Bench([(0x5b20c0, 0x5b20e0), (0x5b2f40, 0x5b2fd0), (0x5b0c40, 0x5b0ca0), (0x5b2480, 0x5b2530), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
OBJS = RAM + 0x30000


def case(r):
    name = r.choice(OPS)
    n = r.choice([0, 1, 2, 3, 5, 8, 12]) if name in ('Lower', 'Find') else r.choice([1, 2, 3, 4, 6])
    nodes, root = stdtree.build(r, n) if n else ({}, None)
    keys = sorted(nodes)
    addr = {k: stdtree.NODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: stdtree.NIL if k is None else addr[k]  # noqa: E731
    blob = bytearray(0x40 * max(n, 1))
    extra = {}
    objs = bytearray(0x100 * max(n, 1))
    for i, k in enumerate(keys):
        base = addr[k] - stdtree.NODES
        struct.pack_into('<III', blob, base, ptr(nodes[k][0]), stdtree.HEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        gamestring.string(r, 0, RAM + 0x4000 + 0x40 * i, RAM + 0x6000 + 0x80 * i, extra)
        blob[base + 0xc:base + 0x20] = extra[RAM + 0x4000 + 0x40 * i][:0x14]
        struct.pack_into('<I', blob, base + 0x20, OBJS + 0x100 * i)
        struct.pack_into('<I', blob, base + 0x24, nodes[k][3])
        objs[0x100 * i + 0xb9] = r.choice([0, 1, 1, 1, 0x80])
    head = struct.pack('<III', addr[keys[0]] if keys else stdtree.HEAD, addr[root] if keys else stdtree.NIL, addr[keys[-1]] if keys else stdtree.HEAD) + bytes(0x20)
    nil = bytearray(0x40)
    struct.pack_into('<I', nil, 0x24, 1)
    tree = bytearray(0x40)
    struct.pack_into('<II', tree, 4, stdtree.HEAD, 0)
    regions = {stdtree.TREE: tree, stdtree.HEAD: head, stdtree.NIL: nil, stdtree.NODES: blob, 0xf7c968: struct.pack('<I', stdtree.NIL), OBJS: objs, HEAP: heap(), **extra}
    if name in ('Lower', 'Find'):
        gamestring.string(r, 0, RAM + 0x800, RAM + 0x900, regions)
        if name == 'Find':
            regions[RAM + 0xa00] = r.bytes(8)
            return Call(CALLS[name], [stdtree.TREE, RAM + 0xa00, RAM + 0x800], regions, conv='t', ret='a')
        return Call(CALLS[name], [stdtree.TREE, RAM + 0x800], regions, conv='t', ret='a')
    struct.pack_into('<I', tree, 0x3c, stdtree.HEAD)
    return Call(CALLS[name], [stdtree.TREE], regions, conv='t', ret='b')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

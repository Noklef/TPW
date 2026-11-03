# The string map's insert (src/game/core/stdtree: 0x5b1620, 0x5b16b0, 0x5b2170, 0x5b31f0, 0x5b3230) against the originals: string_map.py's trees (0 to 11 nodes, gamestring.py's keys of a small
# alphabet, so that equal, prefix and ordered keys come up), the multi flag set or clear, the value's key new or there. _Insert with x nil or not and y the head, the rightmost or any node;
# the value's copy on its own, and its construct at null.
# usage: string_map_insert.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Copy,Insert,Raw,Ctor,Construct)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
import stdtree  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

CALLS = {'Copy': 0x5b1620, 'Insert': 0x5b16b0, 'Raw': 0x5b2170, 'Ctor': 0x5b3230, 'Construct': 0x5b31f0}
B = Bench([(0x5b1620, 0x5b1650), (0x5b16b0, 0x5b17c0), (0x5b2170, 0x5b2480), (0x5b2ff0, 0x5b30b0), (0x5b31f0, 0x5b3203), (0x5b3230, 0x5b324b), (0x45b340, 0x45b9a0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
VALUE, OUT, BYTES = RAM + 0x800, RAM + 0xa00, RAM + 0x900


def case(r):
    name = r.deal(OPS)
    regions = {HEAP: heap()}
    gamestring.string(r, 0, VALUE, BYTES, regions)
    regions[VALUE] = bytes(regions[VALUE][:0x14]) + r.bytes(4)
    regions[OUT] = r.bytes(0x18)
    if name == 'Ctor':
        return Call(0x5b3230, [OUT, VALUE], regions, conv='t', ret='a')
    if name == 'Construct':
        return Call(0x5b31f0, [r.choice([OUT, OUT, OUT, 0]), VALUE], regions, conv='c', ret='v')
    n = r.choice([0, 1, 2, 3, 5, 8, 11])
    nodes, root = stdtree.build(r, n) if n else ({}, None)
    keys = sorted(nodes)
    addr = {k: stdtree.NODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: stdtree.NIL if k is None else addr[k]  # noqa: E731
    blob = bytearray(0x40 * max(n, 1))
    for i, k in enumerate(keys):
        base = addr[k] - stdtree.NODES
        struct.pack_into('<III', blob, base, ptr(nodes[k][0]), stdtree.HEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        gamestring.string(r, 0, RAM + 0x4000 + 0x40 * i, RAM + 0x6000 + 0x80 * i, regions)
        blob[base + 0xc:base + 0x20] = regions[RAM + 0x4000 + 0x40 * i][:0x14]
        struct.pack_into('<II', blob, base + 0x20, r.getrandbits(32), nodes[k][3])
    head = struct.pack('<III', addr[keys[0]] if keys else stdtree.HEAD, addr[root] if keys else stdtree.NIL, addr[keys[-1]] if keys else stdtree.HEAD) + bytes(0x20)
    tree = bytearray(0x20)
    put(tree, 4, 'I', stdtree.HEAD)
    tree[8] = r.choice([0, 0, 0, 1])
    put(tree, 0xc, 'I', n)
    regions.update({stdtree.TREE: tree, stdtree.HEAD: head, stdtree.NIL: put(bytearray(0x40), 0x24, 'I', stdtree.BLACK), stdtree.NODES: blob, 0xf7c968: struct.pack('<I', stdtree.NIL)})
    if name == 'Raw':
        y = stdtree.HEAD if not keys else r.choice([stdtree.HEAD, addr[keys[-1]], addr[r.choice(keys)]])
        return Call(0x5b2170, [stdtree.TREE, OUT, r.choice([stdtree.NIL, stdtree.NIL, y]), y, VALUE], regions, conv='t', ret='a')
    return Call(CALLS[name], [stdtree.TREE, OUT, VALUE], regions, conv='t', ret='a' if name == 'Copy' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The message centre's slot destructors (src/game/world/messagecentre: 0x40f970 one slot, 0x409250 the 29 of them) against the originals: slots of a dword and a std::map of
# tree B (core/stdtree) whose trees are stdtree.py's valid red-black trees of 0 to 4 nodes, the map counter 0x785de0 from 1 up (the last map out frees the nil node 0x785de4).
# usage: messagecentre.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Slot,Slots)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import stdtree  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap  # noqa: E402

B = Bench([(0x409250, 0x409260), (0x40f970, 0x40f9e0), (0x40ff80, 0x4104e0), (0x4104b0, 0x410610), (0x410670, 0x410720), (0x410720, 0x4107a0), (0x67acb0, 0x67acb1),
           (0x67afa0, 0x67b0d0)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
SLOTS, HEADS, NIL, NODES = RAM, RAM + 0x400, RAM + 0x800, RAM + 0x1000
COUNTER = 0x785de0   # (the nil node's pointer follows at 0x785de4)
SLOT_COUNT, MAX_NODES = 0x1d, 4


def slot(r, k, slots, heads, nodes_blob):
    """Slot k: a random dword, then the map (allocator bytes, head, multi flag, size) whose head (in HEADS) and nodes (in NODES, 0x20 apart, colour at +0x10) make a tree."""
    n = r.choice([0, 0, 1, 2, 3, 4])
    nodes, root = stdtree.build(r, n)
    keys = sorted(nodes)
    head = HEADS + 0x20 * k
    addr = {key: NODES + 0x20 * (MAX_NODES * k + i) for i, key in enumerate(keys)}

    def ptr(key):
        return NIL if key is None else addr[key]

    for key in keys:
        base = addr[key] - NODES
        struct.pack_into('<4I', nodes_blob, base, ptr(nodes[key][0]), head if nodes[key][1] is None else addr[nodes[key][1]], ptr(nodes[key][2]), key)
        struct.pack_into('<I', nodes_blob, base + 0x10, nodes[key][3])
    if n:
        struct.pack_into('<3I', heads, 0x20 * k, addr[keys[0]], addr[root], addr[keys[-1]])
    else:
        struct.pack_into('<3I', heads, 0x20 * k, head, NIL, head)
    struct.pack_into('<II', heads, 0x20 * k + 0xc, r.getrandbits(32), stdtree.RED)
    struct.pack_into('<5I', slots, 0x14 * k, r.getrandbits(32), r.getrandbits(32), head, r.getrandbits(32) & 0xff, n)


def case(r):
    op = r.choice(OPS)
    count = SLOT_COUNT if op == 'Slots' else 1
    slots, heads, nodes_blob = bytearray(0x14 * count), bytearray(0x20 * count), bytearray(r.bytes(0x20 * MAX_NODES * count))
    for k in range(count):
        slot(r, k, slots, heads, nodes_blob)
    nil = bytearray(0x20)
    struct.pack_into('<I', nil, 0x10, stdtree.BLACK)
    counter = r.choice([1, 2, count, count + 1, 100]) if op == 'Slots' else r.choice([1, 1, 2, 7])
    regions = {SLOTS: slots, HEADS: heads, NIL: nil, NODES: nodes_blob, COUNTER: struct.pack('<II', counter, NIL), HEAP: heap()}
    return Call(0x409250 if op == 'Slots' else 0x40f970, [SLOTS], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', 'Slot,Slots').split(',') if k]

if __name__ == '__main__':
    B.main(case)

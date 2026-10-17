# The list members of src/game/core/std_lists against the originals: lists of 0 to 6 counted strings with runs of equal ones (the count bytes at the edges, null
# characters for empty strings, lengths that differ); sorted lists of dwords of 0 to 6 elements merged (and a list merged with itself).
# usage: std_lists.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Unique,Merge)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

CALLS = {'Unique': 0x5ce080, 'Merge': 0x5d0590}
B = Bench([(0x5ce080, 0x5ce190), (0x5d0590, 0x5d0690), (0x67acb0, 0x67acb1)])
LIST, HEAD, NODES, TEXT, LIST2, HEAD2, NODES2 = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x4000, RAM + 0x200, RAM + 0x300, RAM + 0x2000


def circular(head, nodes, count, size_of=0x20):
    """next/prev words for a head and count nodes at nodes (size_of apart): returns (head bytes, node addresses)."""
    addrs = [nodes + size_of * k for k in range(count)]
    ring = [head] + addrs
    return ring


def case(r):
    name = r.choice(OPS)
    if name == 'Merge':
        n1, n2 = r.choice([0, 1, 2, 4, 6]), r.choice([0, 1, 3, 6])
        a, b = sorted(r.choice(range(0, 12)) for _ in range(n1)), sorted(r.choice(range(0, 12)) for _ in range(n2))
        regions = {LIST: bytearray(r.bytes(0x10)), LIST2: bytearray(r.bytes(0x10)), HEAD: bytearray(r.bytes(0x10)), HEAD2: bytearray(r.bytes(0x10)), NODES: bytearray(0x20 * 8), NODES2: bytearray(0x20 * 8)}
        for lst, head, nodes, vals in ((LIST, HEAD, NODES, a), (LIST2, HEAD2, NODES2, b)):
            ring = [head] + [nodes + 0x20 * k for k in range(len(vals))]
            for i, addr in enumerate(ring):
                nxt, prv = ring[(i + 1) % len(ring)], ring[i - 1]
                buf = regions[head] if addr == head else regions[nodes]
                off = 0 if addr == head else addr - nodes
                put(buf, off, 'II', nxt, prv)
                if addr != head:
                    put(buf, off + 8, 'I', vals[i - 1])
            put(regions[lst], 4, 'II', head, len(vals))
        other = LIST if r.random() < 0.08 else LIST2
        return Call(CALLS[name], [LIST, other], regions, conv='t', ret='v')
    n = r.choice([0, 1, 2, 3, 4, 6])
    words = [b'', b'a', b'a', b'ab', b'b', b'abc']
    regions = {LIST: bytearray(r.bytes(0x10)), HEAD: bytearray(r.bytes(0x10)), NODES: bytearray(r.bytes(0x20 * 8)), TEXT: bytearray(r.bytes(0x400)), HEAP: heap()}
    ring = [HEAD] + [NODES + 0x20 * k for k in range(n)]
    text_at = 0
    for i, addr in enumerate(ring):
        nxt, prv = ring[(i + 1) % len(ring)], ring[i - 1]
        buf = regions[HEAD] if addr == HEAD else regions[NODES]
        off = 0 if addr == HEAD else addr - NODES
        put(buf, off, 'II', nxt, prv)
        if addr != HEAD:
            w = r.choice(words)
            if w == b'' and r.random() < 0.7:
                put(buf, off + 0xc, 'III', 0, 0, 0)
            else:
                text = TEXT + 0x20 * (i - 1) + 1
                regions[TEXT][0x20 * (i - 1)] = r.choice([0, 1, 2, 0xfe, 0xff])
                regions[TEXT][0x20 * (i - 1) + 1:0x20 * (i - 1) + 1 + len(w)] = w
                put(buf, off + 0xc, 'III', text, len(w), len(w))
    put(regions[LIST], 4, 'II', HEAD, n)
    return Call(CALLS[name], [LIST], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The methods that forward to a member's (src/game/core/forwarders: 0x5d5da0, 0x5d8a70, 0x5da170,
# 0x5da440, 0x5dd490, 0x5dd4a0, 0x5f42d0, 0x618600) against the originals: the owner's member lists with 0 to 2
# nodes (the allocator is the heap region), items of any value; the object that takes four dwords
# from a source or none.
# usage: forwarders.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x5d5da0, 0x5d5db0), (0x5d8a70, 0x5d8a80), (0x5da170, 0x5da180), (0x5da440, 0x5da450), (0x5dd490, 0x5dd4a0),
           (0x5dd4a0, 0x5dd4b0), (0x618600, 0x618610), (0x5e42d0, 0x5e431c), (0x6344c0, 0x6344e4), (0x5f42d0, 0x5f42e3)])
CALLS = {0x5d5da0: 0x18, 0x5d8a70: 0xc, 0x5da170: 0x58, 0x5da440: 0x4, 0x5dd490: 0x8, 0x5dd4a0: 0x6c, 0x5f42d0: 0x20080}
LIST, NODES = RAM + 0x800, RAM + 0x900


def case(r):
    owner = bytearray(r.bytes(0x100))
    regions = {RAM: owner, HEAP: heap()}
    if r.random() < 0.15:
        put(owner, 0x64, 'I', RAM + 0x200)
        regions[RAM + 0x200] = r.bytes(0x60)
        regions[RAM + 0x300] = r.bytes(0x20)
        return Call(0x618600, [RAM, r.choice([0, RAM + 0x300, RAM + 0x304])], regions, conv='t', ret='a')
    address = r.choice(list(CALLS))
    if CALLS[address] < 0x100:
        put(owner, CALLS[address], 'I', LIST)
    else:   # (past the owner's bytes; +0x18 another list, so reading the wrong member shows)
        regions[RAM + CALLS[address]] = struct.pack('<I', LIST)
        put(owner, 0x18, 'I', RAM + 0x400)
        regions[RAM + 0x400] = bytes(0x10)
    n = r.choice([0, 0, 1, 2])
    lst = bytearray(r.bytes(0x10))  # {vtable, head, tail, count}
    nodes = bytearray(0x40)
    for i in range(n):  # nodes {item, next, prev}
        put(nodes, 0xc * i, 'III', r.getrandbits(32), NODES + 0xc * (i + 1) if i + 1 < n else 0, NODES + 0xc * (i - 1) if i else 0)
    put(lst, 4, 'II', NODES if n else 0, NODES + 0xc * (n - 1) if n else 0)
    put(lst, 0xc, 'I', n)
    regions.update({LIST: lst, NODES: nodes})
    return Call(address, [RAM, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

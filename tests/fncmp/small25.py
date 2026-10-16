# The small functions of src/game/core/small25 against the originals: the table clear, the map iterator post-increments (valid red-black trees of stdtree.py), the
# constructor wrappers, the list add that records the item (small15.py's list), the class constructor and its static initialiser, and the string copies (gamestring.py's
# strings: arrays of up to four, the copy destination arrays, null destinations).
# usage: small25.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
import stdtree  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Clear': (0x56a750, 'c', 'v', 0x20), 'PostC': (0x4f0370, 't', 'a', 0x20), 'PostE': (0x4f0460, 't', 'a', 0x20), 'InitA': (0x4f0880, 's', 'a', 0x20),
         'InitB': (0x4f08a0, 's', 'a', 0x20), 'Add': (0x421680, 't', 'a', 0x10), 'Ctor': (0x5f9f80, 't', 'a', 0x20), 'Global': (0x5f9f70, 'c', 'a', 0x30),
         'Range': (0x5ed490, 's', 'a', 0x40), 'Fill': (0x5ed4d0, 's', 'v', 0x30), 'Place': (0x5ed560, 'c', 'v', 0x20)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x4f44d0, 0x4f4580), (0x4f5430, 0x4f54e0), (0x4f1520, 0x4f1560), (0x421590, 0x421620), (0x5da3c0, 0x5da3c1),
          (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)], hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
SRC, DST = RAM + 0x2000, RAM + 0x4000


def strings(r, regions, base, n):
    """n game strings from base (0x14 bytes each), their blocks from the arena below; returns nothing."""
    objs = bytearray(0x14 * max(n, 1))
    for i in range(n):
        gamestring.string(r, 0, RAM + 0x6000 + 0x40 * i + (0 if base == SRC else 0x400), RAM + 0x8000 + 0x80 * i + (0 if base == SRC else 0x800), regions)
        objs[0x14 * i:0x14 * (i + 1)] = regions.pop(RAM + 0x6000 + 0x40 * i + (0 if base == SRC else 0x400))[:0x14]
    regions[base] = objs


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    regions = {RAM: bytearray(r.bytes(0x100)), HEAP: heap()}
    if name == 'Clear':
        regions[0x879ad0] = r.bytes(0x400)
        regions[0x879cd0] = r.bytes(0x10)
        return Call(address, [], regions, conv=conv, ret=ret)
    if name in ('PostC', 'PostE'):
        nodes, root = stdtree.build(r, r.choice([1, 2, 3, 5, 8]))
        keys = sorted(nodes)
        colour = 0x20 if name == 'PostC' else 0x1c
        nil_at = 0x7ced2c if name == 'PostC' else 0x7ced34
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
        regions.update({stdtree.HEAD: struct.pack('<III', addr[keys[0]], addr[root], addr[keys[-1]]) + bytes(0x20), stdtree.NIL: nil, stdtree.NODES: blob,
                        nil_at: struct.pack('<I', stdtree.NIL), RAM + 0x200: struct.pack('<I', addr[r.choice(keys)]), RAM + 0x300: bytes(0x10)})
        return Call(address, [RAM + 0x200, RAM + 0x300, r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name in ('InitA', 'InitB'):
        regions[0x7099f0] = r.bytes(0x10)
        regions[0xfb34b0] = r.bytes(0x10)
        return Call(address, [RAM] + [r.getrandbits(32) for _ in range(3 if name == 'InitA' else 7)], regions, conv=conv, ret=ret)
    if name == 'Add':
        node = RAM + 0x900
        lst = bytearray(r.bytes(0x10))
        empty = r.random() < 0.4
        put(regions[RAM], 0, 'I', RAM + 0x800 if r.random() < 0.6 else 0)
        put(lst, 4, 'II', 0 if empty else node, 0 if empty else node)
        put(lst, 0xc, 'I', r.choice([0, 1, 5]))
        regions.update({RAM + 0x800: lst, RAM + 0x1000: r.bytes(0x60), node: r.bytes(0x20)})
        return Call(address, [RAM, RAM + 0x1000], regions, conv=conv, ret=ret)
    if name == 'Ctor':
        return Call(address, [RAM], regions, conv=conv, ret=ret)
    if name == 'Global':
        regions[0xfa78e8] = r.bytes(0x20)
        return Call(address, [], regions, conv=conv, ret=ret)
    n = r.choice([0, 1, 2, 3, 4])
    if name == 'Range':
        strings(r, regions, SRC, n)
        regions[DST] = r.bytes(0x14 * 5)
        return Call(address, [SRC, SRC + 0x14 * n, DST if r.random() < 0.85 else 0], regions, conv=conv, ret=ret)
    if name == 'Fill':
        strings(r, regions, SRC, 1)
        regions[DST] = r.bytes(0x14 * 5)
        return Call(address, [DST if r.random() < 0.85 else 0, n, SRC], regions, conv=conv, ret=ret)
    strings(r, regions, SRC, 1)
    regions[DST] = r.bytes(0x14 * 2)
    return Call(address, [DST if r.random() < 0.85 else 0, SRC], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

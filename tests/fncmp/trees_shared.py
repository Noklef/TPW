# The list pop, name lookup, shared-array releases, tree lower bounds and global releases of src/game/core/trees_shared against the originals: lists of 0 to 3 nodes, small
# trees with keys around the searched one, shared records with use counts at the edge (the arrays have no elements to destroy), the COM members of vobj3.py, the strings
# of the game (the allocator is the heap region; the STL lock is left out).
# usage: trees_shared.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import vobj3  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, global_heap, heap, put  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Pop': (0x5f46a0, 't', 'a', 0x40), 'Names': (0x5bd230, 't', 'a', 0x40), 'Sh66': (0x5b9fd0, 't', 'v', 0x60), 'Sh10': (0x5ba290, 't', 'v', 0x60),
         'T16a': (0x410ac0, 't', 'a', 0x60), 'T32': (0x47d460, 't', 'a', 0x50), 'T16b': (0x5b30b0, 't', 'a', 0x60), 'G879': (0x5633a0, 'c', 'a', 0x40),
         'Mode1': (0x564140, 't', 'a', 0x20), 'G816': (0x51fd20, 'c', 'v', 0x60), 'Str5': (0x5c4ce0, 't', 'a', 0x70)}


def lock(e):  # the STL lock (0x691de0, thiscall, returns its argument; 0x691ea0 releases)
    e.r['EAX'] = e.r['ECX']


B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x5640a0, 0x5640a0 + 0xa0), (0x51f660, 0x51f680), (0x5da3c0, 0x5da3c1), (0x5f5680, 0x5f5690), (0x67afa0, 0x67b060),
          (0x67df40, 0x67e040), (0x67faf0, 0x67fb20), (0x682720, 0x682780), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)],
          hooks={0x691de0: lock, 0x691ea0: lambda e: None})
NAMES = [b'Sloth', b'sloth', b'SLOTH', b'Slot', b'', b'b']
TREES = {'T16a': (0x785de4, 2), 'T32': (0x7b4a54, 4), 'T16b': (0xf7c970, 2)}


def bst(r, regions, obj, sentinel_at):
    """Head node at RAM+0x1e00 whose +4 is the root of a small binary tree (nodes of 0x10 bytes at RAM+0x1000, the sentinel at RAM+0x1f00)."""
    sentinel = RAM + 0x1f00
    keys = [r.choice([0, 1, 2, 5, 7, 9, 0x80, 0xff, 0x8000, 0xffff]) for _ in range(r.choice([0, 1, 2, 3, 5]))]
    nodes = {}
    root = sentinel
    for k, key in enumerate(keys):
        at = RAM + 0x1000 + 0x10 * k
        nodes[at] = [sentinel, 0, sentinel, key]
        cur, parent = root, None
        while cur != sentinel:
            parent = cur
            cur = nodes[cur][0] if key < nodes[cur][3] else nodes[cur][2]
        if parent is None:
            root = at
        elif key < nodes[parent][3]:
            nodes[parent][0] = at
        else:
            nodes[parent][2] = at
        nodes[at][1] = parent or RAM + 0x1e00
    data = bytearray(0x100)
    for at, (left, parent, right, key) in nodes.items():
        put(data, at - RAM - 0x1000, 'IIII', left, parent, right, key)
    regions[RAM + 0x1000] = data
    regions[RAM + 0x1e00] = struct.pack('<IIII', sentinel, root, sentinel, 0)
    regions[RAM + 0x1f00] = bytes(0x10)
    regions[sentinel_at] = struct.pack('<I', sentinel) + bytes(0xc)
    put(obj, 4, 'I', RAM + 0x1e00)
    return keys


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    regions = {RAM: obj, HEAP: heap(), **global_heap()}
    if name == 'Pop':
        n = r.choice([0, 1, 2, 3])
        put(obj, 4, 'II', RAM + 0x1000 if n else 0, RAM + 0x1000 + 0x10 * (n - 1) if n else 0)
        put(obj, 0xc, 'I', r.choice([n, n, 0, 7]))
        nodes = bytearray(r.bytes(0x60))
        for k in range(n):
            put(nodes, 0x10 * k, 'III', RAM + 0x2000 + 0x40 * k, RAM + 0x1000 + 0x10 * (k + 1) if k + 1 < n else 0, RAM + 0x1000 + 0x10 * (k - 1) if k else 0)
        regions[RAM + 0x1000] = nodes
        return Call(address, [RAM], regions, conv=conv, ret=ret)
    if name == 'Names':
        n = r.choice([0, 1, 2, 3])
        wanted = r.choice(NAMES)
        put(obj, 0x2c, 'I', RAM + 0x1000)
        lst = bytearray(r.bytes(0x20))
        put(lst, 4, 'I', RAM + 0x1100 if n else 0)
        nodes = bytearray(r.bytes(0x60))
        for k in range(n):
            entry = RAM + 0x2000 + 0x40 * k
            put(nodes, 0x10 * k, 'II', entry, RAM + 0x1100 + 0x10 * (k + 1) if k + 1 < n else 0)
            e = bytearray(r.bytes(0x40))
            put(e, 0x14, 'I', RAM + 0x2800 + 0x20 * k)
            regions[entry] = e
            s = bytearray(wanted if r.random() < 0.5 else r.choice(NAMES))
            for i in range(len(s)):
                if r.random() < 0.3:
                    s[i] ^= 0x20
            regions[RAM + 0x2800 + 0x20 * k] = bytes(s) + bytes(0x10)
        regions.update({RAM + 0x1000: lst, RAM + 0x1100: nodes, RAM + 0x2c00: wanted + bytes(0x10)})
        return Call(address, [RAM, RAM + 0x2c00], regions, conv=conv, ret=ret)
    if name in ('Sh66', 'Sh10'):
        put(obj, 0x30, 'I', RAM + 0x200 if r.random() < 0.85 else 0)
        rec = bytearray(r.bytes(0x20))
        put(rec, 4, 'I', RAM + 0x304 if r.random() < 0.7 else 0)
        put(rec, 0xc, 'i', r.choice([1, 1, 2, 0, -1]))
        regions[RAM + 0x200] = rec
        regions[RAM + 0x300] = struct.pack('<I', 0) + r.bytes(0x20)
        return Call(address, [RAM], regions, conv=conv, ret=ret)
    if name in TREES:
        sentinel_at, width = TREES[name]
        keys = bst(r, regions, obj, sentinel_at)
        key = r.choice(keys + [0, 3, 6, 0x7fff, 0x8001, 0xfffe]) & (0xffff if width == 2 else 0xffffffff)
        regions[RAM + 0x400] = struct.pack('<I', key) + bytes(0xc)
        return Call(address, [RAM, RAM + 0x400], regions, conv=conv, ret=ret)
    if name in ('G879', 'Mode1'):  # an object with the COM members of vobj3.py
        saved = vobj3.OPS, vobj3.CALLS['Seven']
        vobj3.OPS, vobj3.CALLS['Seven'] = ['Seven'], (address, conv, ret, 0xa0)
        try:
            c = vobj3.case(r)
        finally:
            vobj3.OPS, vobj3.CALLS['Seven'] = saved
        c.regions.update({HEAP: heap(), **global_heap()})
        if name == 'G879':
            c.args = []
            c.regions[0x879a24] = struct.pack('<I', RAM if r.random() < 0.85 else 0)
        else:
            c.regions[0x8bd524] = struct.pack('<I', r.choice([0, 1, 1, 2]))
        return c
    if name == 'G816':
        for g in (0x816d40, 0x80ced0, 0x80cef8, 0x816d38, 0x80436c):
            regions[g & ~0xf] = bytearray(r.bytes(0x10))
        regions[0x816d40] = struct.pack('<I', r.choice([0, 1, 5])) + bytes(0xc)
        regions[0x80cef8 & ~0xf][8:12] = struct.pack('<I', r.choice([0, HEAP + 0x40]))
        regions[0x816d38 & ~0xf][8:12] = struct.pack('<I', r.choice([0, HEAP + 0x80]))
        return Call(address, [], regions, conv=conv, ret=ret)
    return Call(address, [RAM], regions, conv=conv, ret=ret)  # Str5


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The red-black tree members of src/game/core/stdtree (iterator increment and erase for seven instantiations of the game's std::map / std::set) against the originals:
# valid red-black trees of 1 to 12 nodes (built with the usual insertion), every node erased in turn across the cases, the iterator increment from every node; for B the
# range erase (0x4104b0: the whole tree, a tail, a stretch or nothing, the size now and then 0) and _Erase (0x410720) from the root, a node or nil.
# usage: stdtree.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
import stdstring  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap  # noqa: E402

# name: (Inc address, Erase address, nil global, colour offset)
TREES = {'A': (0x47d570, 0x47c010, 0x7b4a54, 0x18), 'B': (0x410670, 0x40ff80, 0x785de4, 0x10), 'C': (0x4f44d0, 0x4f3b10, 0x7ced2c, 0x20),
         'D': (0x4f4f20, 0x4f4580, 0x7ced24, 0x1c), 'E': (0x4f5430, 0x4f2d50, 0x7ced34, 0x1c), 'F': (0x5b2480, 0x5b1b80, 0xf7c968, 0x24),
         'G': (0x5b2e90, 0x5b2530, 0xf7c970, 0x10)}
INSERT = {'A': 0x47d140, 'G': 0x5b2ba0}
DEC = {'A': 0x47d4b0, 'C': 0x4f6340, 'D': 0x4f64f0, 'E': 0x4f6680, 'F': 0x5b2ff0, 'G': 0x5b3130}
RANGE = {'B': (0x4104b0, 0x410720)}   # erase(first, last), _Erase
CTOR = {'A': (0x47ce70, 0x7b4a50), 'C': (0x4f4120, 0x7ced30), 'D': (0x4f4b70, 0x7ced28), 'E': (0x4f5080, 0x7ced38), 'G': (0x5b2af0, 0xf7c96c)}
B = Bench([(a, a + 0x330) for a in INSERT.values()] + [(0x5b3210, 0x5b3230)] + [(a, a + 0xc0) for a in DEC.values()] + [(a, a + 0xb0) for (a, _c) in CTOR.values()] + [(a, a + 0xb0) for (a, _e, _n, _c) in TREES.values()] + [(e, e + 0x560) for (_a, e, _n, _c) in TREES.values()] + [(0x4104b0, 0x410610), (0x410720, 0x4107a0), (0x67acb0, 0x67acb1), (0x45b340, 0x45b9a0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
TREE, HEAD, NIL, NODES = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x10000
RED, BLACK = 0, 1


def build(r, n):
    """A red-black tree of n distinct keys as dicts {key: [left, parent, right, colour]} (None for nil), by the usual insertion; returns (nodes, root)."""
    nodes, root = {}, None

    def rotate(x, left):
        y = nodes[x][2 if left else 0]
        nodes[x][2 if left else 0] = nodes[y][0 if left else 2]
        child = nodes[y][0 if left else 2]
        if child is not None:
            nodes[child][1] = x
        nodes[y][1] = nodes[x][1]
        p = nodes[x][1]
        nonlocal root
        if p is None:
            root = y
        elif nodes[p][0] == x:
            nodes[p][0] = y
        else:
            nodes[p][2] = y
        nodes[y][0 if left else 2] = x
        nodes[x][1] = y

    for key in r.sample(range(100), n):
        nodes[key] = [None, None, None, RED]
        cur, parent = root, None
        while cur is not None:
            parent = cur
            cur = nodes[cur][0] if key < cur else nodes[cur][2]
        nodes[key][1] = parent
        if parent is None:
            root = key
        elif key < parent:
            nodes[parent][0] = key
        else:
            nodes[parent][2] = key
        z = key
        while nodes[z][1] is not None and nodes[nodes[z][1]][3] == RED:
            p = nodes[z][1]
            g = nodes[p][1]
            if p == nodes[g][0]:
                u = nodes[g][2]
                if u is not None and nodes[u][3] == RED:
                    nodes[p][3] = nodes[u][3] = BLACK
                    nodes[g][3] = RED
                    z = g
                else:
                    if z == nodes[p][2]:
                        z = p
                        rotate(z, True)
                        p = nodes[z][1]
                    nodes[p][3] = BLACK
                    nodes[g][3] = RED
                    rotate(g, False)
            else:
                u = nodes[g][0]
                if u is not None and nodes[u][3] == RED:
                    nodes[p][3] = nodes[u][3] = BLACK
                    nodes[g][3] = RED
                    z = g
                else:
                    if z == nodes[p][0]:
                        z = p
                        rotate(z, False)
                        p = nodes[z][1]
                    nodes[p][3] = BLACK
                    nodes[g][3] = RED
                    rotate(g, True)
        nodes[root][3] = BLACK
    return nodes, root


def case_insert(r, name):
    """An insert under the node the new key belongs below (the head for an empty tree), with x nil (or a non-nil hint when the left child is free)."""
    inc, erase, nil_at, colour = TREES[name]
    n = r.choice([0, 1, 2, 3, 5, 8, 11])
    nodes, root = build(r, n)
    keys = sorted(nodes)
    new = r.choice([k for k in range(100) if k not in nodes])
    addr = {k: NODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: NIL if k is None else addr[k]  # noqa: E731
    parent, cur = None, root
    while cur is not None:
        parent = cur
        cur = nodes[cur][0] if new < cur else nodes[cur][2]
    left = parent is None or new < parent
    y = HEAD if parent is None else addr[parent]
    x = NIL
    if left and (parent is None or nodes[parent][0] is None) and r.random() < 0.3:
        x = NODES + 0x3f00  # a non-nil hint: the new node goes left whatever the keys
    blob = bytearray(0x4000)
    for k in keys:
        base = addr[k] - NODES
        struct.pack_into('<III', blob, base, ptr(nodes[k][0]), HEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        struct.pack_into('<I', blob, base + 0xc, k)
        struct.pack_into('<I', blob, base + colour, nodes[k][3])
    head = struct.pack('<III', addr[keys[0]] if keys else HEAD, addr[root] if keys else NIL, addr[keys[-1]] if keys else HEAD) + bytes(0x20)
    nil = bytearray(0x40)
    struct.pack_into('<I', nil, colour, BLACK)
    tree = bytearray(0x20)
    struct.pack_into('<II', tree, 4, HEAD, 0)
    struct.pack_into('<I', tree, 0xc, n)
    value = struct.pack('<III', new, r.getrandbits(32), r.getrandbits(32))
    regions = {TREE: tree, HEAD: head, NIL: nil, NODES: blob, nil_at: struct.pack('<I', NIL), RAM + 0x800: bytes(0x10), RAM + 0x900: value, HEAP: heap()}
    return Call(INSERT[name], [TREE, RAM + 0x800, x, y, RAM + 0x900], regions, conv='t', ret='a')


def case(r):
    name = r.choice(OPS)
    if name in INSERT and r.random() < 0.3:
        return case_insert(r, name)
    inc, erase, nil_at, colour = TREES[name]
    n = r.choice([1, 2, 3, 4, 5, 6, 7, 8, 10, 12])
    nodes, root = build(r, n)
    keys = sorted(nodes)
    size = max(colour + 4, 0x28)
    addr = {k: NODES + 0x40 * i for i, k in enumerate(keys)}

    def ptr(k):
        return NIL if k is None else addr[k]

    blob = bytearray(0x40 * n)
    blocks = bytearray(r.bytes(14 * stdstring.SLOT))
    extra = {}
    strings = 0
    for k in keys:
        base = addr[k] - NODES
        struct.pack_into('<III', blob, base, ptr(nodes[k][0]), HEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        struct.pack_into('<I', blob, base + 0xc, k)
        struct.pack_into('<I', blob, base + colour, nodes[k][3])
        if name in 'CDE':  # a std::string value at +0xc
            struct.pack_into('<4I', blob, base + 0xc, r.getrandbits(32), *stdstring.string(r, blocks, strings))
            strings += 1
        elif name == 'F':  # one of the game's strings
            gamestring.string(r, 0, RAM + 0x4000 + 0x40 * strings, RAM + 0x6000 + 0x80 * strings, extra)
            blob[base + 0xc:base + 0x20] = extra[RAM + 0x4000 + 0x40 * strings][:0x14]
            strings += 1
    head = struct.pack('<III', addr[keys[0]], addr[root], addr[keys[-1]]) + bytes(0x20)
    nil = bytearray(0x40)
    struct.pack_into('<I', nil, colour, BLACK)
    tree = bytearray(0x20)
    struct.pack_into('<II', tree, 4, HEAD, 0)
    struct.pack_into('<I', tree, 0xc, n)
    regions = {TREE: tree, HEAD: head, NIL: nil, NODES: blob, nil_at: struct.pack('<I', NIL), RAM + 0x800: bytes(0x10), HEAP: heap(), **extra}
    if name in 'CDE':
        regions[stdstring.BLOCKS] = blocks
    node = addr[r.choice(keys)]
    if name in RANGE and r.random() < 0.35:
        erase_range, erase_subtree = RANGE[name]
        if r.random() < 0.25:
            return Call(erase_subtree, [TREE, r.choice([addr[root], node, node, NIL])], regions, conv='t', ret='v')
        order = [addr[k] for k in keys] + [HEAD]
        i = r.randrange(n + 1)
        j = r.randrange(i, n + 1)
        first, last = r.choice([(order[0], HEAD), (order[0], HEAD), (order[i], order[j]), (order[i], HEAD)])
        if r.random() < 0.15:
            struct.pack_into('<I', tree, 0xc, 0)
        return Call(erase_range, [TREE, RAM + 0x800, first, last], regions, conv='t', ret='a')
    if name in CTOR and r.random() < 0.2:  # the constructor: the nil node made already or not
        address, counter = CTOR[name]
        low = min(nil_at, counter) & ~0xf
        glob = bytearray(r.bytes(0x20))
        struct.pack_into('<I', glob, nil_at - low, r.choice([0, NIL]))
        regions[low] = glob
        return Call(address, [RAM + 0x400], {**regions, RAM + 0x400: r.bytes(0x20)}, conv='t', ret='v')
    if name in DEC and r.random() < 0.3:  # the decrement from a node, or from the end (the head)
        regions[RAM + 0x900] = struct.pack('<I', node if r.random() < 0.8 else HEAD)
        return Call(DEC[name], [RAM + 0x900], regions, conv='t', ret='v')
    if r.random() < 0.5:  # the increment from a node, then the end (the head)
        holder = struct.pack('<I', node if r.random() < 0.92 else HEAD)
        regions[RAM + 0x900] = holder
        return Call(inc, [RAM + 0x900], regions, conv='t', ret='v')
    return Call(erase, [TREE, RAM + 0x800, node], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(TREES)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The red-black tree members of src/game/core/stdtree (iterator increment and erase for seven instantiations of the game's std::map / std::set) against the originals:
# valid red-black trees of 1 to 12 nodes (built with the usual insertion), every node erased in turn across the cases, the iterator increment from every node; for B the
# range erase (0x4104b0: the whole tree, a tail, a stretch or nothing, the size now and then 0) and _Erase (0x410720) from the root, a node or nil, as C, D and E's (0x4f4070 / 0x4f4ac0 / 0x4f4fd0, the strings released) and their range erases (0x4f2590 / 0x4f2960 / 0x4f3290) and destructors (0x4e5870 / 0x4e5800 / 0x4e58e0); B's key compare (0x40f950); A's map constructor (0x47b060), the nil node made already or not.
# Tree H (the sound code's map of dword keys to Strings, 0x6c....) as C's, its Strings at +0x10, with its insert; MapH: its map members (insert(value), operator[], the
# string clear 0x6c0ba0, the pair's construct and copy, the WString assignment 0x6c0b10 and the value search 0x6c0b30). Tree I (the sound code's tree of plain values at 0xfb2010) as B's, with
# its destructor. Tree J (0xfb2000: a map of dword keys to trees I, each node's tree I of 0 to 5 nodes, released with it) as B's, with its constructor and destructor (0x6beda0).
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
         'G': (0x5b2e90, 0x5b2530, 0xf7c970, 0x10), 'H': (0x6c2de0, 0x6c1b70, 0xfb2008, 0x24),
         'I': (0x6c3910, 0x6c3340, 0xfb2010, 0x14), 'J': (0x6c2e90, 0x6c21f0, 0xfb2000, 0x20)}
INSERT = {'A': 0x47d140, 'B': 0x4107b0, 'G': 0x5b2ba0, 'H': 0x6c29b0}
DEC = {'A': 0x47d4b0, 'B': 0x410b20, 'C': 0x4f6340, 'D': 0x4f64f0, 'E': 0x4f6680, 'F': 0x5b2ff0, 'G': 0x5b3130, 'H': 0x6c2d20}
RANGE = {'B': (0x4104b0, 0x410720), 'C': (0x4f2590, 0x4f4070), 'D': (0x4f2960, 0x4f4ac0), 'E': (0x4f3290, 0x4f4fd0), 'H': (0x6c1270, 0x6c20b0), 'I': (0x6c31e0, 0x6c3870), 'J': (0x6c13e0, 0x6c2760)}   # erase(first, last), _Erase
DTOR = {'C': (0x4e5870, 0x7ced30), 'D': (0x4e5800, 0x7ced28), 'E': (0x4e58e0, 0x7ced38), 'H': (0x6c11c0, 0xfb200c), 'I': (0x6c3130, 0xfb2014), 'J': (0x6beda0, 0xfb2004)}   # the destructor, the map counter
CTOR = {'A': (0x47ce70, 0x7b4a50), 'C': (0x4f4120, 0x7ced30), 'D': (0x4f4b70, 0x7ced28), 'E': (0x4f5080, 0x7ced38), 'G': (0x5b2af0, 0xf7c96c), 'H': (0x6c2140, 0xfb200c), 'J': (0x6c2860, 0xfb2004)}
CLEARDTOR = {'H': (0x6c0a60, 0xfb200c)}   # a clear, then the destructor
B = Bench([(a, a + 0x330) for a in INSERT.values()] + [(0x5b3210, 0x5b3230)] + [(a, a + 0xc0) for a in DEC.values()] + [(a, a + 0xb0) for (a, _c) in CTOR.values()] + [(a, a + 0xb0) for (a, _e, _n, _c) in TREES.values()] + [(e, e + 0x560) for (_a, e, _n, _c) in TREES.values()] + [(0x40f950, 0x40f961), (0x47b060, 0x47b122), (0x4104b0, 0x410610), (0x410720, 0x4107a0), (0x4f4070, 0x4f4120), (0x4f4ac0, 0x4f4b70), (0x4f4fd0, 0x4f5080), (0x4f2590, 0x4f26f8), (0x4f2960, 0x4f2ac8), (0x4f3290, 0x4f33f8), (0x4f23f0, 0x4f2450), (0x4e5800, 0x4e596b), (0x67acb0, 0x67acb1), (0x45b340, 0x45b9a0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000), (0x6c0b10, 0x6c0ba0), (0x6c1130, 0x6c1240), (0x6c1270, 0x6c13e0), (0x6c1a90, 0x6c1b70), (0x6c20b0, 0x6c2140), (0x6c30a0, 0x6c3130), (0x6c3130, 0x6c31a0), (0x6c31e0, 0x6c3340), (0x6c38a0, 0x6c3910), (0x6c13e0, 0x6c1543), (0x6c2750, 0x6c2860), (0x6c3900, 0x6c3905), (0x6c0a60, 0x6c0b08), (0x6c0ba0, 0x6c0bc6), (0x6beda0, 0x6bee0b)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
TREE, HEAD, NIL, NODES = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x10000
NIL_I, SUBTREES = RAM + 0x380, RAM + 0x20000   # tree J's values: tree I's nil node and its trees (0x400 bytes each)
RED, BLACK = 0, 1


def build(r, n, pool=range(100)):
    """A red-black tree of n distinct keys (from pool) as dicts {key: [left, parent, right, colour]} (None for nil), by the usual insertion; returns (nodes, root)."""
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

    for key in r.sample(pool, n):
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
    extra = {}
    if name == 'H':  # {key, String}: the node's String a copy (String_CtorCopy)
        gamestring.string(r, 0, RAM + 0xa00, RAM + 0xb00, extra)
        value = struct.pack('<I', new) + extra.pop(RAM + 0xa00)[:0x14]
    regions = {TREE: tree, HEAD: head, NIL: nil, NODES: blob, nil_at: struct.pack('<I', NIL), RAM + 0x800: bytes(0x10), RAM + 0x900: value, HEAP: heap(), **extra}
    return Call(INSERT[name], [TREE, RAM + 0x800, x, y, RAM + 0x900], regions, conv='t', ret='a')


def case_map_h(r):
    """H's map members: insert(value), operator[], the pair's construct and copy, the WString assignment and the value search, over a tree of 0 to 9 keys
    (mostly from 0 to 9, the search's indices) whose Strings are random, the multi-insert byte now and then set."""
    n = r.choice([0, 1, 2, 3, 4, 5, 6, 8, 9])
    nodes, root = build(r, n, r.choice([range(10), range(10), range(12), range(100)]))
    keys = sorted(nodes)
    addr = {k: NODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: NIL if k is None else addr[k]  # noqa: E731
    blob = bytearray(0x40 * max(n, 1))
    extra, texts = {}, []
    for i, k in enumerate(keys):
        base = addr[k] - NODES
        struct.pack_into('<III', blob, base, ptr(nodes[k][0]), HEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        struct.pack_into('<I', blob, base + 0xc, k)
        struct.pack_into('<I', blob, base + 0x24, nodes[k][3])
        length = gamestring.string(r, 0, RAM + 0x4000 + 0x40 * i, RAM + 0x6000 + 0x80 * i, extra)
        blob[base + 0x10:base + 0x24] = extra.pop(RAM + 0x4000 + 0x40 * i)[:0x14]
        texts.append(bytes(extra[RAM + 0x6000 + 0x80 * i][:length]))
    head = struct.pack('<III', addr[keys[0]] if keys else HEAD, addr[root] if keys else NIL, addr[keys[-1]] if keys else HEAD) + bytes(0x20)
    nil = bytearray(0x40)
    struct.pack_into('<I', nil, 0x24, BLACK)
    tree = bytearray(r.bytes(0x10))
    struct.pack_into('<IBxxxI', tree, 4, HEAD, int(r.random() < 0.1), n)
    regions = {TREE: tree, HEAD: head, NIL: nil, NODES: blob, 0xfb2008: struct.pack('<I', NIL), RAM + 0x800: r.bytes(0x10), HEAP: heap(), **extra}
    key = r.choice(keys + [r.randrange(12), r.getrandbits(32)] if keys else [r.randrange(12), r.getrandbits(32)])
    regions[RAM + 0x900] = struct.pack('<I', key)
    op = r.deal(['insert', 'index', 'index', 'construct', 'copy', 'assign', 'find', 'find', 'find', 'clearat', 'clearat'])
    if op in ('insert', 'construct', 'copy'):  # a value {key, String}
        gamestring.string(r, 0, RAM + 0xa00, RAM + 0xb00, regions)
        regions[RAM + 0x900] = struct.pack('<I', key) + regions.pop(RAM + 0xa00)[:0x14]
    if op == 'insert':
        return Call(0x6c1a90, [TREE, RAM + 0x800, RAM + 0x900], regions, conv='t', ret='v')
    if op == 'index':
        return Call(0x6c1130, [TREE, RAM + 0x900], regions, conv='t', ret='a')
    if op == 'construct':
        regions[RAM + 0xc00] = r.bytes(0x20)
        return Call(0x6c30a0, [r.choice([RAM + 0xc00, RAM + 0xc00, 0]), RAM + 0x900], regions, conv='c', ret='v')
    if op == 'copy':
        regions[RAM + 0xc00] = r.bytes(0x20)
        return Call(0x6c3110, [RAM + 0xc00, RAM + 0x900], regions, conv='t', ret='a')
    if op == 'clearat':  # map[key] = '' for keys 1 to below the size: 0, inside, the size, past it, any
        key = r.choice(keys + [0, n, n + 1, r.randrange(12), r.getrandbits(32)])
        return Call(0x6c0ba0, [TREE, key], regions, conv='t', ret='v')
    if op == 'assign':  # map[key] = a WString (through the String's vtable +0x18)
        gamestring.string(r, 1, RAM + 0xa00, RAM + 0xb00, regions)
        return Call(0x6c0b10, [TREE, RAM + 0xa00, key], regions, conv='t', ret='a')
    # the search: a text one of the Strings has (or a prefix, or another)
    text = r.choice(texts + [b'', b'ab']) if texts else r.choice([b'', b'ab'])
    if text and r.random() < 0.2:
        text = text[:-1]
    gamestring.string(r, 0, RAM + 0xa00, RAM + 0xb00, regions, text=text)
    regions[RAM + 0xc00] = r.bytes(4)
    return Call(0x6c0b30, [TREE, RAM + 0xa00, RAM + 0xc00], regions, conv='t', ret='a')


def subtree(r, i, extra):
    """Tree J's i-th value's tree I (0 to 5 nodes, colour at +0x14) into extra; returns its (head, a dword, size)."""
    m = r.choice([0, 0, 1, 2, 3, 5])
    nodes, root = build(r, m)
    keys = sorted(nodes)
    head = SUBTREES + 0x400 * i
    addr = {k: head + 0x20 * (j + 1) for j, k in enumerate(keys)}
    blob = bytearray(0x20 * (m + 1))
    if m:
        struct.pack_into('<III', blob, 0, addr[keys[0]], addr[root], addr[keys[-1]])
    else:
        struct.pack_into('<III', blob, 0, head, NIL_I, head)
    struct.pack_into('<I', blob, 0x14, RED)
    for k in keys:
        at = addr[k] - head
        left, parent, right = nodes[k][:3]
        struct.pack_into('<IIII', blob, at, NIL_I if left is None else addr[left], head if parent is None else addr[parent], NIL_I if right is None else addr[right], k)
        struct.pack_into('<I', blob, at + 0x14, nodes[k][3])
    extra[head] = bytes(blob)
    return head, r.getrandbits(32), m


def case(r):
    name = r.deal(OPS)
    if name == 'MapH':
        return case_map_h(r)
    if name == 'KeyLessB':  # B's key compare (0x40f950): two words, equal, one apart, at the ends, random above them
        a = r.choice([0, 1, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)])
        b = r.choice([a, a + 1, a - 1, 0, 0xffff, 0x8000, r.getrandbits(16)]) & 0xffff
        regions = {RAM: struct.pack('<HH', a, r.getrandbits(16)), RAM + 0x10: struct.pack('<HH', b, r.getrandbits(16))}
        return Call(0x40f950, [RAM, RAM + 0x10], regions, conv='t', ret='a')
    map_ctor = name == 'MapCtorA'  # A's map constructor (0x47b060: the two bytes its arguments point at, then the tree's)
    if map_ctor:
        name = 'A'
        kind = None
    else:   # which member: dealt over the tree's, so that a short run (the coverage gate's) reaches each. Each tree its own deck (the
        # pairs name it): r.deal keeps one deck per list, and C, D and E have the same members, so a shared deck dealt them in turn
        # and left some tree's member with no case in the gate's run.
        _tree, kind = r.deal([(name, k) for k in ['inc', 'erase'] + [k for k, table in (('insert', INSERT), ('range', RANGE), ('subtree', RANGE),
                                                                                        ('dtor', DTOR), ('cleardtor', CLEARDTOR), ('ctor', CTOR), ('dec', DEC)) if name in table]])
    if kind == 'insert':
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
        elif name == 'J':  # a tree I object at +0x10 (its allocator byte, head, a dword, size)
            struct.pack_into('<4I', blob, base + 0x10, r.getrandbits(8), *subtree(r, keys.index(k), extra))
        elif name in 'FH':  # one of the game's strings (H: after the key)
            gamestring.string(r, 0, RAM + 0x4000 + 0x40 * strings, RAM + 0x6000 + 0x80 * strings, extra)
            at = base + (0xc if name == 'F' else 0x10)
            blob[at:at + 0x14] = extra[RAM + 0x4000 + 0x40 * strings][:0x14]
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
    if name == 'J' and kind != 'ctor':  # tree I's nil and counter: its trees' count, more, or fewer (the nil node freed on the way)
        nil_i = bytearray(0x20)
        struct.pack_into('<I', nil_i, 0x14, BLACK)
        regions[NIL_I] = nil_i
        regions[0xfb2010] = struct.pack('<II', NIL_I, r.choice([n, n, n + 1, n + 7, max(1, n - 2), 1]))
    node = addr[r.choice(keys)]
    if map_ctor:
        low = 0x7b4a50
        glob = bytearray(r.bytes(0x20))
        struct.pack_into('<I', glob, nil_at - low, r.choice([0, NIL]))
        regions[low] = glob
        regions[RAM + 0x480] = r.bytes(0x10)
        return Call(0x47b060, [RAM + 0x400, RAM + 0x480, RAM + 0x488], {**regions, RAM + 0x400: r.bytes(0x20)}, conv='t', ret='a')
    if kind == 'subtree':  # _Erase from the root, a node or nil
        return Call(RANGE[name][1], [TREE, r.choice([addr[root], node, node, NIL])], regions, conv='t', ret='v')
    if kind == 'range':
        erase_range = RANGE[name][0]
        order = [addr[k] for k in keys] + [HEAD]
        i = r.randrange(n + 1)
        j = r.randrange(i, n + 1)
        first, last = r.choice([(order[0], HEAD), (order[0], HEAD), (order[i], order[j]), (order[i], HEAD)])
        if r.random() < 0.15:
            struct.pack_into('<I', tree, 0xc, 0)
        return Call(erase_range, [TREE, RAM + 0x800, first, last], regions, conv='t', ret='a')
    if kind in ('dtor', 'cleardtor'):  # the destructor: the map counter 1 (the nil node freed), more, or 0
        address, counter = (DTOR if kind == 'dtor' else CLEARDTOR)[name]
        low = min(nil_at, counter) & ~0xf
        glob = bytearray(r.bytes(0x20))
        struct.pack_into('<I', glob, nil_at - low, NIL)
        struct.pack_into('<I', glob, counter - low, r.choice([1, 1, 2, 5, 0]))
        del regions[nil_at]
        if low <= 0xfb2010 < low + len(glob) and 0xfb2010 in regions:   # (J's: tree I's nil and counter inside the same 0x20 bytes)
            glob[0x10:0x18] = regions.pop(0xfb2010)
        regions[low] = glob
        return Call(address, [TREE], regions, conv='t', ret='v')
    if kind == 'ctor':  # the constructor: the nil node made already or not
        address, counter = CTOR[name]
        low = min(nil_at, counter) & ~0xf
        glob = bytearray(r.bytes(0x20))
        struct.pack_into('<I', glob, nil_at - low, r.choice([0, NIL]))
        regions[low] = glob
        return Call(address, [RAM + 0x400], {**regions, RAM + 0x400: r.bytes(0x20)}, conv='t', ret='v')
    if kind == 'dec':  # the decrement from a node, or from the end (the head)
        regions[RAM + 0x900] = struct.pack('<I', node if r.random() < 0.8 else HEAD)
        return Call(DEC[name], [RAM + 0x900], regions, conv='t', ret='v')
    if kind == 'inc':  # the increment from a node, then the end (the head)
        holder = struct.pack('<I', node if r.random() < 0.92 else HEAD)
        regions[RAM + 0x900] = holder
        return Call(inc, [RAM + 0x900], regions, conv='t', ret='v')
    return Call(erase, [TREE, RAM + 0x800, node], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(list(TREES) + ['KeyLessB', 'MapCtorA', 'MapH', 'MapH'])).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The wide string map of src/game/core/wstringmap (0x4f6400, 0x4f6940, 0x4f2b40, 0x4f2ad0) against the originals: valid red-black trees (stdtree.py's) of 0 to 12 nodes whose
# strings are wide characters from a small alphabet in ascending order (some empty, some with a null character pointer; their blocks counted or not), searched (or erased) with
# keys equal to a node's or not.
# usage: wstringmap.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Lower,Upper,Find,Erase)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import stdtree  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap  # noqa: E402

CALLS = {'Lower': 0x4f6400, 'Upper': 0x4f6940, 'Find': 0x4f2b40, 'Erase': 0x4f2ad0}
B = Bench([(a, a + 0x100) for a in CALLS.values()] + [(0x4f4f20, 0x4f4fd0), (0x4f4580, 0x4f4ae0), (0x4f4ac0, 0x4f4b70), (0x4f2960, 0x4f2ac8),
                                                        (0x4f23f0, 0x4f2450), (0x67acb0, 0x67acb1)], hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
CHARS, KEY, OUT = RAM + 0x4000, RAM + 0x900, RAM + 0x7000
ALPHA = [0x41, 0x42, 0x61, 0x100, 0x8000, 0xffff]


def case(r):
    name = r.choice(OPS)
    n = r.choice([0, 1, 2, 3, 5, 8, 12])
    nodes, root = stdtree.build(r, n) if n else ({}, None)
    keys = sorted(nodes)
    strings = sorted({tuple(r.choice(ALPHA) for _ in range(r.choice([0, 1, 2, 3, 4]))) for _ in range(len(keys) * 2)})[:len(keys)]
    while len(strings) < len(keys):
        strings.append(strings[-1] + (0xffff,) if strings else (1,))
    if name == 'Erase' and strings and r.random() < 0.4:  # (a multimap's: keys repeated, so more than one node erased)
        strings = sorted(r.choice(strings) for _ in strings)
    addr = {k: stdtree.NODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: stdtree.NIL if k is None else addr[k]  # noqa: E731
    blob = bytearray(0x40 * max(len(keys), 1))
    pool = bytearray(0x400)
    at = 2   # (each string's block: a count byte before the characters, freed 2 bytes before them by the erase)
    for i, k in enumerate(keys):
        base = addr[k] - stdtree.NODES
        struct.pack_into('<III', blob, base, ptr(nodes[k][0]), stdtree.HEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        struct.pack_into('<I', blob, base + 0x1c, nodes[k][3])
        s = strings[i]
        struct.pack_into('<%dH' % len(s), pool, at, *s)
        pool[at - 1] = r.choice([0, 0, 1, 2, 0xff])
        struct.pack_into('<III', blob, base + 0x10, CHARS + at if s or r.random() < 0.5 else 0, len(s), len(s) + r.choice([0, 0, 3]))
        at += 2 * len(s) + 4
    head = struct.pack('<III', addr[keys[0]] if keys else stdtree.HEAD, addr[root] if keys else stdtree.NIL, addr[keys[-1]] if keys else stdtree.HEAD) + bytes(0x20)
    nil = bytearray(0x40)
    struct.pack_into('<I', nil, 0x1c, 1)
    tree = bytearray(0x20)
    struct.pack_into('<III', tree, 4, stdtree.HEAD, 0, n)
    if strings and r.random() < 0.5:
        s = r.choice(strings)
    else:
        s = tuple(r.choice(ALPHA) for _ in range(r.choice([0, 1, 2, 3, 4])))
    kb = bytearray(0x10)
    struct.pack_into('<II', kb, 4, CHARS + 0x300 if s or r.random() < 0.5 else 0, len(s))
    struct.pack_into('<%dH' % len(s), pool, 0x300, *s)
    regions = {stdtree.TREE: tree, stdtree.HEAD: head, stdtree.NIL: nil, stdtree.NODES: blob, 0x7ced24: struct.pack('<I', stdtree.NIL), KEY: kb, CHARS: pool}
    if name == 'Find':  # the output word is given too
        regions[OUT] = r.bytes(4)
        return Call(CALLS[name], [stdtree.TREE, OUT, KEY], regions, conv='t', ret='a')
    if name == 'Erase':  # the nodes freed (and their strings' blocks released)
        regions[HEAP] = heap()
    return Call(CALLS[name], [stdtree.TREE, KEY], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

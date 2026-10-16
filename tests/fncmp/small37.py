# The wide string map bounds of src/game/core/small37 (0x4f6400, 0x4f6940) against the originals: valid red-black trees (stdtree.py's) of 0 to 12 nodes whose strings are
# wide characters from a small alphabet in ascending order (some empty, some with a null character pointer), searched with keys equal to a node's or not.
# usage: small37.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Lower,Upper)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import stdtree  # noqa: E402
from fncmp import Bench, Call, RAM  # noqa: E402

CALLS = {'Lower': 0x4f6400, 'Upper': 0x4f6940}
B = Bench([(a, a + 0x100) for a in CALLS.values()], hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
CHARS, KEY = RAM + 0x4000, RAM + 0x900
ALPHA = [0x41, 0x42, 0x61, 0x100, 0x8000, 0xffff]


def case(r):
    name = r.choice(OPS)
    n = r.choice([0, 1, 2, 3, 5, 8, 12])
    nodes, root = stdtree.build(r, n) if n else ({}, None)
    keys = sorted(nodes)
    strings = sorted({tuple(r.choice(ALPHA) for _ in range(r.choice([0, 1, 2, 3, 4]))) for _ in range(len(keys) * 2)})[:len(keys)]
    while len(strings) < len(keys):
        strings.append(strings[-1] + (0xffff,) if strings else (1,))
    addr = {k: stdtree.NODES + 0x40 * i for i, k in enumerate(keys)}
    ptr = lambda k: stdtree.NIL if k is None else addr[k]  # noqa: E731
    blob = bytearray(0x40 * max(len(keys), 1))
    pool = bytearray(0x400)
    at = 0
    for i, k in enumerate(keys):
        base = addr[k] - stdtree.NODES
        struct.pack_into('<III', blob, base, ptr(nodes[k][0]), stdtree.HEAD if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
        struct.pack_into('<I', blob, base + 0x1c, nodes[k][3])
        s = strings[i]
        struct.pack_into('<%dH' % len(s), pool, at, *s)
        struct.pack_into('<II', blob, base + 0x10, CHARS + at if s or r.random() < 0.5 else 0, len(s))
        at += 2 * len(s) + 2
    head = struct.pack('<III', addr[keys[0]] if keys else stdtree.HEAD, addr[root] if keys else stdtree.NIL, addr[keys[-1]] if keys else stdtree.HEAD) + bytes(0x20)
    nil = bytearray(0x40)
    struct.pack_into('<I', nil, 0x1c, 1)
    tree = bytearray(0x20)
    struct.pack_into('<II', tree, 4, stdtree.HEAD, 0)
    if strings and r.random() < 0.5:
        s = r.choice(strings)
    else:
        s = tuple(r.choice(ALPHA) for _ in range(r.choice([0, 1, 2, 3, 4])))
    kb = bytearray(0x10)
    struct.pack_into('<II', kb, 4, CHARS + 0x300 if s or r.random() < 0.5 else 0, len(s))
    struct.pack_into('<%dH' % len(s), pool, 0x300, *s)
    regions = {stdtree.TREE: tree, stdtree.HEAD: head, stdtree.NIL: nil, stdtree.NODES: blob, 0x7ced24: struct.pack('<I', stdtree.NIL), KEY: kb, CHARS: pool}
    return Call(CALLS[name], [stdtree.TREE, KEY], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

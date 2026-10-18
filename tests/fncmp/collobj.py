# The track rides' collision object pool and list releases (src/game/world/collobj: 0x544330,
# 0x5457e0, 0x545810, 0x54b290) against the originals: lists empty to 8 nodes in any order in memory,
# masks 0, one bit or any, nodes in use or not; chunk lists of up to three chunks with a node pair or none.
# usage: collobj.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x544330, 0x544356), (0x5457e0, 0x545809), (0x545810, 0x545888), (0x54b290, 0x54b2f0)])
NODES, POOL, GLOBALS = RAM + 0x200, RAM + 0x1000, 0x877b70


def free_all(r):
    # chunks at RAM + 0x100 + 0x40 * i (list head at the owner's +0xbc), their nodes at RAM + 0x400 + 0x20 * j
    obj = bytearray(r.bytes(0x400)) + bytes(0x200)
    owner = RAM + 0x20
    nodes = [RAM + 0x400 + 0x20 * j for j in range(6)]
    cs = [RAM + 0x100 + 0x40 * i for i in range(3)]
    put(obj, 0x20 + 0xbc, 'I', cs[0] if r.random() < 0.9 else 0)
    for i, c in enumerate(cs):
        put(obj, 0x100 + 0x40 * i + 0x24, 'I', cs[i + 1] if i + 1 < len(cs) and r.random() < 0.8 else 0)
        put(obj, 0x100 + 0x40 * i + 0x1c, 'I', r.choice([0, nodes[2 * i]]))
        put(obj, 0x100 + 0x40 * i, 'I', r.getrandbits(32))
    for j in range(6):
        put(obj, 0x400 + 0x20 * j + 0x14, 'I', nodes[j + 1] if j % 2 == 0 and r.random() < 0.7 else 0)
        put(obj, 0x400 + 0x20 * j, 'I', r.getrandbits(32))
    return Call(0x54b290, [owner], {RAM: obj, 0x877b70: r.bytes(0x20)})


def case(r):
    op = r.randrange(4)
    if op == 3:
        return free_all(r)
    globals_ = bytearray(r.bytes(0x20))
    if op == 0:
        put(globals_, 0x18, 'I', POOL)
        return Call(0x544330, [], {GLOBALS: globals_, POOL: r.bytes(0x5000)})
    collider = bytearray(r.bytes(0x100))
    put(collider, 0xac, 'I', r.choice([0, 1, 0x10000, r.getrandbits(32)]))
    put(globals_, 0xc, 'I', r.choice([0, 1, 0x10000, r.getrandbits(32)]))
    nodes = bytearray(r.bytes(8 * 0x18))
    n = r.choice([0, 1, 2, r.randrange(9)])
    order = r.sample(range(8), n)
    for k, i in enumerate(order):
        put(nodes, i * 0x18 + 0x14, 'I', NODES + order[k + 1] * 0x18 if k + 1 < n else 0)
        put(nodes, i * 0x18, 'I', r.choice([0, 1, 2, 3, 0x10, 0x11, r.getrandbits(32)]))
    put(collider, 0xc0 if op == 1 else 0xa8, 'I', NODES + order[0] * 0x18 if n else 0)
    args = [RAM]
    if op == 2:
        args.append(r.choice([0, 1, 2, 0x10, 0xfffffffe, r.getrandbits(32)]))
    return Call(0x5457e0 if op == 1 else 0x545810, args, {RAM: collider, NODES: nodes, GLOBALS: globals_})


if __name__ == '__main__':
    B.main(case)

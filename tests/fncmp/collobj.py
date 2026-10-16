# The track rides' collision object pool and list releases (src/game/world/collobj: 0x544330,
# 0x5457e0, 0x545810) against the originals: lists empty to 8 nodes in any order in memory,
# masks 0, one bit or any, nodes in use or not.
# usage: collobj.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x544330, 0x544356), (0x5457e0, 0x545809), (0x545810, 0x545888)])
NODES, POOL, GLOBALS = RAM + 0x200, RAM + 0x1000, 0x877b70


def case(r):
    op = r.randrange(3)
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

# The owner item list searches (src/game/online/itemlist: 0x5bc9c0, 0x5bcb60, 0x5bd270, 0x5bd670,
# 0x5bd690, 0x5bd9d0) against the originals: lists of 0 to 6 items whose keys, kinds and pairs
# come from a few values so that none, one or several match (the last one wins).
# usage: itemlist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FindByKey,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5bc9c0, 0x5bc9e7), (0x5bcb60, 0x5bcb8b), (0x5bd270, 0x5bd298), (0x5bd670, 0x5bd690), (0x5bd690, 0x5bd6b0), (0x5bd9d0, 0x5bda00)])
CALLS = {'FindByKey': 0x5bc9c0, 'At': 0x5bcb60, 'ValueByKey': 0x5bd270, 'FindKind1': 0x5bd670, 'FindKind100': 0x5bd690,
         'FindByPair': 0x5bd9d0}
LIST, NODES, ITEMS = RAM + 0x100, RAM + 0x1000, RAM + 0x2000


def case(r):
    op = r.choice(OPS)
    n = r.choice([0, 1, 2, 3, 6])
    owner = bytearray(r.bytes(0x40))
    put(owner, 0x2c, 'I', LIST)
    lst = bytearray(r.bytes(0x10))
    put(lst, 4, 'I', NODES if n else 0)
    nodes, items = bytearray(0x100), bytearray(r.bytes(0x100 * 6))
    for i in range(n):
        put(nodes, 8 * i, 'II', ITEMS + 0x100 * i, NODES + 8 * (i + 1) if i + 1 < n else 0)
        put(items, 0x100 * i + 8, 'I', r.choice([1, 2, 3]))
        put(items, 0x100 * i + 0xc, 'I', r.choice([1, 0x64, 0, 2]))
        put(items, 0x100 * i + 0x44, 'I', r.choice([5, 6]))
        put(items, 0x100 * i + 0x64, 'I', r.choice([7, 8]))
    regions = {RAM: owner, LIST: lst, NODES: nodes, ITEMS: items}
    if op in ('FindByKey', 'ValueByKey'):
        args = [RAM, r.choice([1, 2, 3, 4])]
    elif op == 'At':
        args = [RAM, r.choice([0, 1, 2, 5, 6, -1])]
    elif op == 'FindByPair':
        args = [RAM, r.choice([5, 6, 9]), r.choice([7, 8, 9])]
    else:
        args = [RAM]
    return Call(CALLS[op], [a & 0xffffffff for a in args], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

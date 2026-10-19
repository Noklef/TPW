# The Exec-style list (src/game/core/list: 0x62f9e0-0x62fa33) against the originals: a list header and a few nodes in one block, the list
# empty or linked through some of them (the header's head and tailPred pointing at its own tail and head when empty), the node to insert
# fresh or already linked.
# usage: list.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Init,InsertAfter,First)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x62f9e0, 0x62fa34)])
LIST = RAM          # the header (head, tail 0, tailPred)
NODES = RAM + 0x10  # 8-byte nodes (next, prev), 8 of them


def block(r):
    """The header and nodes: a list of 0-6 of the nodes in a random order."""
    b = bytearray(r.bytes(0x10 + 8 * 8))
    order = r.sample(range(8), r.choice([0, 0, 1, 2, 3, 6]))
    chain = [LIST] + [NODES + 8 * k for k in order] + [LIST + 4]
    for i in range(1, len(chain) - 1):
        struct.pack_into('<2I', b, chain[i] - RAM, chain[i + 1], chain[i - 1])
    struct.pack_into('<3I', b, 0, chain[1], 0 if r.random() < 0.9 else r.getrandbits(32), chain[-2])
    return b, order


def case(r):
    op = r.deal(OPS)
    b, order = block(r)
    regions = {RAM: b}
    if op == 'Init':
        return Call(0x62f9e0, [LIST], regions, ret='a')
    if op == 'First':
        return Call(0x62fa20, [LIST], regions, ret='a')
    linked = [LIST] + [NODES + 8 * k for k in order]
    after = r.choice(linked)
    node = NODES + 8 * r.choice([k for k in range(8) if k not in order] or [0])
    return Call(0x62fa00, [r.choice([LIST, 0, r.getrandbits(32)]), after, node], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', 'Init,InsertAfter,First').split(',') if k]


if __name__ == '__main__':
    B.main(case)

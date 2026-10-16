# The free list take of src/game/core/small42 (0x520d60) against the original: a table of up to 12 records with random links (negative ones too) and a head word either
# side of zero, the 0x140-byte records for the index.
# usage: small42.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x520d60, 0x520e00)])
NODES = RAM + 0x1000


def case(r):
    nodes = bytearray(r.bytes(0x34 * 12))
    link = lambda: r.choice([-1, 0, 1, 2, 5, 11, r.randrange(12), 0x7fff & r.getrandbits(4)])  # noqa: E731
    for k in range(12):
        put(nodes, 0x34 * k + 0x30, 'hH', link(), r.choice([0xffff, 0, 3, r.randrange(12)]))
    index = r.randrange(4)
    records = bytearray(r.bytes(0x140 * 4))
    put(records, 0x140 * index + 2, 'h', r.choice([-1, 0, 1, 7, r.randrange(12), -5]))
    head = r.choice([-1, 0, 1, 5, 11, r.randrange(12)])
    regions = {0x80cf00: records, 0x80cef8: struct.pack('<I', NODES), NODES: nodes, 0x816d24: put(bytearray(r.bytes(4)), 0, 'h', head)}
    return Call(0x520d60, [index], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

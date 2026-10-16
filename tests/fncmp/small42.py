# The free list take of src/game/core/small42 (0x520d60) against the original: a table of up to 12 records with random links (negative ones too) and a head word either
# side of zero, the 0x140-byte records for the index.
# usage: small42.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'Take': 0x520d60, 'Return': 0x521890, 'Scale': 0x521d60}
B = Bench([(0x520d60, 0x520e00), (0x521890, 0x521940), (0x521d60, 0x521e60)])
NODES = RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    if name == 'Scale':
        index = r.randrange(4)
        records = bytearray(r.bytes(0x140 * 4))
        for off in (0x68, 0x69, 0x6a, 0x6b):
            records[0x140 * index + off] = r.choice([0, 1, 2, 0x7f, 0x80, 0xff, 0x40, r.getrandbits(8)])
        put(records, 0x140 * index + 0x64, 'h', r.choice([0, 1, 2, 100, -1, 0x7fff, r.randrange(-50, 3000)]))
        put(records, 0x140 * index + 0x60, 'i', r.choice([0, 1, 2, 100, -1, 0x7fffffff, r.randrange(-50, 100000)]))
        factor = r.choice([0, 1, 512, 1024, 1023, 2000, 0x100000, -1 & 0xffffffff, r.getrandbits(12), r.getrandbits(32)])
        return Call(CALLS[name], [index], {0x80cefc: records, 0x80cef0: struct.pack('<I', factor)}, ret='v')
    if name == 'Return':
        nodes = bytearray(r.bytes(0x34 * 12))
        link = lambda: r.choice([-1, 0, 1, 2, 5, 11, r.randrange(12)])  # noqa: E731
        for k in range(12):
            put(nodes, 0x34 * k + 0x30, 'hH', link(), r.choice([0xffff, 0, 3, r.randrange(12)]))
        index = r.randrange(4)
        record = r.choice([-1, 0, 1, 5, 11, r.randrange(12)])
        records = bytearray(r.bytes(0x140 * 4))
        put(records, 0x140 * index + 2, 'h', record if r.random() < 0.6 else r.choice([-1, 3, 7]))
        regions = {0x80cf00: records, 0x80cef8: struct.pack('<I', NODES), NODES: nodes, 0x816d24: put(bytearray(r.bytes(4)), 0, 'h', r.choice([-1, 0, 4, r.randrange(12)]))}
        return Call(CALLS[name], [record & 0xffffffff, index], regions, ret='v' if name == 'Return' else 'a')
    nodes = bytearray(r.bytes(0x34 * 12))
    link = lambda: r.choice([-1, 0, 1, 2, 5, 11, r.randrange(12), 0x7fff & r.getrandbits(4)])  # noqa: E731
    for k in range(12):
        put(nodes, 0x34 * k + 0x30, 'hH', link(), r.choice([0xffff, 0, 3, r.randrange(12)]))
    index = r.randrange(4)
    records = bytearray(r.bytes(0x140 * 4))
    put(records, 0x140 * index + 2, 'h', r.choice([-1, 0, 1, 7, r.randrange(12), -5]))
    head = r.choice([-1, 0, 1, 5, 11, r.randrange(12)])
    regions = {0x80cf00: records, 0x80cef8: struct.pack('<I', NODES), NODES: nodes, 0x816d24: put(bytearray(r.bytes(4)), 0, 'h', head)}
    return Call(CALLS[name], [index], regions, ret='v' if name == 'Return' else 'a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

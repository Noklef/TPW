# The bit reader and the record lookup of src/game/core/small43 (0x597090, 0x59c610) against the originals: counts 0 to 32 (and more) with the accumulator and held bit
# counts random, the stream of random words; records whose ids are right, wrong or found only elsewhere in the table.
# usage: small43.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Bits,Active)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'Bits': 0x597090, 'Active': 0x59c610}
B = Bench([(0x597090, 0x597100), (0x59c610, 0x59c690), (0x5da3c0, 0x5da3c1)])
TABLE = 0x76e300


def case(r):
    name = r.choice(OPS)
    if name == 'Bits':
        regions = {0xaa67f8: struct.pack('<I', r.choice([0, 1, 5, 15, 16, 17, 31, 32, 40, r.randrange(48)])), 0xbe6800: struct.pack('<I', RAM + 0x100 + 2 * r.randrange(8)),
                   0xe74a40: struct.pack('<I', r.choice([0, 0x10, 0xfffffff0])), 0xf74a48: struct.pack('<I', r.getrandbits(32)), RAM: r.bytes(0x400)}
        return Call(CALLS[name], [r.choice([0, 1, 2, 3, 7, 8, 15, 16, 17, 24, 31, 32, 33, r.randrange(40)])], regions, ret='a')
    table = bytearray(r.bytes(0x38 * 0x160))
    for k in range(0x160):
        put(table, 0x38 * k + 4, 'I', k if r.random() < 0.7 else r.randrange(0x160))
        put(table, 0x38 * k + 8, 'I', r.choice([0, 1, 1, 2, r.getrandbits(32)]))
    ident = r.choice([0x15f, 0x160, 0, 1, 5, 0x15e, r.randrange(0x15e)])
    return Call(CALLS[name], [ident], {TABLE: table}, conv='s', ret='b')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

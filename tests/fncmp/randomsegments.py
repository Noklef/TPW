# The object of two random segments (src/game/render/randomsegments: RandomSegments_Ctor 0x5dfcd0) against the original: the generator at [0xf82884] +0x34 random, the two
# vectors at [0xf83778] random, the block's allocation failing now and then, and the registry [0xf85614]'s slot 6 (Class702d48_Ctor's) a recorder. The direction's zero case
# would take six equal draws: not reached.
# usage: randomsegments.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x5dfcd0, 0x5e032e), (0x5e3b90, 0x5e3bb0)])
OBJECT, SOURCE, WORLD, REGISTRY, TABLE = RAM, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x340


def case(r):
    regions = {LOG: log_region(), HEAP: heap(0x40, r.choice([0] * 7 + [1])), OBJECT: bytearray(r.bytes(0xe0)), SOURCE: r.bytes(0x20),
                WORLD: put(bytearray(r.bytes(0x40)), 0x34, 'I', r.getrandbits(32)), REGISTRY: put(bytearray(r.bytes(8)), 0, 'I', TABLE), TABLE: vtable({6: 1}, 16),
                0xf83778: struct.pack('<I', SOURCE), 0xf82884: struct.pack('<I', WORLD), 0xf85614: struct.pack('<I', REGISTRY)}
    return Call(0x5dfcd0, [OBJECT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

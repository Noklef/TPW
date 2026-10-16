# Starting a draw batch (src/game/render/batch: 0x565240) against the original, over the draw buffer
# of tests/fncmp/drawbuffer.py (streams, batch slots in and around the table): modes with and
# without 2 and 4, masks that meet the flags or not, the scene flag set or not, the primitive kinds
# 0xc4, 0x1c4, 0x2c4, 0x3c4 and others.
# usage: batch.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x565240, 0x56530d), (0x582640, 0x582804)])
BUF, BATCHES, OBJECT = RAM + 0x20, RAM + 0x100, RAM + 0x1000


def case(r):
    mem = bytearray(r.bytes(0x100 + 16 * 0x38))
    b = 0x20
    put(mem, b + 0, 'I', r.choice([0, 1, 2, 3, 4, 5, 6, 7, r.getrandbits(32)]))
    put(mem, b + 4, 'I', r.randrange(4))
    put(mem, b + 0x44, 'I', BATCHES)
    put(mem, b + 0x48, 'I', r.choice([0, 1, 8, 16, r.randrange(17)]))
    for off in (0x4c, 0x50, 0x54):
        put(mem, b + off, 'I', r.randrange(16))
    start = r.getrandbits(32)
    put(mem, b + 0x5c, 'I', start)
    put(mem, b + 0x3c, 'I', r.choice([start, start + 0x100, start + 0x10000, r.getrandbits(32)]) & 0xffffffff)
    obj = bytearray(r.bytes(0x60))
    put(obj, 0xc, 'I', r.choice([0, 0xffffffff, r.getrandbits(32)]))
    scene = bytearray(r.bytes(0x20))
    put(scene, 8, 'I', r.choice([0, 0x4000000, r.getrandbits(32)]))
    regions = {RAM: mem, OBJECT: obj, 0x8bcbc0: put(bytearray(r.bytes(8)), 4, 'I', BUF), 0x8bd500: scene}
    mode = r.choice([0, 1, 2, 3, 4, 6, r.getrandbits(32)])
    kind = r.choice([0xc4, 0x1c4, 0x2c4, 0x3c4, 0, 0x1c3, 0x1c5, 0x4c4, 0xffffffc4, r.getrandbits(32)])
    flags = r.choice([0, 1, 0xff, 0xffffffff, r.getrandbits(32)])
    return Call(0x565240, [OBJECT, mode, r.getrandbits(32), kind, flags, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

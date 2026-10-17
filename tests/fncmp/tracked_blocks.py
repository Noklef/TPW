# The tracked handle of src/game/core/tracked_blocks against the original: handles in and out of the thing table, stamps around the expiry, things of
# any kind with the object's handle among their words.
# usage: tracked_blocks.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x5d46d0, 0x5d47a0), (0x4e0220, 0x4e02f0), (0x506580, 0x5065d0), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1)])
THINGS, CLOCK = RAM + 0x2000, RAM + 0x40000


def case(r):
    if r.random() < 0.3:  # Blocks7_Free: seven groups, each block pointer null or one the allocator handed out
        obj = bytearray(r.bytes(0x100))
        for k in range(7):
            put(obj, 0x10 * k + 0xc, 'I', r.choice([0, 0, HEAP + 0x40 + 0x10 * k, RAM + 0x180]))
        return Call(0x5d46d0, [RAM], {RAM: obj, HEAP: heap()}, conv='t', ret='v')
    obj = bytearray(r.bytes(0x100))
    handle = r.choice([0, 1, 2, 5, 11])
    mine = r.choice([0, 1, 0x1234, 0xffff])
    put(obj, 0, 'H', mine)
    put(obj, 0x5e, 'H', handle)
    stamp = r.choice([0, 100, 5000, 0xfffffff0, r.getrandbits(32)])
    put(obj, 0x60, 'I', stamp)
    slots = bytearray(r.bytes(20 * 12))
    things = bytearray(r.bytes(0x300 * 12))
    for h in range(12):
        put(slots, 20 * h, 'I', THINGS + 0x300 * h)
        things[0x300 * h + 2] = r.choice([0, 3, 4, 4, 5, 5, 6])
        put(things, 0x300 * h + 0x218, 'HH', mine if r.random() < 0.5 else r.getrandbits(16), mine if r.random() < 0.5 else r.getrandbits(16))
    now = (stamp + r.choice([0, 0x63, 0x64, 0x65, 1000, 0xffffffff, r.getrandbits(32)])) & 0xffffffff
    regions = {RAM: obj, 0x7cfb90: slots, THINGS: things, 0x80239c: struct.pack('<I', CLOCK - 0x1da70c), CLOCK: struct.pack('<I', now),
               RAM + 0x300: r.bytes(8)}
    return Call(0x4e0220, [RAM, RAM + 0x300], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

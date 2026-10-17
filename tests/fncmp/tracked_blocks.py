# The tracked handle of src/game/core/tracked_blocks against the original: handles in and out of the thing table, stamps around the expiry, things of
# any kind with the object's handle among their words; the tracked person's state (0x4e02f0) and the release of the tracked people of the world's
# object list (0x4dad20) on lists of things in any state.
# usage: tracked_blocks.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Blocks,Tracked,InState,Release)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x5d46d0, 0x5d47a0), (0x4e0220, 0x4e02f0), (0x506580, 0x5065d0), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1),
           (0x4e02f0, 0x4e0362), (0x5065d0, 0x506650), (0x4dad20, 0x4dadb7), (0x4e0210, 0x4e0220), (0x4e0370, 0x4e03f0)])
THINGS, CLOCK = RAM + 0x2000, RAM + 0x40000
OPS = [k for k in os.environ.get('OPS', 'Blocks,Tracked,InState,Release').split(',') if k]


def tracker(r, obj, at, mine):
    """An object's tracking fields at obj + at: its own handle, the tracked handle and the stamp."""
    put(obj, at, 'H', mine)
    put(obj, at + 0x5e, 'H', r.choice([0, 1, 2, 5, 11]))
    stamp = r.choice([0, 100, 5000, 0xfffffff0, r.getrandbits(32)])
    put(obj, at + 0x60, 'I', stamp)
    return stamp


def case(r):
    op = r.choice(OPS)
    if op == 'Blocks':  # Blocks7_Free: seven groups, each block pointer null or one the allocator handed out
        obj = bytearray(r.bytes(0x100))
        for k in range(7):
            put(obj, 0x10 * k + 0xc, 'I', r.choice([0, 0, HEAP + 0x40 + 0x10 * k, RAM + 0x180]))
        return Call(0x5d46d0, [RAM], {RAM: obj, HEAP: heap()}, conv='t', ret='v')
    obj = bytearray(r.bytes(0x100))
    mine = r.choice([0, 1, 0x1234, 0xffff])
    stamp = tracker(r, obj, 0, mine)
    slots = bytearray(r.bytes(20 * 12))
    things = bytearray(r.bytes(0x300 * 12))
    for h in range(12):
        put(slots, 20 * h, 'I', THINGS + 0x300 * h)
        things[0x300 * h + 2] = r.choice([0, 3, 4, 4, 5, 5, 6])
        put(things, 0x300 * h + 0x218, 'HH', mine if r.random() < 0.5 else r.getrandbits(16), mine if r.random() < 0.5 else r.getrandbits(16))
        put(things, 0x300 * h + 0x19c, 'I', r.choice([0xb, 0xd, 0xb, 0xd, 1, 2, 0, r.getrandbits(32)]))
    now = (stamp + r.choice([0, 0x63, 0x64, 0x65, 1000, 0xffffffff, r.getrandbits(32)])) & 0xffffffff
    world = bytearray(r.bytes(0x40))
    put(world, 0, 'I', now)
    regions = {RAM: obj, 0x7cfb90: slots, THINGS: things, 0x80239c: struct.pack('<I', CLOCK - 0x1da70c), CLOCK: world, RAM + 0x300: r.bytes(8)}
    if op == 'Tracked':
        return Call(0x4e0220, [RAM, RAM + 0x300], regions, conv='t', ret='a')
    if op == 'InState':
        return Call(0x4e02f0, [RAM], regions, conv='t', ret='a')
    # Release: a list of objects (handles 1-11, none twice) from the head at +0x1da746 (CLOCK + 0x3a), each tracking (or not) one of the things
    chain = r.sample(range(1, 12), r.choice([0, 1, 2, 4, 7]))
    for a, b in zip(chain, chain[1:] + [0]):
        at = 0x300 * a
        tracker(r, things, at, r.choice([mine, mine, r.getrandbits(16)]))
        put(things, at + 0x60, 'I', (now - r.choice([0, 0x64, 0x65, 1000, r.getrandbits(32)])) & 0xffffffff)
        put(things, at + 0xc, 'H', b)
        put(things, at + 0x64, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        put(things, at + 0x19c, 'I', r.choice([1, 2, 0, 4, 0xb, 0xd, r.getrandbits(32)]))
    put(world, 0x3a, 'H', chain[0] if chain else 0)
    return Call(0x4dad20, [RAM + r.choice([0, 0x80]), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

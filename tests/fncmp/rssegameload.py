# The RSSE module's save game read (src/game/core/savemods: RSSE_Load 0x5597a0) against the original, with RSSE_FreeContext and the
# occluder's callees run. The world is rsseload.py's (its ride box, sound occluder world, heap and import table); the system running or not,
# its list none or 1-2 empty records (one with a node of no type now and then) to be freed first. The stream (a record stream) as RSSE_Save
# writes it: 'RSSE' (mostly), the globals' size (0x14 mostly; 0, 4 or 0x18) and bytes, five pads, 0-3 records of the record size (0xf4
# mostly; 0 or 0x40), each one's +0xc8 none or the box's ride and +0xd4 an occluder or none, then its blocks (code, stack, variables,
# +0x90's, limbo, riders, walkers, +0x4c's, the folder) of 0 or a few bytes, 'OBJ ' (mostly), 0-3 nodes of 0x1c bytes (now and then
# 8 or 0x10). The stream whole, or cut short after the last record's node count and size (earlier, the original's counts would be
# uninitialised stack).
# usage: rssegameload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import RAM, Bench, Call, put, record_stream  # noqa: E402
import rsseload as rl  # noqa: E402

B = Bench(rl.merged(rl.RANGES + [(0x5597a0, 0x55a064), (0x558500, 0x5587ea), (0x5f5e20, 0x5f5e50), (0x5f5680, 0x5f5690)]))
OLD, STREAM, SINK = RAM + 0x36000, RAM + 0x37000, RAM + 0x38000


def image(r, ride):
    """the stream's bytes and the offset from which a cut leaves every count read whole"""
    d = bytearray(b'RSSE' if r.random() < 0.97 else r.bytes(4))
    gsize = r.choice([0x14] * 8 + [0, 4, 0x18])
    g = bytearray(r.bytes(gsize))
    if gsize:
        put(g, 0, 'I', r.choice([1, 1, 0]))
    d += struct.pack('<I', gsize) + g + b'PAD_' * 5
    n = r.choice([0, 1, 1, 2, 3])
    rsize = r.choice([0xf4] * 8 + [0x40, 0])
    d += struct.pack('<II', n, rsize)
    safe = len(d)
    for _ in range(n):
        rec = bytearray(r.bytes(rsize))
        if rsize > 0xd8:
            put(rec, 0xc8, 'I', r.choice([0, ride, ride]))
            put(rec, 0xd4, 'I', r.choice([0, r.getrandbits(32), r.getrandbits(32)]))
        d += rec

        def block(size):
            d.extend(struct.pack('<I', size) + r.bytes(size))

        block(r.choice([0, 4, 8, 16, 6]))       # the code
        block(r.choice([0, 0, 8, 12]))          # the stack
        block(r.choice([0, 0, 4, 8]))           # the variables
        block(r.choice([0, 3, 8]))              # +0x90's
        block(r.choice([0, 8, 16]))             # the limbo
        block(r.choice([0, 16, 32]))            # the riders
        w = r.choice([0, 1, 2])
        d += struct.pack('<I', w) + r.bytes(0x20 * w)   # the walkers
        block(r.choice([0, 4, 12]))             # +0x4c's
        block(r.choice([0, 5, 9]))              # the folder
        d += b'OBJ ' if r.random() < 0.97 else r.bytes(4)
        objs, nsize = r.choice([0, 1, 2, 3]), r.choice([0x1c] * 6 + [8, 0x10])
        d += struct.pack('<II', objs, nsize)
        safe = len(d)
        d += r.bytes(objs * nsize)
    return bytes(d), safe


def case(r):
    regions = rl.case(r).regions
    table = regions[0x7a4610]
    rides = [i for i in range(len(table) // 4) if struct.unpack_from('<I', table, 4 * i)[0] == rl.BOX]
    ride = rides[0] if rides else r.randrange(1, 8)
    data, safe = image(r, ride)
    if r.random() < 0.15:
        data = data[:r.randrange(safe, len(data) + 1)]
    regions.update({STREAM: record_stream(STREAM, SINK, len(data)), SINK: data + bytes(0x40)})
    # the system: running or not, its list none or 1-2 empty records (one with a node of no type now and then)
    g = bytearray(r.bytes(0x30))
    put(g, 0x10, 'I', r.choice([1, 1, 0]))
    old = r.choice([0, 1, 2])
    put(g, 0x20, 'I', OLD if old else 0)
    put(g, 0x28, 'I', 0)
    ctx = bytearray(0x200)
    for k in range(old):
        put(ctx, 0x100 * k, 'II', OLD + 0x100 * (k + 1) if k + 1 < old else 0, OLD + 0x100 * (k - 1) if k else 0)
        put(ctx, 0x100 * k + 8, 'I', r.getrandbits(32))
    nodes = bytearray(0x20)
    if old and r.random() < 0.3:
        put(ctx, 0xb0, 'I', OLD + 0x200)
    regions.update({0x879190: g, OLD: ctx, OLD + 0x200: nodes})
    regions.pop(rl.PREV, None)
    return Call(0x5597a0, [STREAM], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

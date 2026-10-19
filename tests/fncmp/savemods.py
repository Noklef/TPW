# The save game writers of src/game/core/savemods (Camera_Save 0x42cdc0, Particles_Save 0x51f680, FlyingRides_Save 0x55de70, RSSE_Save 0x559350) against the originals,
# through a record stream with room for the bytes or not: the sink compared after the call holds what was written. Also the readers Particles_Load 0x51f7a0
# and Camera_Load 0x42cec0 (which snaps the yaw when 0x78d912 is set).
# usage: savemods.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, fb, global_heap, heap, put, record_stream  # noqa: E402

# name: (address, returns, size)
CALLS = {'Cam': (0x42cdc0, 'b', 0x100), 'Part': (0x51f680, 'a', 0x120), 'Fly': (0x55de70, 'a', 0x1b0), 'Rsse': (0x559350, 'a', 0x450), 'PLoad': (0x51f7a0, 'a', 0x290),
         'CamLoad': (0x42cec0, 'a', 0x223)}
B = Bench([(a, a + n) for (a, _r, n) in CALLS.values()] + [(0x5f5e20, 0x5f5e50), (0x5f5e50, 0x5f5e80), (0x5f5660, 0x5f5690), (0x5da3c0, 0x5da3c1)])
STREAM, SINK, ARENA = RAM, RAM + 0x100, RAM + 0x300000


def room(r, need):
    """How many bytes the stream has left: plenty, none, or short of the whole at some point."""
    return r.choice([need + 0x100, need + 0x100, need + 0x100, 0, 4, 8, r.randrange(need + 1), r.randrange(0x20)])


def case(r):
    name = r.choice(OPS)
    address, ret, _n = CALLS[name]
    if name == 'PLoad':  # the module as Particles_Save writes it, then spoiled here and there; a stream cut before the count (read into a stack slot) is the only short one
        count = r.choice([0, 1, 2, 3, 4, 8])
        block1 = bytearray(r.bytes(0x9e68))
        put(block1, 0xc, 'I', count)
        head = r.choice([-1, -1, 0, 1, 2, 3, 3])
        put(block1, 0x9e4c, 'h', head)
        for k in range(4):
            put(block1, 0x2c + 0x140 * k + 0xd0, 'h', r.choice([-1, k + 1 if k < 3 else -1, 2]) if k != 2 else -1)
        pool = r.choice([0, 0, count * 0x34, count * 0x34, r.randrange(count * 0x34 + 1)])
        data = struct.pack('<III', r.choice([0x5054434c] * 9 + [0x1234]), r.choice([1, 1, 1, 1, 0, 7]), r.choice([0x9e68] * 9 + [0x9e69]))
        data += bytes(block1) + struct.pack('<I', r.choice([0x8b60] * 9 + [0x8b61])) + r.bytes(0x8b60) + struct.pack('<I', pool) + r.bytes(pool)
        cut = 12 + 0x9e68 + 4 - 1
        left = len(data) + 0x10 if r.random() < 0.8 else r.randrange(cut + 1)
        regions = {STREAM: record_stream(STREAM, SINK, left), SINK: data + bytes(0x10), HEAP: heap(), **global_heap(), 0x80ced0: r.bytes(0x9e68),
                   0x804370: r.bytes(0x8b60), 0x816d40: struct.pack('<I', r.choice([1, 1, 1, 0])) + bytes(0xc)}
        old = r.choice([0, 0, HEAP + 0x40])
        regions[0x80ced0] = bytearray(regions[0x80ced0])
        put(regions[0x80ced0], 0x28, 'I', old)
        return Call(address, [STREAM], regions, ret=ret)
    if name == 'Cam':
        left = r.choice([0x100, 0x100, 0x100, 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, r.randrange(0x40)])
        regions = {STREAM: record_stream(STREAM, SINK, left), SINK: r.bytes(0x100), 0x7908f0: r.bytes(0x200)}
        return Call(address, [STREAM], regions, ret=ret)
    if name == 'CamLoad':  # the 40 bytes Camera_Save writes (a yaw about the quarter turns, any, or special), the stream cut at each read or anywhere; the snap on or off
        q = r.randrange(-12, 13) * math.pi / 4
        yaw = r.choice([fb(q), fb(q + r.uniform(-0.01, 0.01)), fb(q + r.uniform(-1, 1)), fb(r.uniform(-1e4, 1e4)), r.f(), 0x80000000, 0x7f800000, 0x7fc00000, 0x7f800001])
        data = r.bytes(4) + struct.pack('<I', yaw) + r.bytes(32)
        left = r.choice([0x100, 0x100, 0x100, 0x100, 0, 4, 8, 12, 24, 36, r.randrange(0x30)])
        regions = {STREAM: record_stream(STREAM, SINK, left), SINK: data + r.bytes(0x18), 0x7908f0: r.bytes(0x200),
                   0x78d912: bytes([r.choice([0, 1, 1, r.getrandbits(8)])])}
        return Call(address, [STREAM], regions, ret=ret)
    if name == 'Part':
        full = r.choice([0, 1, 1, 7, 0xffffffff])
        flag = r.choice([0, 1, 1, 1, 5])
        count = r.choice([0, 1, 2, 3, 0x10])
        need = 0x20000
        regions = {STREAM: record_stream(STREAM, SINK, room(r, need)), SINK: r.bytes(need + 0x200), 0x816d40: struct.pack('<I', flag) + bytes(0xc),
                   0x80ced0: bytearray(r.bytes(0x9e68)), 0x804370: r.bytes(0x8b60), ARENA: r.bytes(0x1000)}
        put(regions[0x80ced0], 0xc, 'I', count)
        put(regions[0x80ced0], 0x28, 'I', ARENA)
        return Call(address, [STREAM, full], regions, ret=ret)
    if name == 'Fly':
        flag = r.choice([0, 1, 1, 1, 9])
        table = bytearray(400)
        put(table, 0, 'I', r.getrandbits(32))
        pointers = r.sample(range(99), r.choice([0, 1, 2, 3]))
        for k, slot in enumerate(pointers):
            put(table, 4 + 4 * slot, 'I', ARENA + 0x1230 * k)
        need = 0x1000 + 0x1230 * len(pointers)
        regions = {STREAM: record_stream(STREAM, SINK, room(r, need)), SINK: r.bytes(need + 0x200), 0x8791ec: struct.pack('<I', flag) + bytes(0xc),
                   0x8791f8: table, 0x7660b8: r.bytes(0x10), ARENA: r.bytes(0x1230 * 3)}
        return Call(address, [STREAM], regions, ret=ret)
    # Rsse: the flag and the list head, 0 to 2 contexts with their arrays, names and object lists carved from the arena
    flag = r.choice([0, 1, 1, 1, 3])
    arena = bytearray(r.bytes(0x4000))
    cursor = [0x10]

    def carve(size):
        at = cursor[0]
        cursor[0] += (size + 15) & ~15
        return at

    contexts = []
    for _ in range(r.choice([0, 1, 2])):
        ctx = carve(0xf4)
        contexts.append(ctx)
        code_len, stack_size, vars_n = r.choice([0, 1, 3]), r.choice([0, 2]), r.choice([0, 1, 4])
        unk90, limbo, riders, walkers, w4c = r.choice([0, 2]), r.choice([0, 1, 2]), r.choice([0, 1]), r.choice([0, 1, 2]), r.choice([0, 3])
        put(arena, ctx + 0x50, 'I', code_len)
        put(arena, ctx + 0x18, 'I', ARENA + carve(code_len * 4 + 4))
        put(arena, ctx + 0x54, 'I', stack_size)
        put(arena, ctx + 0x20, 'I', ARENA + carve(stack_size * 4 + 4))
        put(arena, ctx + 0x8c, 'I', vars_n)
        put(arena, ctx + 0x1c, 'I', ARENA + carve(vars_n * 4 + 4))
        put(arena, ctx + 0x90, 'I', unk90)
        put(arena, ctx + 0x34, 'I', ARENA + carve(unk90 + 4))
        put(arena, ctx + 0x58, 'I', limbo)
        put(arena, ctx + 0x24, 'I', ARENA + carve(limbo * 8 + 8))
        put(arena, ctx + 0x64, 'I', riders)
        put(arena, ctx + 0x28, 'I', ARENA + carve(riders * 16 + 16))
        put(arena, ctx + 0x7c, 'I', walkers)
        put(arena, ctx + 0x2c, 'I', ARENA + carve(walkers * 32 + 32))
        put(arena, ctx + 0x4c, 'I', w4c)
        put(arena, ctx + 0x30, 'I', ARENA + carve(w4c * 4 + 4))
        name_len = r.choice([0, 1, 5])
        at = carve(name_len + 1)
        arena[at:at + name_len] = bytes(r.choice(b'abcXYZ') for _ in range(name_len))
        arena[at + name_len] = 0
        put(arena, ctx + 0x38, 'I', ARENA + at)
        objects = [carve(0x1c) for _ in range(r.choice([0, 1, 2]))]
        for k, o in enumerate(objects):
            put(arena, o, 'I', ARENA + objects[k + 1] if k + 1 < len(objects) else 0)
        put(arena, ctx + 0xb0, 'I', ARENA + objects[0] if objects else 0)
    for k, ctx in enumerate(contexts):
        put(arena, ctx, 'I', ARENA + contexts[k + 1] if k + 1 < len(contexts) else 0)
    head = struct.pack('<5I', flag, 0, 0, 0, ARENA + contexts[0] if contexts else 0) + bytes(0xc)
    need = 0x2000
    regions = {STREAM: record_stream(STREAM, SINK, room(r, need)), SINK: r.bytes(need + 0x200), 0x8791a0: head, ARENA: arena}
    return Call(address, [STREAM], regions, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

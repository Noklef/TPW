# A coaster link destroyed (src/game/coaster/registry: 0x436fa0 CoasterLink_Destroy) against the original: the list at 0x790fe0 of 0-4
# links (+0x10 their ids, +0x18 next) holding the id asked for mostly (one link, now and then two with it: the first is taken), else
# not; the id table (0x790b04 used, 0x790b00 holes, 0x791ce0 next, 0x790bd0 the table) about the link's id (+0x14); the count 0x792214.
# The link: its train (+4) with 0-3 cars (+0x2e8), its cars (+0x128; 0 now and then), each with flag 2 (byte +0) or not (CoasterCar_Sleep
# over 0-2 strip-less segments past those the loop reads, the sounds off or not started as carwake.py's), a shape (+4) of 0-3 segments
# (+4) and segments (+0x28) whose rides (+0xc) are 0 or a ride with no holder (Ride_Sleep: nothing; Ride_Destroy 0x463c80 a cdecl
# recorder, 31, as queuepieces.py's); its blocks +0x148, +0x130, +0x128, +0x12c, the rings' buffers (+0xb4, +0xd4) and the link itself
# freed through the GlobalFree import (global_heap: the pointers hashed); no children (+0x38) and no entries (+0x4c).
# usage: linkdestroy.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_region, put  # noqa: E402
import carwake  # noqa: E402

B = Bench(sorted(set(carwake.RANGES + [(0x436fa0, 0x437144), (0x468810, 0x4688c0), (0x43b0c0, 0x43b0f0), (0x43eaf0, 0x43eb6b)])),
          hooks={**carwake.holder.HOOKS, 0x463c80: _recorder(31, 1, cdecl=True)})
LINKS, TRAIN, CARS, SHAPES, SEGS = RAM + 0x40000, RAM + 0x41000, RAM + 0x41400, RAM + 0x41800, RAM + 0x42000
IDS = 0x30


def case(r):
    regions = {HEAP: heap(), LOG: log_region(), **global_heap()}
    n = r.choice([0, 1, 1, 2, 3, 4, 4])
    want = r.randrange(1, 9)
    ids = [r.choice([r.randrange(1, 9), want + 1, 0]) for _ in range(n)]
    if n and r.random() < 0.85:
        ids[r.randrange(n)] = want
        if n > 1 and r.random() < 0.2:
            ids[r.randrange(n)] = want
    links = [LINKS + i * 0x200 for i in range(n)]
    nodes = list(links)
    regions[0x790fe0] = struct.pack('<I', nodes[0] if nodes else 0)
    count = r.choice([0, 1, 2, 3])
    shapes = []
    for k in range(4):
        shape = bytearray(r.bytes(8))
        put(shape, 4, 'I', r.choice([0, 1, 2, 3]))
        regions[SHAPES + k * 0x10] = shape
        shapes.append(shape)
    train = bytearray(r.bytes(0x300))
    put(train, 0x2e8, 'I', count)
    regions[TRAIN] = train
    cars = bytearray(r.bytes(0x80 * 3))
    for i in range(3):
        c = 0x80 * i
        k = r.randrange(4)
        cars[c] = r.getrandbits(8) & ~2 | r.choice([0, 2])
        put(cars, c + 4, 'I', SHAPES + k * 0x10)
        segs = SEGS + i * 0x400
        put(cars, c + 0x28, 'I', segs)
        put(cars, c + 8, 'I', 3)   # (Sleep's segments come after the loop's)
        put(cars, c + 0xc, 'I', r.choice([0, 0, 1, 2]))
        for o in (0x64, 0x68, 0x70):
            put(cars, c + o, 'I', r.choice([0, 0, r.getrandbits(32)]))
        data = bytearray(r.bytes(0x60 * 5))
        for s in range(5):
            put(data, 0x60 * s + 0xc, 'I', r.choice([0, r.randrange(1, 0x10)]) if s < 3 else 0)
        regions[segs] = data
    regions[CARS] = cars
    for i, at in enumerate(links):
        link = bytearray(r.bytes(0x150))
        put(link, 0x10, 'I', ids[i])
        put(link, 0x18, 'I', links[i + 1] if i + 1 < n else 0)
        put(link, 4, 'I', TRAIN)
        put(link, 0x14, 'I', r.choice([r.randrange(1, IDS - 8), 0]))
        put(link, 0x38, 'I', 0)
        put(link, 0x4c, 'I', 0)
        put(link, 0x128, 'I', r.choice([CARS, CARS, CARS, 0]))
        for o in (0x12c, 0x130, 0x148, 0xb4, 0xd4):
            put(link, o, 'I', r.choice([0, RAM + 0x50000 + o * 0x10]))
        link[0x3c] = r.getrandbits(8)
        regions[at] = link
    table = bytearray(IDS * 4)
    for k in range(IDS):
        put(table, 4 * k, 'I', r.choice([0, 0, RAM + 0x60000]))
    used = r.randrange(IDS // 2)
    holes = r.choice([0, 0, 1, 2, r.randrange(6)])
    regions.update({0x790bd0: table, 0x790b04: struct.pack('<I', used), 0x790b00: struct.pack('<I', holes),
                    0x791ce0: struct.pack('<I', r.randrange(IDS)), 0x792214: struct.pack('<I', r.choice([n, 1, 0])),
                    0x7a4610: bytes(0x40)})
    sound = bytearray(r.bytes(0x20))   # 0x802bc0..: on (+8) or started (+0x14) 0
    put(sound, r.choice([8, 0x14]), 'I', 0)
    regions.update({0x802bc0: sound, 0x8791a0: bytes(4), 0x8791b0: bytes(4), 0x785a2c: struct.pack('<I', r.choice([0, 0, 1])),
                    0x803a3c: r.bytes(4), 0x7b4960: r.bytes(0x40)})
    return Call(0x436fa0, [r.choice([want] * 6 + [0, want + 1])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

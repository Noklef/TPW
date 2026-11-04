# A model's sprite vertices turned to the camera (src/game/model/aimsprites: 0x44b510) against the original: the script slot's three setters recorders, the real matrix routines; faces with and without
# 0x400 and 0x8000 in both arrays. A NaN only in a vertex after the first that reaches the loop: the first with no winner reads the original's stack.
# usage: aimsprites.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

REG, POSITION, REGF = 13, 14, 15
CALLS = {0x475590: (REG, 3), 0x475480: (POSITION, 5), 0x4755d0: (REGF, 3)}
B = Bench([(0x44b510, 0x44ba70), (0x46f150, 0x46f190), (0x46f200, 0x46f220), (0x46fbb0, 0x46fc10), (0x46fd40, 0x470100), (0x4702d0, 0x4704d0), (0x475480, 0x4754f0), (0x475590, 0x475620), (0x67b410, 0x67b450), (0x5da3c0, 0x5da3c1)],
          hooks={addr: _recorder(k, n, cdecl=True) for addr, (k, n) in CALLS.items()}, sines=4096)
INSTANCE, HOLDER, BLOCK, MODEL, CAMERA = RAM, RAM + 0x100, RAM + 0x180, RAM + 0x200, RAM + 0x300
VERTICES, FACES, OTHERS, MATRICES = RAM + 0x400, RAM + 0x800, RAM + 0x1000, RAM + 0x1800
NAN = 0x7fc00001


def matrix(r):
    buf = bytearray(0x40)
    r.floats(buf, range(0, 0x40, 4), r.choice([1.0, 1.0, 10.0]))
    r.floats(buf, [0x30, 0x34, 0x38], r.choice([1.0, 20.0, 500.0]))
    if r.randrange(3) == 0:   # (a rotation about Y: an inverse that keeps its length)
        a = r.uniform(-3.2, 3.2)
        rows = [math.cos(a), 0, -math.sin(a), 0, 0, 1, 0, 0, math.sin(a), 0, math.cos(a), 0]
        put(buf, 0, '12f', *rows)
    put(buf, 0x3c, 'f', 1.0)
    return buf


def case(r):
    log = log_region()
    count = r.choice([0, 1, 1, 2, 3, 4])
    first, base = r.choice([0, 1, 2, 8]), r.choice([0, 0, 1, 3])
    model = bytearray(r.bytes(0x80))
    put(model, 0x44, '2H', first, base)
    put(model, 0x48, 'H', count)
    put(model, 0x70, '2I', FACES, OTHERS)
    regions = {LOG: log, INSTANCE: struct.pack('<I', HOLDER), HOLDER: put(put(bytearray(r.bytes(0x30)), 4, 'I', MODEL), 0x28, 'I', BLOCK), MODEL: model,
               BLOCK: put(put(bytearray(r.bytes(8)), 0, 'B', r.choice([4] * 15 + [0]) | r.getrandbits(8) & ~4), 4, 'I', VERTICES), 0x790a98: struct.pack('<I', CAMERA), CAMERA: matrix(r), 0x74ced0: r.bytes(4)}
    faces = bytearray(r.bytes(0xa0 * 8))
    others = bytearray(r.bytes(0x58 * 8))
    for i in range(8):
        for buf, at in ((faces, i * 0xa0), (others, i * 0x58)):
            put(buf, at, 'I', r.getrandbits(32) & ~0x8400 | r.choice([0, 0, 0, 0x400, 0x400, 0x8000, 0x8400]))
    regions[FACES], regions[OTHERS] = faces, others
    vertices = bytearray(r.bytes(0x14 * 4))
    reached = False
    identity = struct.pack('<16f', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)
    for i in range(4):
        at = i * 0x14
        put(vertices, at, 'B', r.getrandbits(8) & ~2 | r.choice([2, 2, 2, 0]))
        put(vertices, at + 4, 'I', MATRICES + i * 0x40)
        put(vertices, at + 0xc, 'I', r.getrandbits(16))
        m = matrix(r)
        if i < count and vertices[at] & 2:
            k = base + i
            flags = struct.unpack_from('<I', faces, k * 0xa0)[0] if k < first else struct.unpack_from('<I', others, (k - first) * 0x58)[0]
            if not flags & 0x8000:
                if reached and r.randrange(4) == 0:
                    put(m, r.choice([0x30, 0x34, 0x38]), 'I', NAN)
                elif reached and r.randrange(4) == 0:   # (where the camera is: no direction with the view whole, so the kept up shows in the roll)
                    m = identity
                    regions[CAMERA] = identity
                reached = True
        regions[MATRICES + i * 0x40] = m
    regions[VERTICES] = vertices
    return Call(0x44b510, [INSTANCE], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

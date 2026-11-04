# A rectangle of the terrain's texture tested against the view (src/game/terrain/procedural: 0x56e7e0) against the original, its own calls down the quarters included: the box's 8 corners go to the transform object's slot 1 (a recorder, which fills the answer: its first two points change between 3 records in turn); the clip codes the game's over a table of the case: none outside (the leaf: its columns' extents, the far distance noted), all outside (nothing) or some (quartered, down to a rectangle of area 1), NaN depths among them; empty and uneven rectangles.
# usage: terraintexcull.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, fill, log_region, put, strings, vtable  # noqa: E402

TRANSFORM = 1
B = Bench([(0x56e7e0, 0x56ea84), (0x56e780, 0x56e7dd), (0x561bc0, 0x561c3c)])
SELF, HEADER, OBJECT, OBJECT_VT = RAM, RAM + 0x400, RAM + 0x500, RAM + 0x600
FLOATS = [1.0, -1.0, 2.0, 0.5, 100.0, 0.0, -0.0, 3.5, 1e-3]


def depth(r):
    return r.choice(FLOATS + [r.uniform(-1000, 1000), r.uniform(-1000, 1000), float('nan')] if r.randrange(4) == 0 else FLOATS + [r.uniform(-1000, 1000)] * 4)


def point(r):
    return struct.pack('<4f', r.uniform(-4, 4), r.uniform(-4, 4), depth(r), r.uniform(-4, 4))


def case(r):
    log = log_region()
    self = bytearray(r.bytes(0x200))
    put(self, 0, 'I', HEADER)
    r.floats(self, [0x1c], r.choice([1.0, 1000.0]))
    x0, y0 = r.randrange(0x40), r.randrange(0x40)
    x1, y1 = x0 + r.choice([0, 1, 1, 2, 3, 4, 8]), y0 + r.choice([0, 1, 1, 2, 3, 4, 8])
    if r.randrange(16) == 0:
        y1 = y0 - 1
    header = bytearray(r.bytes(0x28))
    r.floats(header, [8, 0xc, 0x10, 0x14, 0x20, 0x24], r.choice([1.0, 100.0]))
    blob(log, TRANSFORM, 3, (1 << 24) - 1)
    strings(log, TRANSFORM, 0, 1 << 2)
    fill(log, TRANSFORM, 2, b''.join(point(r) + point(r) for _ in range(3)), records=3)
    fill(log, TRANSFORM, 2, b''.join(point(r) for _ in range(6)), which=1, offset=0x20)
    table = r.choice([bytes(0x80), bytes(0x80), b'\xff' * 0x80, b'\x80' * 0x80, bytes(r.choice([0, 0x80]) for _ in range(0x80)), bytes(r.choice([0, 0, 0, 1, 2, 4, 8, 0x80]) for _ in range(0x80)), bytes(r.choice([0, 1]) for _ in range(0x80))])
    regions = {LOG: log, SELF: self, HEADER: header, OBJECT: struct.pack('<I', OBJECT_VT) + r.bytes(0xc), OBJECT_VT: vtable({}, cdecl={TRANSFORM: 8}),
               0x87b1d8: struct.pack('<I', OBJECT), 0x87b088: r.bytes(4), 0x879a20: r.bytes(4), 0x879970: table}
    return Call(0x56e7e0, [SELF, x0, y0, x1, y1], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

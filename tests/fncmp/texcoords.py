# A model's texture coordinates (src/game/render/texcoords: 0x581410) against the original: entries of 0-12 vertices from 0-20,
# strides 8-0x28; no texture, one without flag 0x1000 or +0x1c, or one with any scales and offsets (widths and heights 0, negative or
# any); modes 1 (normals by the view), with bit 2 (shifted by the light's dots, the limit any, 0 or a NaN now and then) or neither;
# floats any (special values now and then).
# usage: texcoords.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, SPECIAL, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x581410, 0x581781)])
SETTINGS, ENTRIES, ORDER, NORMALS, TEX, VIEW, BLOCK, LIGHT = (RAM + 0x100 * k for k in range(8))
PACKS, EDGES, OUT = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000


def floats(r, n, scale=None):
    return b''.join(struct.pack('<I', r.f(scale)) for _ in range(n))


def case(r):
    index = r.randrange(4)
    entries = bytearray(r.bytes(0x40))
    count = r.choice([0, 1, 2, 3, 4, 5, 8, 12])
    first = r.randrange(1, 21)
    put(entries, 16 * index + 8, 'HH', first, first + count - 1)
    settings = bytearray(r.bytes(0x100))
    put(settings, 0x64, 'III', NORMALS, PACKS, ENTRIES)
    put(settings, 0x74, 'I', EDGES)
    put(settings, 0x94, 'I', ORDER)
    flags = r.getrandbits(32)
    flags = flags | 0x1000 if r.random() < 0.9 else flags & ~0x1000
    texture = bytearray(r.bytes(0x40))
    put(texture, 8, 'I', flags)
    put(texture, 0x1c, 'I', 0 if r.random() < 0.05 else r.getrandbits(32))
    put(texture, 0x14, 'hh', *(r.choice([0, -1, 1, 64, 256, r.randrange(-0x8000, 0x8000)]) for _ in range(2)))
    texture[0x20:0x30] = floats(r, 4, r.choice([1, 2, 100]))
    stride = r.choice([8, 0x20, 0x24, 0x28])
    mode = r.choice([1, 1, 2, 2, 6, 0, 4, r.getrandbits(32)])
    order = b''.join(struct.pack('<H', r.randrange(16)) for _ in range(0x80))
    limit = r.choice([fb(r.uniform(0, 2)), fb(1.0), 0, r.f(), r.choice(SPECIAL)])
    regions = {SETTINGS: settings, ENTRIES: entries, ORDER: order, NORMALS: floats(r, 0x30, 1), PACKS: floats(r, 0x50, 1),
               EDGES: floats(r, 0xc0, 2), TEX: texture, VIEW: floats(r, 16, 1), LIGHT: floats(r, 3, 1),
               BLOCK: put(bytearray(r.bytes(0x40)), 0x30, 'I', LIGHT), OUT: r.bytes(stride * 12 + 8),
               0x87afe8: struct.pack('<I', VIEW), 0x87a248: struct.pack('<I', BLOCK),
               0x8c1618: struct.pack('<II', r.f(2), limit)}
    tex = 0 if r.random() < 0.05 else TEX
    return Call(0x581410, [OUT, SETTINGS, index, tex, stride, mode, 0, 0], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

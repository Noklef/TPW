# The park map's coaster lines (src/game/world/parkmap: ParkMap_DrawCoasterLines 0x5f2fd0) against the original: up to 3 coasters of 0-3 nodes (a null list too) on a small surface of
# each depth (and a depth past 32, which draws nothing), the map's width any, the points on and off the surface, the coaster's float anywhere. The registry, the spline, the
# blend and the line drawers run for real on both sides.
# usage: parkmaplines.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x438720, 0x438770), (0x43e1b0, 0x43e330), (0x5edad0, 0x5f0610), (0x5f09a0, 0x5f0ae0), (0x5f2fd0, 0x5f3240), (0x5fa680, 0x5fa6b0), (0x5fd050, 0x5fd650), (0x6173e0, 0x6175d0), (0x5da3c0, 0x5da3c1)])
MAP, LINKS, HOLDERS, VALUES, NODES, PIXELS = RAM, RAM + 0x100, RAM + 0x300, RAM + 0x500, RAM + 0x600, RAM + 0x1000
W, H = 40, 60
DEPTHS = {8: (0xe0, 0x1c, 0x3, 0, 1), 16: (0xf800, 0x7e0, 0x1f, 0, 0), 24: (0xff0000, 0xff00, 0xff, 0, 0), 32: (0xff0000, 0xff00, 0xff, 0xff000000, 0)}


def case(r):
    depth = r.deal([8, 16, 24, 32, 15, 40])
    masks = DEPTHS[(depth + 7) // 8 * 8] if depth <= 32 else DEPTHS[32]
    n = (depth + 7) // 8 if depth <= 32 else 4
    pitch = W * n
    m = bytearray(0x54)
    put(m, 0x24, 'I', r.choice([r.randrange(12, 129)] * 12 + [r.randrange(1, 12), 0, 0x80000000, 0xffffffff]))
    put(m, 0x2c, '4I', *masks[:4])
    put(m, 0x3c, '6I', depth, masks[4], PIXELS, W, H, pitch)

    count = r.randrange(0, 4)
    links = bytearray(0x60 * 3)
    holders = bytearray(0xa0 * 3)
    values = bytearray(0x50 * 3)
    nodes = bytearray(0x60 * 9)
    for i in range(3):
        at = i * 0x60
        put(links, at + 0x18, 'I', LINKS + (i + 1) * 0x60 if i < 2 else 0)
        put(links, at + 0x28, 'I', VALUES + i * 0x50)
        put(links, at + 0x54, 'I', HOLDERS + i * 0xa0)
        length = r.choice([0, 1, 2, 2, 3, 3])
        put(holders, i * 0xa0 + 0x90, '3I', HOLDERS + i * 0xa0 + 0x94, 0, NODES + i * 3 * 0x60 if length else 0)
        put(values, i * 0x50 + 0x44, 'I', r.choice([fb(r.uniform(0, 300)), fb(r.uniform(-100, 100)), r.f()]))
        for j in range(3):
            node = (i * 3 + j) * 0x60
            put(nodes, node + 0x18, 'I', NODES + node + 0x60 if j + 1 < length else 0)
            for off in range(0x34, 0x58, 4):
                put(nodes, node + off, 'I', fb(r.uniform(-4, 40)) if r.random() > 0.01 else r.choice([0x7fc00000, 0x7fa00000, fb(500.0), fb(-500.0)]))
    regions = {MAP: m, LINKS: links, HOLDERS: holders, VALUES: values, NODES: nodes, PIXELS: r.bytes(H * pitch),
               0xf86bd8: r.bytes(0x6c), 0x790fe0: struct.pack('<I', LINKS), 0x792214: struct.pack('<I', count),
               0xfa7e30: bytes(16)}
    return Call(0x5f2fd0, [MAP], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

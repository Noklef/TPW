# The view's update (ViewCtl702bc8_Update, src/game/frontend/viewcontrols: 0x5dc860) against the original. An object of random bytes with ordinary floats mostly; the mouse at and past the
# screen's edges; the matrix the identity (so that the ray can be worked out here, and some of the list's things put within 10 of where it meets the sphere) or random; the camera near and
# far from the origin; Mat4_Transform real, Dialog7cc4c4_ShowThing a recorder (k 15).
# usage: viewupdate.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

SHOW = 15
B = Bench([(0x5dc860, 0x5dced0), (0x578d20, 0x578fea + 0x10), (0x5d5ee0, 0x5d5ef0)], hooks={0x4b9980: _recorder(SHOW, 1, cdecl=True)})
SELF, INPUT, TARGETS, CAMERA, MATRIX, LIST, NODES, VALUES, THINGS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x800, RAM + 0x1000


def world(r, x, y, aspect, regions, grazing):
    """the camera, the matrix and the list's things; grazing: the camera just outside the sphere, the ray straight at it, so that the near meeting is under 0.001 away"""
    reach = r.choice([5, 50, 110, 200, 400])
    p = [0, 0, -116.0005] if grazing else [r.uniform(-reach, reach) for _ in range(3)]
    camera = bytearray(r.bytes(0x40))
    put(camera, 8, '3f', *p)
    identity = grazing or r.random() < 0.8
    matrix = struct.pack('<16f', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1) if identity else struct.pack('<16f', *[r.uniform(-2, 2) for _ in range(16)])
    regions.update({CAMERA: camera, 0x87afe8: struct.pack('<I', MATRIX), MATRIX: matrix, 0xf83778: struct.pack('<I', CAMERA)})
    nx = (x * 0.0009765625 - 1) * 0.3862
    ny = ((0x600 - y) * 0.00130208337213 - 1) * aspect * 0.3862
    a = [nx - p[0], ny - p[1], 1 - p[2]]
    n = math.sqrt(sum(c * c for c in a)) or 1
    u = [c / n for c in a]
    t = -sum(p[i] * u[i] for i in range(3))
    missed = sum((-p[i] - t * u[i]) ** 2 for i in range(3))
    h = t - math.sqrt(max(13456 - missed, 0))
    if h < 0.001:
        h = t + math.sqrt(max(13456 - missed, 0))
    hit = [p[i] + u[i] * h for i in range(3)]
    count = r.choice([0, 1, 2, 3, 5])
    nodes, values, things = bytearray(0x100), bytearray(r.bytes(0x400)), bytearray(r.bytes(0x1000))
    for i in range(count):
        put(nodes, 8 * i, 'II', VALUES + 0x40 * i, NODES + 8 * (i + 1) if i + 1 < count else 0)
        put(values, 0x40 * i + 0xc, 'I', THINGS + 0x100 * i)
        near = r.choice([1, 3, 5, 8, 12, 500])
        put(things, 0x100 * i + 0x34, '3f', *[c + r.uniform(-near, near) for c in hit])
    lst = bytearray(r.bytes(8))
    put(lst, 4, 'I', NODES if count else 0)
    regions.update({LIST: lst, NODES: nodes, VALUES: values, THINGS: things})


def case(r):
    log = log_region()
    x = r.choice([r.randrange(0, 40), r.randrange(0, 2048), r.randrange(2000, 2060), 1024, -5, 3000, r.getrandbits(32) - (1 << 31)])
    y = r.choice([r.randrange(0, 30), r.randrange(0, 1536), r.randrange(1500, 1560), 768, -5, 2000, r.getrandbits(32) - (1 << 31)])
    grazing = r.random() < 0.05
    if grazing:
        x, y = 1024, 768
    aspect = r.uniform(0.5, 1.5)
    obj = bytearray(r.bytes(0x80))
    pick = r.choice([0, 1, 1, 1])
    limit = r.choice([0.5, 1.2, 1.5])
    floats = {0x10: r.pos(2), 0x18: r.pos(1), 0x34: r.f(100), 0x38: r.f(100), 0x3c: r.f(100), 0x40: struct.unpack('<I', struct.pack('<f', limit))[0] if r.random() < 0.9 else r.f(5),
              0x44: r.f(7), 0x48: r.f(5), 0x4c: r.f(2), 0x50: r.f(5), 0x54: r.f(200)}
    for off, bits in floats.items():
        put(obj, off, 'I', bits)
    put(obj, 0x58, 'I', r.choice([0, 1, 0x100, r.getrandbits(32)]))
    put(obj, 0x68, 'II', pick, LIST)
    inp = bytearray(r.bytes(0x40))
    put(inp, 8, 'I', r.pos(1))
    if r.random() < 0.05:  # no time passed, the angle on its limit: the speed stays
        put(inp, 8, 'I', 0)
        obj[0x4c:0x50] = obj[0x40:0x44]
    put(inp, 0x10, 'I', TARGETS)
    put(inp, 0x1c, 'I', r.getrandbits(32) & ~2 | r.choice([0, 2, 2]))
    put(inp, 0x20, 'ii', x, y)
    targets = bytearray(r.bytes(0x20))
    put(targets, 0x14, 'II', r.f(100), r.f(100))
    regions = {LOG: log, SELF: obj, INPUT: inp, TARGETS: targets, 0xf82884: struct.pack('<I', INPUT), 0x78d910: bytes([r.choice([0, 0, 1, r.getrandbits(8)])]), 0x8bcbcc: struct.pack('<f', aspect)}
    world(r, x, y, aspect, regions, grazing)
    return Call(0x5dc860, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

# The terrain's rectangle in view each frame (src/game/terrain/procedural: 0x56ea90) against the original, TerrainTex_CullRect and the column extents included (the world from terraintexcull.py), the far distance and the matrices' update recorders: the far distance set or 460, the whole land's flag, a view inside the land, about its edges and off it, small and large scales, NaNs now and then.
# usage: terraintexview.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, put, strings, vtable  # noqa: E402
from terraintexcull import OBJECT, OBJECT_VT, TRANSFORM, point  # noqa: E402

SETFAR, UPDATE = 2, 3
B = Bench([(0x56ea90, 0x56ee90), (0x56e7e0, 0x56ea84), (0x56e780, 0x56e7dd), (0x561bc0, 0x561c3c), (0x67a830, 0x67a860)], hooks={0x578ca0: _recorder(SETFAR, 1), 0x5783b0: _recorder(UPDATE, 0, cdecl=True)})
SELF, HEADER, VIEW = RAM, RAM + 0x400, RAM + 0x700
OPS = ['inside', 'inside', 'edge', 'off', 'whole', 'nan', 'shifted', 'touching']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    blob(log, TRANSFORM, 3, (1 << 24) - 1)
    strings(log, TRANSFORM, 0, 1 << 2)
    fill(log, TRANSFORM, 2, b''.join(point(r) + point(r) for _ in range(3)), records=3)
    fill(log, TRANSFORM, 2, b''.join(point(r) for _ in range(6)), which=1, offset=0x20)
    table = r.choice([bytes(0x80), bytes(0x80), b'\xff' * 0x80, bytes(r.choice([0, 0, 0, 1, 2, 4, 8, 0x80]) for _ in range(0x80)), b'\x80' * 0x80])
    big = table in (bytes(0x80), b'\xff' * 0x80)   # (no quartering: any size is quick)
    w, h = r.randrange(1, 0x7f if big else 8), r.randrange(1, 0x7f if big else 8)
    step = r.choice([1.0, 2.0, 5.0, 8.0, 10.0])
    ox, oz = r.uniform(-50, 50), r.uniform(-50, 50)
    scale = r.choice([1 / step, 1 / step, 0.2, 0.125, 1.0, 16.0])
    scale_z = scale * r.choice([1.0, 1.0, 0.5])
    far = r.choice([r.uniform(2, 100), r.uniform(100, 2000), 0.0])
    flags = r.getrandbits(32) & ~0x2020 | r.choice([0, 0x2000]) | (0x20 if op == 'whole' else 0)
    half = (far if flags & 0x2000 else 460.0) / 2
    if op == 'off':
        fx, fz = r.choice([-1e4, 1e4, ox]), r.choice([-1e4, 1e4])
    elif op == 'edge':
        fx, fz = ox + r.choice([0, w]) * step + r.uniform(-20, 20), oz + r.choice([0, h]) * step + r.uniform(-20, 20)
    elif op == 'shifted':   # (the land far from the view: the box's ends kept within a byte)
        ox, oz = r.choice([-1, 1]) * r.uniform(2000, 9000), r.choice([-1, 1]) * r.uniform(2000, 9000)
        fx, fz = r.uniform(-5, 5), r.uniform(-5, 5)
    elif op == 'touching':   # (the rectangle's high end on column or row 0)
        fx, fz = -half - 0.999 + r.uniform(0.01, 0.99) / scale, -half - 0.999 + r.uniform(0.01, 0.99) / scale_z
        if r.getrandbits(1):
            fx = r.uniform(0, w) * step
        elif r.getrandbits(1):
            fz = r.uniform(0, h) * step
    else:
        fx, fz = ox + r.uniform(0, w) * step, oz + r.uniform(0, h) * step
    self = put(bytearray(r.bytes(0x300)), 0, 'I', HEADER)
    put(self, 0x10, 'ff', scale, scale_z)
    header = bytearray(r.bytes(0x28))
    put(header, 8, 'ffff', ox, oz, step, step * r.choice([1.0, 1.0, 2.0]))
    put(header, 0x18, 'II', w, h)
    put(header, 0x20, 'ff', r.uniform(-10, 0), r.uniform(0, 30))
    view = bytearray(r.bytes(0x40))
    put(view, 8, 'f', r.uniform(-3, 3))
    put(view, 0x28, 'f', r.uniform(-3, 3))
    put(view, 0x30, 'f', fx)
    put(view, 0x38, 'f', fz)
    if op == 'nan':
        put(r.choice([view, self, header]), r.choice([8, 0x10, 0x14]), 'f', float('nan'))
    regions = {LOG: log, SELF: self, HEADER: header, VIEW: view, OBJECT: struct.pack('<I', OBJECT_VT) + r.bytes(0xc), OBJECT_VT: vtable({}, cdecl={TRANSFORM: 8}), 0x87b1d8: struct.pack('<I', OBJECT), 0x87b088: r.bytes(4), 0x879a20: r.bytes(4), 0x879970: table,
               0x8bcbc8: struct.pack('<I', flags), 0x8bd4c0: struct.pack('<f', far), 0x7014c0: struct.pack('<f', r.choice([1.0, 1000.0])), 0x87afe8: struct.pack('<I', VIEW), 0x8bd4e8: r.bytes(4)}
    return Call(0x56ea90, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

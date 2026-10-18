# Starting a draw batch (src/game/render/batch: 0x565240) against the original, over the draw buffer
# of tests/fncmp/drawbuffer.py (streams, batch slots in and around the table): modes with and
# without 2 and 4, masks that meet the flags or not, the scene flag set or not, the primitive kinds
# 0xc4, 0x1c4, 0x2c4, 0x3c4 and others. And the fog of a batch's vertices (0x565e70 on given vertices,
# 0x565ee0 on the batch's own): the render flags (0x8bcbc8) with and without fog (0x2000), colour fog (0x4000)
# and alpha fog (0x8000), the driver kind (0x8bd524) 1 or not, the fog and 0-4 vertices as vertexfog.py's.
# usage: batch.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Begin,Fog,FogBatch)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, bf, fb, put  # noqa: E402

B = Bench([(0x565240, 0x56530d), (0x582640, 0x582804), (0x565e70, 0x565f4d), (0x5747d0, 0x5749db)])
BUF, BATCHES, OBJECT = RAM + 0x20, RAM + 0x100, RAM + 0x1000
VERTICES, BATCH = RAM + 0x2000, RAM + 0x2400
FOG, SCRATCH = 0x8bd47c, 0xfb34a4


def begin(r):
    mem = bytearray(r.bytes(0x100 + 16 * 0x38))
    b = 0x20
    put(mem, b + 0, 'I', r.choice([0, 1, 2, 3, 4, 5, 6, 7, r.getrandbits(32)]))
    put(mem, b + 4, 'I', r.randrange(4))
    put(mem, b + 0x44, 'I', BATCHES)
    put(mem, b + 0x48, 'I', r.choice([0, 1, 8, 16, r.randrange(17)]))
    for off in (0x4c, 0x50, 0x54):
        put(mem, b + off, 'I', r.randrange(16))
    start = r.getrandbits(32)
    put(mem, b + 0x5c, 'I', start)
    put(mem, b + 0x3c, 'I', r.choice([start, start + 0x100, start + 0x10000, r.getrandbits(32)]) & 0xffffffff)
    obj = bytearray(r.bytes(0x60))
    put(obj, 0xc, 'I', r.choice([0, 0xffffffff, r.getrandbits(32)]))
    scene = bytearray(r.bytes(0x20))
    put(scene, 8, 'I', r.choice([0, 0x4000000, r.getrandbits(32)]))
    regions = {RAM: mem, OBJECT: obj, 0x8bcbc0: put(bytearray(r.bytes(8)), 4, 'I', BUF), 0x8bd500: scene}
    mode = r.choice([0, 1, 2, 3, 4, 6, r.getrandbits(32)])
    kind = r.choice([0xc4, 0x1c4, 0x2c4, 0x3c4, 0, 0x1c3, 0x1c5, 0x4c4, 0xffffffc4, r.getrandbits(32)])
    flags = r.choice([0, 1, 0xff, 0xffffffff, r.getrandbits(32)])
    return Call(0x565240, [OBJECT, mode, r.getrandbits(32), kind, flags, r.getrandbits(32)], regions, conv='t', ret='a')


def fog(r, op):
    start = r.uniform(-50, 200)
    end = fb(start + r.choice([r.uniform(1, 400), r.uniform(-50, 0)]))
    start = fb(start)
    span = bf(end) - bf(start)
    colour = [fb(r.choice([r.uniform(0, 255), float(r.randrange(256))])) for _ in range(3)]
    stride = r.choice([0x18, 0x20, 0x18, 0x10])
    count = r.choice([0, 1, 2, 3, 4])
    buf = bytearray(r.bytes(0x100))
    lo, hi = sorted((bf(start), bf(end)))
    for k in range(count):
        put(buf, k * stride + 8, 'I', r.choice([fb(r.uniform(lo - 20, hi + 20)), start, end, fb(r.uniform(lo, hi))]))
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x1c, 'I', stride)
    put(obj, 0x20, 'I', BATCH)
    batch = bytearray(r.bytes(0x40))
    put(batch, 0x20, 'I', count)
    put(batch, 0x28, 'I', VERTICES)
    render = r.getrandbits(32) & ~0xe000 | r.choice([0, 0x2000, 0x4000, 0x6000, 0x8000, 0xa000, 0xc000, 0xe000])
    regions = {VERTICES: buf, OBJECT: obj, BATCH: batch, SCRATCH: r.bytes(4), 0x8bcbc8: struct.pack('<I', render),
               0x8bd524: struct.pack('<I', r.choice([1, 1, 1, 0, 4, r.getrandbits(32)])),
               FOG: struct.pack('<I4I4I', r.getrandbits(32), r.getrandbits(32), *colour, start, end, fb(1 / span), r.getrandbits(32))}
    if op == 'Fog':
        return Call(0x565e70, [OBJECT, VERTICES, count], regions, conv='t')
    return Call(0x565ee0, [OBJECT], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', 'Begin,Fog,FogBatch').split(',') if k]


def case(r):
    op = r.deal(OPS)
    return begin(r) if op == 'Begin' else fog(r, op)


if __name__ == '__main__':
    B.main(case)

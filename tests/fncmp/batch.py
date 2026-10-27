# Starting a draw batch (src/game/render/batch: 0x565240) against the original, over the draw buffer
# of tests/fncmp/drawbuffer.py (streams, batch slots in and around the table): modes with and
# without 2 and 4, masks that meet the flags or not, the scene flag set or not, the primitive kinds
# 0xc4, 0x1c4, 0x2c4, 0x3c4 and others. And the fog of a batch's vertices (0x565e70 on given vertices,
# 0x565ee0 on the batch's own): the render flags (0x8bcbc8) with and without fog (0x2000), colour fog (0x4000)
# and alpha fog (0x8000), the driver kind (0x8bd524) 1 or not, the fog and 0-4 vertices as vertexfog.py's.
# A model's material batch (0x581880) over the same buffer, the batch object at [0x879ab0]: settings
# flags with and without 0xc0000 and 0x2000000, material entries 0-3 with a material or none, a texture set or none, the texture's index
# used or not (0x8bd550), textures 0 or with flags 0x400, 0x1000 and others, +0x3c 0 or not (as texformat.py's driver flags cases).
# A model's triangles added to a batch (0x581a00): see triangles().
# A batch's vertices transformed (0x565f50): fog()'s object and batch (counts 0-4), the transformer's slot 3 a recorder (cdecl, 5
# arguments) answering a float (no NaN), the batch's +0x30 any (a NaN now and then).
# usage: batch.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Begin,Model,Fog,FogBatch,Triangles,Transform)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, bf, fb, log_region, put, set_float_return, vtable  # noqa: E402

B = Bench([(0x565240, 0x56530d), (0x582640, 0x582804), (0x581880, 0x5819fd), (0x575f90, 0x575f9d), (0x5911d0, 0x591210), (0x581790, 0x581880),
           (0x590c40, 0x590ca8), (0x590fb0, 0x590fcb),
           (0x565e70, 0x565f4d), (0x565f50, 0x565f8a), (0x5747d0, 0x5749db), (0x581a00, 0x581cca)])
BUF, BATCHES, OBJECT = RAM + 0x20, RAM + 0x100, RAM + 0x1000
VERTICES, BATCH = RAM + 0x2000, RAM + 0x2400
FOG, SCRATCH = 0x8bd47c, 0xfb34a4


def buffer(r):
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
    regions = {RAM: mem, OBJECT: obj, 0x8bcbc0: put(bytearray(r.bytes(0x10)), 4, 'I', BUF), 0x8bd500: scene}
    return regions


MODEL, SETTINGS, ENTRIES, MATERIAL, SET, TEXTURES, TEXTURE = (RAM + 0x2000 + 0x100 * k for k in range(7))


def model(r):
    regions = buffer(r)
    scene = bytearray(regions[0x8bd500]) + r.bytes(0x40)
    put(scene, 0x50, 'B', r.choice([0, 0x40, r.getrandbits(8)]))   # (0x8bd550)
    put(scene, 0x24, 'I', r.choice([1, 1, 0, 4]))   # (0x8bd524, as texformat.py)
    regions[0x8bd500] = scene
    regions[0x879ab0] = struct.pack('<I', OBJECT)
    m = bytearray(r.bytes(0x10))
    put(m, 4, 'I', SETTINGS)
    regions[MODEL] = m
    settings = bytearray(r.bytes(0x70))
    put(settings, 0, 'I', r.getrandbits(32) & r.choice([0, 0xc0000, 0x2000000, 0x40000, 0xffffffff]))
    put(settings, 0x6c, 'I', ENTRIES)
    regions[SETTINGS] = settings
    entries = bytearray(r.bytes(0x40))
    for k in range(4):
        put(entries, 0x10 * k, 'I', r.choice([MATERIAL, MATERIAL, MATERIAL, 0]))
    regions[ENTRIES] = entries
    material = bytearray(r.bytes(8))
    put(material, 2, 'H', r.randrange(4))
    put(material, 4, 'I', r.choice([SET, SET, SET, 0]))
    regions[MATERIAL] = material
    regions[SET] = put(bytearray(r.bytes(0x10)), 8, 'I', TEXTURES)
    regions[TEXTURES] = b''.join(struct.pack('<I', r.choice([TEXTURE, TEXTURE, 0])) for _ in range(4))
    t = bytearray(r.bytes(0x60))
    put(t, 8, 'I', r.getrandbits(32) & r.choice([0, 0x400, 0x1000, 0x1400, 0x30000, 0x100000, 0xffffffff]))
    put(t, 0x3c, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[TEXTURE] = t
    regions[RAM + 0x2800] = r.bytes(4)
    return Call(0x581880, [MODEL, r.randrange(4), RAM + 0x2800], regions, conv='t', ret='a')


def begin(r):
    regions = buffer(r)
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
    if op == 'Transform':   # (the transformer's slot 3 a recorder answering a float)
        log = log_region()
        x = bf(r.f())
        # (no NaN answer: the emulator's recorder pushes it as a Python float, its payload lost)
        set_float_return(log, 3, r.choice([0.0, 1.5, -2.25, r.uniform(-1e4, 1e4), x if x == x else float('inf')]))
        put(batch, 0x30, 'I', r.choice([fb(r.uniform(-1e4, 1e4)), fb(0.0), r.f(), 0x7fc00000, 0x7f800001]))
        regions.update({LOG: log, TXF: struct.pack('<I', TVT), TVT: vtable({}, cdecl={3: 5}), 0x87b1d8: struct.pack('<I', TXF)})
        return Call(0x565f50, [OBJECT, r.getrandbits(32)], regions, conv='t')
    return Call(0x565ee0, [OBJECT], regions, conv='t')


TMODEL, TSETTINGS, TENTRIES, TFACES, TREMAP, TSRC, TBATCH, TVERTS, TINDICES, TXF, TVT = (RAM + 0x10000 + 0x1000 * k for k in range(11))


def triangles(r):
    """0x581a00 (thiscall (batch, matrix, vertices), pops 12): a model whose settings give 0-12 vertices (+0x58) and 0-3 material entries
    (+0x5a), each 0-6 faces from a start (corners its base +8 plus 0-31, through a remap table of 32 into the batch's vertices); the batch's
    counts small, the scale (0x8bd4ac) small or any; the source's x, y and w floats in -100..100 or any (a NaN now and then, in some cases on about every other one); the transformer
    ([0x87b1d8]) a recorder (slot 2 cdecl)."""
    n, groups = r.choice([0, 1, 3, 4, 8, 12, r.randrange(13)]), r.choice([0, 1, 2, 3, 1, 2])
    settings = bytearray(r.bytes(0x98))
    put(settings, 0x58, 'HH', n, groups)
    put(settings, 0x6c, 'I', TENTRIES)
    put(settings, 0x70, 'I', TFACES)
    put(settings, 0x94, 'I', TREMAP)
    base = r.choice([0, 0, 1, 4, r.randrange(8)])
    entries = bytearray(r.bytes(0x30))
    vb = r.choice([0, 0, 3, 0x100, r.getrandbits(15)])
    for g in range(3):
        put(entries, 0x10 * g + 4, 'HHH', r.randrange(8), r.choice([0, 1, 2, 4, 6, r.randrange(7)]), vb + r.choice([16, 16, r.randrange(17)]))
    faces = bytearray(r.bytes(0x70))
    for k in range(14):
        put(faces, 8 * k + 2, 'HHH', *(vb + 16 + r.randrange(16) for _ in range(3)))
    reach = base + n * groups
    remap = b''.join(struct.pack('<H', r.choice([r.randrange(max(reach, 1)), r.randrange(n or 1), r.randrange(48)])) for _ in range(32))
    nans = r.choice([0.03] * 7 + [0.5])   # (in some cases a NaN on about every other corner and coordinate)

    def coord():
        return r.choice([fb(r.uniform(-100, 100)), fb(r.uniform(-100, 100)), fb(r.uniform(-2, 2)), r.f()]) if r.random() >= nans else \
            r.choice([0x7f800001, 0xffa00000, 0x7fc00000])   # (a signalling NaN goes through the x87 as the first x)
    src = b''.join(struct.pack('<4I', coord(), coord(), coord(), coord()) for _ in range(12))
    verts = b''.join(struct.pack('<4I', coord(), coord(), coord(), coord()) + r.bytes(0x10) for _ in range(48))
    batch = bytearray(r.bytes(0x40))
    put(batch, 0x20, 'IIII', base, r.choice([0, 0, 3, r.randrange(16)]), TVERTS, TINDICES)
    regions = {TMODEL: put(bytearray(r.bytes(0x10)), 4, 'I', TSETTINGS), TSETTINGS: settings, TENTRIES: entries, TFACES: faces, TREMAP: remap,
               TSRC: src, TBATCH: batch, TVERTS: verts, TINDICES: r.bytes(0x100), TXF: struct.pack('<I', TVT), TVT: vtable({}, cdecl={2: 7}),
               LOG: log_region(), 0x87b1d8: struct.pack('<I', TXF), 0x8c63b0: r.bytes(0x24),
               0x8bd4ac: struct.pack('<i', r.choice([1, 1, 2, -1, 0, r.randrange(-1000, 1000), r.getrandbits(31)]))}
    return Call(0x581a00, [TMODEL, TBATCH, r.getrandbits(32), TSRC], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Begin,Model,Fog,FogBatch,Triangles,Transform').split(',') if k]


def case(r):
    op = r.deal(OPS)
    if op == 'Begin':
        return begin(r)
    if op == 'Model':
        return model(r)
    if op == 'Triangles':
        return triangles(r)
    return fog(r, op)


if __name__ == '__main__':
    B.main(case)

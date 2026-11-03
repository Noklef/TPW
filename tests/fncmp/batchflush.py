# A batch closed and flushed (src/game/render/batch: Batch_Close 0x5658c0, Batch_Flush 0x565590; src/game/render/depthsort: DepthSort_TrianglesFloat 0x565490) against the
# original, with the fog, the packer and the integer sort running as they are. DrawBuffer_Begin 0x582670 (k 1, its answer a block whose +0x28 on holds the keys), DrawBuffer_End
# 0x582780 (k 2), the transformer's slot 3 (k 3, cdecl, a float answer, no NaN: fncmp's recorder loses its payload) and the object's slot 4 (k 4) are recorders. The world: the
# flags that choose each step on or off, the driver kind 1, 4 or neither, 0-59 triangles over 16 vertices, equal keys and NaNs (when no fog reads them).
# usage: batchflush.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, bf, fb, log_region, put, set_float_return, set_return, vtable  # noqa: E402

B = Bench([(0x5653d0, 0x56548b), (0x565490, 0x565558), (0x565590, 0x5658b9), (0x5658c0, 0x565929), (0x5747d0, 0x5749db), (0x563130, 0x563213)],
          hooks={0x582670: _recorder(1, 1), 0x582780: _recorder(2, 2)})
OBJECT, VT, BATCH, TXF, TVT, VERTS, INDICES, BLOCK = RAM, RAM + 0x40, RAM + 0x100, RAM + 0x200, RAM + 0x210, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000
BUFFER = 0x12345678


def bits(r, *choices):
    out = 0
    for bit in choices:
        out |= r.choice([0, bit])
    return out


def sort_case(r):
    """the float sort alone: keys with ties, NaNs quiet and signalling; lo and hi inside them, or crossed."""
    n = r.randrange(1, 41)
    keys = [fb(r.uniform(-50, 50)) for _ in range(n)]
    if r.random() < 0.5:
        keys = [r.choice(keys[:4]) for _ in range(n)]
    if r.random() < 0.4:
        for _ in range(r.randrange(1, 4)):
            keys[r.randrange(n)] = r.choice([0x7fc00000, 0xffc12345, 0x7f800001, 0xffa00000, 0x7f800000, 0xff800000])
    lo = r.randrange(n) if r.random() < 0.2 else 0
    hi = r.randrange(lo, n) if r.random() < 0.2 else n - 1
    return Call(0x565490, [BLOCK + 8, INDICES, lo, hi], {BLOCK: r.bytes(8) + struct.pack('<%dI' % n, *keys) + r.bytes(8), INDICES: r.bytes(6 * n)}, ret='v')


def case(r):
    if r.random() < 0.2:
        return sort_case(r)
    log = log_region()
    close = r.random() < 0.4
    stride = r.choice([0x18, 0x20, 0x28, 0x30])
    count, triangles = r.choice([0, 1, 2, 3, 4, 8, r.randrange(13)]), r.choice([0, 1, 2, 3, 8, 40, r.randrange(60)])
    flags = bits(r, 0x40, 0x20, 0x400, 0x100, 0x200, 0x10, 0xf)
    render = r.getrandbits(32) & ~0x800e002 | r.choice([0, 0x4000, 0x8000, 0xc000]) | r.choice([0, 0, 2, 0x8000000])
    kind = r.choice([1, 1, 1, 4, 4, 4, 0, 2])
    fogged = flags & 0x40 and kind == 1
    start = r.uniform(-50, 200)
    end = fb(start + r.uniform(1, 400))
    start = fb(start)
    lo, hi = bf(start), bf(end)
    colour = [fb(float(r.randrange(256))) for _ in range(3)]
    verts = bytearray(r.bytes(0x300))
    shared = fb(r.uniform(lo, hi))
    for k in range(16):
        x = fb(r.choice([r.uniform(-500, 500), r.randrange(-500, 500) + 0.5, float(r.randrange(-500, 500))]))
        z = r.choice([shared, shared, fb(r.uniform(lo - 20, hi + 20)), start, end])
        if not fogged and r.random() < 0.1:
            z = r.choice([0x7fc00000, 0x7f800001, 0xffa00000, 0x7f800000, r.f()])
        put(verts, k * stride, 'I', x)
        put(verts, k * stride + 4, 'I', r.choice([x, r.f() if not fogged else x]))
        put(verts, k * stride + 8, 'I', z)
    indices = b''.join(struct.pack('<H', r.randrange(16)) for _ in range(180))
    batch = bytearray(r.bytes(0x40))
    put(batch, 0, 'I', flags)
    put(batch, 4, 'I', r.choice([0, 0, 0, 0x800, 0x1000, 0x100000, r.getrandbits(32) & ~0x800]))
    put(batch, 0x20, 'IIII', count, triangles * 3 + r.choice([0, 0, 0, 1, 2]), VERTS, INDICES)
    put(batch, 0x30, 'I', r.choice([fb(r.uniform(-1e4, 1e4)), 0, r.f(), 0x7fc00000, 0x7f800001]))
    obj = bytearray(r.bytes(0x40))
    put(obj, 0, 'I', VT)
    put(obj, 0xc, 'I', r.choice([0, 0, 0, 0xffffffff, r.getrandbits(32)]))
    put(obj, 0x1c, 'I', stride)
    put(obj, 0x20, 'I', BATCH)
    x = bf(r.f())
    set_float_return(log, 3, r.choice([0.0, 1.5, r.uniform(-1e4, 1e4), x if x == x else float('inf')]))
    set_return(log, 4, r.choice([0, 1, r.getrandbits(32)]))
    set_return(log, 1, BLOCK)
    regions = {LOG: log, OBJECT: obj, VT: vtable({4: 1}), BATCH: batch, TXF: struct.pack('<I', TVT), TVT: vtable({}, cdecl={3: 5}), VERTS: verts, INDICES: indices,
               BLOCK: r.bytes(0x130), 0x87b1d8: struct.pack('<I', TXF), 0x8bcbc4: struct.pack('<II', BUFFER, render), 0x8bd524: struct.pack('<I', kind),
               0x8bd508: struct.pack('<I', r.choice([0, 0x80000020, r.getrandbits(32)])), 0xfb34a0: r.bytes(8),
               0x8bd47c: struct.pack('<I4I4I', r.getrandbits(32), r.getrandbits(32), *colour, start, end, fb(1 / (hi - lo)), r.getrandbits(32))}
    if not close:
        return Call(0x565590, [OBJECT], regions, conv='t', ret='a')
    if r.random() < 0.1:
        put(obj, 0x20, 'I', 0)
    return Call(0x5658c0, [OBJECT, r.choice([fb(-1.0), fb(-1.0), 0x7fc00000, fb(r.uniform(-1e4, 1e4)), r.f()])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

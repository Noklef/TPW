# One textured quad (Batch_DrawQuad, src/game/render/batchquad: 0x565f90) against the original: the transformer's slot 3 is a cdecl recorder (k 3); Batch_Begin (k 11), DrawBatch_AddPolygon (k 12),
# DrawBuffer_End (k 13) and Batch_Flush (k 14) are recorders; the clip codes and the fog run as they are. A polygon's hash is of its vertices (dwords 5, 13, 21 and 29 are unset in the original too)
# or of the codes it is given: one in a case, the other argument logged as 0. The world: points any, the clip table and the mask, the render flags' fog bits, the object's batch the one begun or another.
# usage: batchquad.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, log_region, put, set_float_return, set_return, strings, vtable  # noqa: E402

B = Bench([(0x565f90, 0x566218), (0x561bc0, 0x561c40), (0x5747d0, 0x5749db)],
          hooks={0x565240: _recorder(11, 5), 0x565bb0: _recorder(12, 5), 0x582780: _recorder(13, 2), 0x565590: _recorder(14, 0)})
OBJECT, SPEC, POINTS, BATCH, OTHER, VERTS, MASK, XF, XFT = (RAM + 0x1000 * k for k in range(9))
VERTICES = 0xffffffff & ~(1 << 5 | 1 << 13 | 1 << 21 | 1 << 29)
NANS = [0x7fc12345, 0x7f812345, 0xffc00000, 0x7f800000, 0xff812345]


def fnum(r, lo, hi, nan=0.1):
    return struct.pack('<I', r.choice(NANS)) if r.random() < nan else struct.pack('<f', r.uniform(lo, hi))


def batch(r, verts):
    b = bytearray(r.bytes(0x38))
    put(b, 0x20, 'II', r.deal([0, 1, 2, 4, 4, 4]), r.deal([0, 3, 6, 6, 6]))
    put(b, 0x28, 'I', verts)
    put(b, 0x30, 'I', r.choice([0, struct.unpack('<I', fnum(r, -100, 100))[0]]))
    return b


def case(r):
    log = log_region()
    render = r.getrandbits(32) & ~0xe000 | r.deal([0, 0x2000, 0x6000, 0xa000, 0xe000, 0x4000, 0x8000])
    spec = bytearray(r.bytes(0x2c))
    put(spec, 4, 'I', POINTS)
    for off in (0x18, 0x1c, 0x20, 0x24):
        spec[off:off + 4] = fnum(r, 0, 1)
    if r.random() < 0.5:
        put(spec, 0, 'I', struct.unpack_from('<I', spec, 0)[0] | 0x400)
    points = b''.join(fnum(r, -10, 10, 0.03) for _ in range(16))
    table = r.deal(['zero', 'zero', 'few', 'few', 'all', 'any'])
    clip = bytes(128) if table == 'zero' else bytes(r.choice([0, 0, 1, 2, 4, 0x20, 0x13]) for _ in range(128)) if table == 'few' else b'\xff' * 128 if table == 'all' else r.bytes(128)
    mask = r.choice([0xfffffffd, 0xff, 0, 0xfffffffd, r.getrandbits(32)])
    other = r.random() < 0.2
    if r.random() < 0.5:                                                 # (the hash: of the vertices or of the codes; a fill is not used, so the order against it is no matter)
        strings(log, 12, 0, out=1 << 3)
        blob(log, 12, 0, VERTICES)
    else:
        strings(log, 12, 0, out=1 << 0)
        blob(log, 12, 3, 1)
    set_return(log, 11, r.deal([BATCH, BATCH, BATCH, BATCH, BATCH, BATCH, 0]))
    set_return(log, 12, r.deal([1, 1, 1, 1, 1, r.getrandbits(32) | 1, 0]))
    set_return(log, 14, r.getrandbits(32))
    set_float_return(log, 3, r.choice([0.0, 1.5, -2.25, r.uniform(-1e4, 1e4), float('inf')]))
    obj = bytearray(r.bytes(0x30))
    put(obj, 0x1c, 'I', 0x20)
    put(obj, 0x20, 'I', OTHER if other else BATCH)
    start, end = r.uniform(-10, 10), r.uniform(-10, 10)
    fog = struct.pack('<2I3f3fI', r.getrandbits(32), r.getrandbits(32), r.uniform(0, 255), r.uniform(0, 255), r.uniform(0, 255), start, end, 1 / ((end - start) or 1.0), r.getrandbits(32))
    verts = bytearray(r.bytes(0x20 * 4))
    for i in range(4):
        verts[i * 0x20 + 8:i * 0x20 + 12] = fnum(r, -20, 20, 0.02)
    regions = {LOG: log, OBJECT: bytes(obj), SPEC: bytes(spec), POINTS: points, BATCH: bytes(batch(r, VERTS)), OTHER: bytes(batch(r, VERTS)), VERTS: bytes(verts), MASK: r.bytes(0x30),
               XF: struct.pack('<I', XFT), XFT: vtable({}, cdecl={3: 5}), 0x87b1d8: struct.pack('<I', XF), 0x87a248: struct.pack('<I', MASK), 0x879a20: struct.pack('<I', r.getrandbits(32)),
               0x879970: clip, 0x8bcbc4: struct.pack('<II', r.getrandbits(32), render), 0x8bd47c: fog}
    return Call(0x565f90, [OBJECT, SPEC, mask], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

# A quad added to a batch (Batch_DrawQuadInPlace, src/game/render/batchquad: 0x566220) against the original: four points near the view volume (clippoly.py's), the clip table sparse bits or the
# index's own, any mask, so that a quad is left out, written straight in or clipped by the real DrawBatch_AddPolygon; the object's ends at, under and past what is needed; the fog bits any, the
# fog's code real; Batch_Begin (k 11) and the transformer's slot 3 (k 3) recorders. An object without a batch: a recorder's Batch_Begin does not set one, so an answer other than 0 faults in both.
# usage: batchquadplace.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import clippoly  # noqa: E402
import drawbatchclip  # noqa: E402
from batchquad import fnum  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, fb, log_region, put, set_float_return, set_return, vtable  # noqa: E402

B = Bench([(0x566220, 0x5666a8), (0x561bc0, 0x562f80), (0x565bb0, 0x565e80), (0x5747d0, 0x5749f0), (0x582670, 0x582704), (0x5827d0, 0x582820)], hooks={0x565240: _recorder(11, 5)})
OBJECT, SPEC, POINTS, BATCH, MASK, XF, XFT, OUT, VERTS = (RAM + 0x1000 * k for k in range(9))


def case(r):
    log = log_region()
    render = r.getrandbits(32) & ~0xe000 | r.deal([0, 0x2000, 0x6000, 0xa000, 0xe000, 0x4000, 0x8000])
    spec = bytearray(r.bytes(0x2c))
    put(spec, 4, 'I', POINTS)
    for off in (0x18, 0x1c, 0x20, 0x24):
        spec[off:off + 4] = fnum(r, 0, 1)
    points = bytearray()
    for p in clippoly.polygon(r, 4):
        for k in range(4):
            points += struct.pack('<I', fb(p[k]))
    if r.random() < 0.05:
        put(points, 4 * r.randrange(16), 'I', r.f(special=1))
    count, used = r.choice([0, 3, 0x20, 0x40]), r.choice([0, 3, 0x10])
    batch = bytearray(r.bytes(0x38))
    put(batch, 0x20, 'IIII', count, used, VERTS, OUT)
    put(batch, 0x30, 'I', r.choice([0, struct.unpack('<I', fnum(r, -100, 100))[0]]))
    need_v, need_i = (count + 10) * 0x20 + VERTS, OUT + 2 * used + 0x30
    exact_v, exact_i = (count + 4) * 0x20 + VERTS, OUT + 2 * used + 0xc
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x1c, 'II', 0x20, BATCH if r.random() < 0.92 else 0)
    put(obj, 0x28, 'II', r.choice([need_v, need_v, need_v + 0x100, exact_v, exact_v - 1]), r.choice([need_i, need_i, need_i + 0x40, exact_i, exact_i - 1]))
    set_return(log, 11, r.choice([0, 0, 0, BATCH]))
    set_float_return(log, 3, r.choice([0.0, 1.5, -2.25, r.uniform(-1e4, 1e4)]))
    start, end = r.uniform(-10, 10), r.uniform(-10, 10)
    fog = struct.pack('<2I3f3fI', r.getrandbits(32), r.getrandbits(32), r.uniform(0, 255), r.uniform(0, 255), r.uniform(0, 255), start, end, 1 / ((end - start) or 1.0), r.getrandbits(32))
    regions = drawbatchclip.globs(r)
    regions.update({LOG: log, OBJECT: obj, SPEC: spec, POINTS: points, BATCH: batch, VERTS: r.bytes(0x1000), OUT: r.bytes(0x100), MASK: r.bytes(0x30),
                    XF: struct.pack('<I', XFT), XFT: vtable({}, cdecl={3: 5}), 0x87b1d8: struct.pack('<I', XF), 0x87a248: struct.pack('<I', MASK),
                    0x8bcbc4: struct.pack('<II', r.getrandbits(32), render), 0x8bd47c: fog})
    mask = r.choice([0, 0xc0, 0x3f, 1 << r.randrange(6), r.getrandbits(32), 0xfffffffd])
    return Call(0x566220, [OBJECT, SPEC, mask], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

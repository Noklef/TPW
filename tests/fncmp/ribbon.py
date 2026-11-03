# A ribbon's next point (src/game/render/ribbon: Ribbon_AddPoint 0x57dd10) against the original. The projection's slot 1 ([0x87b1d8], cdecl, k 1 of 8: its input
# hashed, its output a fill) and DrawBatch_AddPolygon 0x565bb0 (k 2, thiscall of 5: the quad hashed without each vertex's +0x14, which it never sets) are recorders;
# Mat4_Transform runs as it is. The world: the object's current ribbon with 0 or more points, from and to (from none, or the same place), identity matrices with a
# last point at to (a step of no length), no width and no sideways (a sum of no length).
# usage: ribbon.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fb, fill, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x57dd10, 0x57e4b0), (0x578d20, 0x579004)], hooks={0x565bb0: _recorder(2, 5)})
OBJECT, MATRIX, FROM, TO, BATCH, PROJECTION, TABLE = RAM, RAM + 0xb000, RAM + 0xb040, RAM + 0xb060, RAM + 0xb080, RAM + 0xb090, RAM + 0xb0a0
QUAD = 0xffffffff & ~(1 << 5 | 1 << 13 | 1 << 21 | 1 << 29)
IDENTITY = [1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0]


def floats(r, n):
    return [r.f() for _ in range(n)]


def case(r):
    log = log_region()
    obj = bytearray(r.bytes(0xa1c0))
    ribbon = r.randrange(16)
    put(obj, 0xa188, 'I', ribbon)
    put(obj, 0x9da4 + ribbon * 4, 'I', r.deal([0, 0, 1, 2, 5, 0x7fffffff, 0xffffffff]))
    last, left, right, side = floats(r, 4), floats(r, 4), floats(r, 4), floats(r, 2)
    width = r.f()
    frm, to = floats(r, 4), floats(r, 4)
    identity = r.random() < 0.3
    matrix = [fb(v) for v in IDENTITY] if identity else floats(r, 16)
    if identity and r.random() < 0.5:   # a step of no length
        last[:2] = to[:2]
    if r.random() < 0.15:   # a sum of no length
        width = fb(r.choice([0.0, -0.0]))
        side = [fb(r.choice([0.0, -0.0])) for _ in range(2)]
    same = r.random()
    if same < 0.1:   # the same place, or all but one of it
        frm[:3] = to[:3]
    elif same < 0.2:
        frm[:3] = to[:3]
        frm[r.randrange(3)] = r.f()
    out = floats(r, 8)
    if r.random() < 0.1:   # the first point's sideways of no length
        out[:2] = out[4:6]
    for off, values in ((0x9de4 + ribbon * 0x10, last), (0x9ee4 + ribbon * 0x20, left), (0x9ef4 + ribbon * 0x20, right), (0xa0e4 + ribbon * 8, side), (0xa16c, [width]),
                        (0xa170, floats(r, 4))):
        put(obj, off, '%dI' % len(values), *values)
    strings(log, 1, 0, out=1 << 2)
    blob(log, 1, 3, 0x3f)
    fill(log, 1, 2, struct.pack('<8I', *out))
    blob(log, 2, 0, QUAD)
    set_return(log, 2, r.choice([1, 1, 2, 0, r.getrandbits(32)]))
    regions = {LOG: log, OBJECT: obj, MATRIX: struct.pack('<16I', *matrix), FROM: struct.pack('<4I', *frm), TO: struct.pack('<4I', *to), BATCH: r.bytes(0x10),
               PROJECTION: struct.pack('<I', TABLE), TABLE: vtable({}, 8, cdecl={1: 8}), 0x87b1d8: struct.pack('<I', PROJECTION), 0x87b088: struct.pack('<I', MATRIX),
               0x879ab0: struct.pack('<I', BATCH)}
    return Call(0x57dd10, [OBJECT, 0 if r.random() < 0.25 else FROM, TO], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

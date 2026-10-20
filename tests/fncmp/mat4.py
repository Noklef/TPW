# The matrix object methods (src/game/math/mat4: 0x5780a0, 0x578800, 0x578bc0, 0x577460, 0x578820, 0x578be0, 0x578ca0, 0x578d20, 0x578900, 0x578b20)
# against the originals: any matrix bits, overlapping copies included; the LU back substitution with any row indices 0-3 and right-hand sides
# with leading zeros (and NaNs); the perspective with camera-like and any floats, the render flag 0x200 either way; the far set
# beyond, at or before the kept near (a signalling NaN one too) with any render view; the transform's modes 0-4 and others (unsigned)
# with the output apart from, over the vector or over the matrix; the LU decomposition (0x578900) and the inverse (0x578b20) of
# pivotable() matrices.
# usage: mat4.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=InitGlobals,Identity,Assign,MoveAlongX,LuBackSubstitute,Perspective,SetFar,Transform,LuDecompose,Invert)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, bf, fb  # noqa: E402

B = Bench([(0x5780a0, 0x578190), (0x578800, 0x578820), (0x578bc0, 0x578bd4), (0x577460, 0x5774d0), (0x578820, 0x5788f2), (0x578900, 0x578b20), (0x578b20, 0x578bbe),
           (0x578be0, 0x578d20), (0x583220, 0x583280), (0x578d20, 0x579004)])
CALLS = {'InitGlobals': 0x5780a0, 'Identity': 0x578800, 'Assign': 0x578bc0, 'MoveAlongX': 0x577460, 'LuBackSubstitute': 0x578820,
         'Perspective': 0x578be0, 'SetFar': 0x578ca0, 'Transform': 0x578d20, 'LuDecompose': 0x578900, 'Invert': 0x578b20}


def matrix(r):
    return r.floats(bytearray(0x40), range(0, 0x40, 4), r.choice([1, 10, None]))


def pivotable(r):
    """A matrix for 0x578900: any bits (a quarter of them 0, a NaN, a signalling one or infinity), rows of zeros and copied rows now and
    then; a row with a non-NaN first entry and some nonzero non-NaN one is made if none is (else the first pivot is the stack's)."""
    m = bytearray(matrix(r))
    for i in range(r.choice([0, 0, 1, 3, 6])):
        struct.pack_into('<I', m, 4 * r.randrange(16), r.choice([0, 0x80000000, 0x7fc00000, 0x7f800001, 0x7f800000, 0xff800000]))
    if r.random() < 0.1:
        m[0x10 * r.randrange(4):][:0x10] = bytes(0x10)
    if r.random() < 0.1:
        a, b = r.sample(range(4), 2)
        m[0x10 * a:0x10 * a + 0x10] = m[0x10 * b:0x10 * b + 0x10]
    rows = [struct.unpack_from('<4f', m, 0x10 * i) for i in range(4)]
    if not any(row[0] == row[0] and any(v == v and v != 0 for v in row) for row in rows):
        struct.pack_into('<f', m, 0, 1.0)
    return bytes(m)


def case(r):
    op = r.deal(OPS)
    regions = {RAM: r.bytes(0x100)}
    if op == 'InitGlobals':
        return Call(0x5780a0, [], {0x87af00: r.bytes(0x300)})
    if op == 'Identity':
        return Call(0x578800, [RAM + r.choice([0, 0x40])], regions, conv='t')
    if op == 'Transform':  # the output apart, over the vector or over the matrix
        data = matrix(r) + r.floats(bytearray(0x10), range(0, 0x10, 4), r.choice([1, 100, None])) + r.bytes(0xb0)
        out = RAM + r.choice([0x60, 0x60, 0x60, 0x40, 0x44, 0x3c, 0x0, 0x10, 0x20])
        mode = r.choice([0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 0xffffffff, r.getrandbits(32)])
        return Call(0x578d20, [RAM, out, RAM + 0x40, mode], {RAM: data}, conv='t')
    if op == 'MoveAlongX':
        return Call(0x577460, [RAM, r.f()], {RAM: matrix(r)}, conv='t')
    if op == 'LuBackSubstitute':
        index = [r.randrange(4) for _ in range(4)] if r.random() < 0.5 else r.sample(range(4), 4)
        b = r.floats(bytearray(0x10), range(0, 0x10, 4), r.choice([1, 100, None]))
        for i in range(r.choice([0, 0, 1, 2, 3, 4])):
            struct.pack_into('<I', b, 4 * i, r.choice([0, 0, 0x80000000, 0x7fc00000, 0x7f800001]))
        return Call(0x578820, [RAM, RAM + 0x40, RAM + 0x60], {RAM: matrix(r), RAM + 0x40: struct.pack('<4i', *index), RAM + 0x60: b})
    if op == 'LuDecompose':
        return Call(0x578900, [RAM, RAM + 0x40, RAM + 0x50], {RAM: pivotable(r), RAM + 0x40: r.bytes(0x20)})
    if op == 'Invert':
        return Call(0x578b20, [RAM], {RAM: pivotable(r)}, conv='t')
    if op == 'Perspective':
        if r.random() < 0.6:
            args = [fb(r.uniform(0.01, 50)), fb(r.uniform(50, 20000)), fb(r.uniform(1, 179))]
        else:
            # (fsin/fcos of half angles from ~1e6 to 2^63 radians are left out: the x87 reduces them by a 66-bit pi, and neither the
            # emulator's nor the port's CRT stands in for that; see matrix.py)
            fov = r.f()
            if 1e7 < abs(bf(fov)) < 2e21:
                fov = r.choice([fb(r.uniform(-1e7, 1e7)), fb(r.uniform(2e21, 3e38)) | (fov & 0x80000000)])
            args = [r.f(), r.f(), fov]
            if r.random() < 0.2:
                args[1] = args[0]
        return Call(0x578be0, [RAM] + args, {RAM: r.bytes(0x40), 0x8bcbc8: struct.pack('<I', r.getrandbits(32)), 0x87b1dc: r.bytes(4),
                                             0x768acc: r.bytes(4), 0x87af58: r.bytes(4)}, conv='t')
    if op == 'SetFar':  # the kept near and field of view, a far (beyond, at or before the near), any render view
        near, fov = fb(r.uniform(0.01, 50)), fb(r.uniform(1, 179))
        if r.random() < 0.2:
            near, fov = r.choice([(r.f(), fov), (near, fb(r.uniform(-1e6, 1e6))), (r.choice([0x7f800001, 0xffc00000, 0]), fov)])
        far = r.choice([fb(r.uniform(50, 20000)), fb(r.uniform(50, 20000)), near, fb(bf(near) * 0.5), r.f()])
        view = bytearray(r.floats(bytearray(0x78), range(0, 0x78, 4), r.choice([1, 300, None])))
        struct.pack_into('<I', view, 0, r.getrandbits(32))
        if r.random() < 0.7:
            struct.pack_into('<f', view, 0x44, r.uniform(0.5, 2))
        return Call(0x578ca0, [RAM, far], {RAM: r.bytes(0x40), 0x8bcbc8: view, 0x87b1dc: struct.pack('<I', near), 0x768acc: r.bytes(4),
                                           0x87af58: struct.pack('<I', fov)}, conv='t')
    # copied upwards a dword at a time, so a source just below the matrix smears
    m = RAM + r.choice([0, 0x40, 0x80])
    return Call(0x578bc0, [m, RAM + r.choice([0, 0x40, 0x80, 0x44, 0x7c, 0x84])], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

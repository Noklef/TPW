# The matrices made from the two inputs (src/game/math/mat4: Matrices_Update 0x5783b0) against the original: any pivotable matrices, the guards of the two statics set or not, the second input's flag either way, the transformer a recorder that logs the matrix it is given, the camera's (x, y) of all kinds: ordinary, one of them 0 (so that the unit vector's cosine is 1 or past it by rounding), tiny, huge, infinite and NaN; the two pairs likewise.
# usage: matricesupdate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, log_region, vtable  # noqa: E402
from mat4 import pivotable  # noqa: E402

ATEXIT = 1
TRANSFORM = 6   # (the slot of the transformer's vtable)
ERRNO = RAM + 0xf00   # (the C library's errno, the thread's data being none here)
B = Bench([(0x5783b0, 0x5787a9), (0x5787b0, 0x578820), (0x578820, 0x5788f2), (0x578900, 0x578b20), (0x578b20, 0x578bbe), (0x67a400, 0x6902c0)], hooks={0x67a7d0: _recorder(ATEXIT, 1, cdecl=True), 0x686a00: lambda e: e.r.__setitem__('EAX', ERRNO)})
SELF, TABLE, CAMERA = RAM, RAM + 0x40, RAM + 0x400


def part(r):
    """One side of a (x, y) pair."""
    return r.choice([0.0, 0.0, -0.0, 1.0, -1.0, 3.0, -3.0, 0.1, r.uniform(-1, 1), r.uniform(-1, 1), r.uniform(-1000, 1000), r.uniform(-1000, 1000), 1e-30, -1e-30, 1e-45, -1e-45, 1e30, 3e38, -3e38, float('inf'), -float('inf'), float('nan')])


def case(r):
    log = log_region()
    blob(log, TRANSFORM, 1, 0xffff)
    world = bytearray(r.bytes(0x280))   # 0x87af60 on
    world[0x87aff0 - 0x87af60:0x87b030 - 0x87af60] = pivotable(r)
    world[0x87b030 - 0x87af60:0x87b070 - 0x87af60] = pivotable(r)
    struct.pack_into('<f', world, 0x87af70 - 0x87af60, part(r))
    struct.pack_into('<f', world, 0x87af90 - 0x87af60, part(r))
    camera = 0x87aff0 if r.random() < 0.2 else CAMERA
    struct.pack_into('<I', world, 0x87afe8 - 0x87af60, camera)
    struct.pack_into('<I', world, 0x87b1d8 - 0x87af60, SELF)
    if r.random() < 0.8:
        struct.pack_into('<f', world, 0x87afb0 - 0x87af60, part(r))
        struct.pack_into('<f', world, 0x87afd0 - 0x87af60, part(r))
    view = bytearray(r.bytes(0x40))
    struct.pack_into('<f', view, 4, part(r))
    struct.pack_into('<f', view, 0x14, part(r))
    regions = {LOG: log, SELF: struct.pack('<I', TABLE) + bytes(0x3c), TABLE: vtable({}, cdecl={TRANSFORM: 4}), CAMERA: bytes(view), ERRNO: bytes(4), 0x87af60: bytes(world),
               0xfb33e0: r.bytes(1), 0xfb3400: r.bytes(0x40), 0xfb3440: r.bytes(1), 0xfb3460: r.bytes(0x40), 0x8bd508: struct.pack('<I', r.choice([0, 0x20000000, 0xdfffffff, 0xffffffff]))}
    return Call(0x5783b0, [], regions, conv='c', ret='a', ignore=[(ERRNO, 4)])   # (the C library's errno past 1 or of a NaN, which the port does not keep)


if __name__ == '__main__':
    B.main(case)

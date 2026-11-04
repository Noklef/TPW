# A set of points drawn as triangles (src/game/render/pointsprites: 0x58ac90) against the original: the texture's format, the batch's begin, add (its 3 vertices hashed but for the specular dwords, stack garbage) and close recorders, the transformer's slot 6 (cdecl) a recorder; Mat4_Transform the original's, on a view or the static, so the matrix used shows in the points.
# usage: pointsprites.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, log_region, put, set_return, vtable  # noqa: E402

ATEXIT, FORMAT, BEGIN, ADD, CLOSE = 1, 2, 3, 4, 5
B = Bench([(0x58ac90, 0x58afa5), (0x578d20, 0x579004)],
          hooks={0x67a7d0: _recorder(ATEXIT, 1, cdecl=True), 0x590c40: _recorder(FORMAT, 0), 0x565240: _recorder(BEGIN, 5), 0x565bb0: _recorder(ADD, 5), 0x5658c0: _recorder(CLOSE, 1)})
SELF, TEXTURE, POINTS, VIEW, MATRIX, BATCH, TRANSFORMER, VTABLE, TABLE = (RAM + 0x100 * k for k in range(9))
MASK = 0xffffff & ~(1 << 5 | 1 << 13 | 1 << 21)


def matrix(r):
    buf = bytearray(0x40)
    r.floats(buf, range(0, 0x40, 4), r.choice([1.0, 1.0, 10.0]))
    return buf


def case(r):
    log = log_region()
    count = r.choice([-1, 0, 1, 2, 3, 6])
    texture = bytearray(r.bytes(0x30))
    put(texture, 8, 'I', r.getrandbits(32) & ~0x1000 | r.choice([0, 0x1000]))
    r.floats(texture, [0x20, 0x24, 0x28, 0x2c], r.choice([1.0, 16.0]))
    self_ = bytearray(r.bytes(0x20))
    put(self_, 0, 'i', count)
    put(self_, 8, 'I', TEXTURE)
    put(self_, 0xc, 'I', POINTS)
    r.floats(self_, [0x10], r.choice([1.0, 8.0, 100.0]))
    put(self_, 0x18, 'I', r.choice([0, MATRIX]))
    if r.randrange(20) == 0:
        put(self_, 0x10, 'I', r.choice([0x7fc00001, 0x7f800001, 0x7f800000]))
    points = bytearray(0xc * 6)
    r.floats(points, range(0, len(points), 4), r.choice([1.0, 100.0]))
    guard = r.choice([0, 1, 1, 0xfe, 0xff])
    regions = {LOG: log, SELF: self_, TEXTURE: texture, POINTS: points, VIEW: matrix(r), MATRIX: matrix(r), BATCH: r.bytes(4), TRANSFORMER: struct.pack('<I', VTABLE), VTABLE: vtable({}, cdecl={6: 4}),
               TABLE: struct.pack('<40I', *[r.choice([1, 1, 1, 1, 1, 1, 1, 0]) for _ in range(40)]), 0x87b088: struct.pack('<I', VIEW), 0x87b1d8: struct.pack('<I', TRANSFORMER), 0x879ab0: r.bytes(4),
               0x8bcbcc: put(bytearray(4), 0, 'f', r.uniform(0.25, 2.0)), 0xfb33e0: bytes([guard]), 0xfb3400: bytes(matrix(r))}
    set_return(log, FORMAT, r.getrandbits(32))
    set_return(log, BEGIN, r.choice([BATCH, BATCH, BATCH, 0]))
    answers(log, ADD, TABLE)
    set_return(log, CLOSE, r.choice([0, 1, 7]))
    blob(log, ADD, 0, MASK)
    return Call(0x58ac90, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

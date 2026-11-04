# A height map lit into a texture (src/game/gfx/bumplight: 0x5eb8a0) against the original: the alloc a recorder answering a block of the case (with room before it: rows shifted below the first wrap to negative), the shrink and the free recorders, _CIpow recorder 12 (its two doubles from the x87 stack) answering a float of the case; Vec3_Clamp and __ftol the original's.
# usage: bumplight.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, log_region, mask_this, put, set_float_return, set_return  # noqa: E402

ALLOC, SHRINK, FREE, POWK = 10, 11, 13, 12
POW = _recorder(POWK, 4, cdecl=True)


def _pow(e):  # _CIpow: st1 to the power st0, popping both; the recorder sees them as the env's (x, y)
    y, x = e.st.pop(0), e.st.pop(0)
    sp = e.r['ESP']
    e.r['ESP'] = sp - 16
    e.write(sp - 16, struct.pack('<2d', x, y))
    POW(e)
    e.r['ESP'] = sp


B = Bench([(0x5eb8a0, 0x5ebe30), (0x5ebe30, 0x5ebe50), (0x5ebe50, 0x5ebedf), (0x67a830, 0x67a860)],
          hooks={0x67acc0: _recorder(ALLOC, 1, cdecl=True), 0x5e80d0: _recorder(SHRINK, 1), 0x55f1e0: _recorder(FREE, 0), 0x67c5d0: _pow})
SELF, HEIGHTS, MASK, DATA, OUT, BEFORE = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x800, RAM + 0x1000
BLOCK = BEFORE + 0x200


def case(r):
    log = log_region()
    width, rows = r.choice([0, 1, 2, 3, 5, 8]), r.choice([0, 1, 2, 3, 5])
    texWidth, texRows = r.choice([1, 2, 3, 4]), r.choice([0, 1, 2, 3, 4, 4, 4])
    self_ = bytearray(r.bytes(0x70))
    r.floats(self_, [0xc, 0x10, 0x14], r.choice([0.5, 1.0, 1.0, 3.0]))
    r.floats(self_, [0x4, 0x18], r.choice([1.0, 8.0, 100.0]))
    r.floats(self_, [0x1c, 0x20], r.choice([90.0, 180.0, 400.0]))
    if r.randrange(20) == 0:
        put(self_, r.choice([4, 0xc, 0x10, 0x14, 0x18, 0x1c, 0x20]), 'I', r.choice([0x7fc00001, 0x7f800001, 0x7f800000, 0xff800000]))
    put(self_, 0x24, 'I', texRows)
    put(self_, 0x28, 'i', r.choice([0, 0, 1, 2, -1, -3, 7]))
    put(self_, 0x38, 'I', MASK)
    put(self_, 0x3c, 'I', texWidth)
    put(self_, 0x68, 'I', OUT)
    heights = put(put(bytearray(r.bytes(0x10)), 0, '2i', width, rows), 0xc, 'I', DATA)
    size = width * rows
    regions = {LOG: log, SELF: self_, HEIGHTS: heights, MASK: bytes(r.choice([0, 1, 1, 0xff, r.getrandbits(8)]) for _ in range(max(size, 1))),
               DATA: r.bytes(size + width + 2), OUT: r.bytes(max(size * 4, 4)), BEFORE: r.bytes(0x400)}
    set_return(log, ALLOC, BLOCK)
    blob(log, SHRINK, 0, 0xf)
    mask_this(log, FREE)
    set_float_return(log, POWK, r.choice([0.0, 0.25, 1.0, r.uniform(0, 100)]))
    return Call(0x5eb8a0, [SELF, HEIGHTS], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

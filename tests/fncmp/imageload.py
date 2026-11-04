# An image read from a file (src/game/gfx/imageload: Image_LoadFile 0x5d3890) against the original. Recorders (driver.cpp's kImageLoadEnv): ImageCodec_InfoFromFile 16 (fills the surface), SurfaceInfo_Alloc 17,
# ImageCodec_LoadFromFile 18, SurfaceInfo_Free 19; SurfaceInfo_Ctor is run. Each read answers through a status of the case, negative now and then; a surface paletted or not, of a small size; the old pixels there or not; the allocation failing now and then.
# usage: imageload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fill, heap, log_region, mask_this, put, set_return, strings  # noqa: E402

B = Bench([(0x5d3890, 0x5d3ac0), (0x5fa680, 0x5fa6a0), (0x67acb0, 0x67acb1), (0x67acc0, 0x67acc1)], hooks={0x5fa8a0: _recorder(16, 4), 0x5fa6f0: _recorder(17, 1), 0x5faa00: _recorder(18, 5), 0x5fa740: _recorder(19, 0)})
IMAGE, FOLDER, NAME, STATUS1, STATUS2, BITS = RAM, RAM + 0x40, RAM + 0x80, RAM + 0xc0, RAM + 0xc4, RAM + 0x100


def case(r):
    w, h = r.randrange(0, 7), r.randrange(0, 7)
    log = log_region()
    for k in (16, 17, 18, 19):
        mask_this(log, k)
    strings(log, 16, 2, 1 | 8)
    strings(log, 17, 0, 1)
    strings(log, 18, 4, 1 | 2)
    set_return(log, 16, STATUS1)
    set_return(log, 18, STATUS2)
    surface = bytearray(r.bytes(0x28))
    put(surface, 0x14, 'I', r.choice([1, 1, 0, 2, r.getrandbits(32)]))
    put(surface, 0x18, 'III', BITS, w, h)
    fill(log, 16, 3, bytes(surface))
    regions = {LOG: log, HEAP: heap(fail=r.choice([0, 0, 0, 0, 0, 1])), IMAGE: put(bytearray(r.bytes(0x10)), 0xc, 'I', 0), FOLDER: bytes(r.randrange(97, 123) for _ in range(r.randrange(0, 30))) + b'\0',
               NAME: bytes(r.randrange(97, 123) for _ in range(r.randrange(0, 30))) + b'\0', STATUS1: r.choice([0, 1, 0, 0, -1, -0x80000000]).to_bytes(4, 'little', signed=True),
               STATUS2: r.choice([0, 1, 0, 0, -1, -0x80000000]).to_bytes(4, 'little', signed=True), BITS: r.bytes(w * h * 4 + 4)}
    return Call(0x5d3890, [IMAGE, FOLDER, NAME], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)

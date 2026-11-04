# The lighting of a height image (src/game/gfx/lightshade: LightShade_Apply 0x5eb8a0) against the original, with Image_ShrinkToImage, Vec3_Clamp and the small callees run. _CIpow 0x67c5d0 is recorder 10 (the driver's env pow; here
# its two doubles from the x87 stack), answering a float of the case. Small height images, a mask with holes, a colour image of random bytes, a row offset of either sign, the tile's rows 0 now and then.
# usage: lightshade.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, set_float_return  # noqa: E402

POW = _recorder(10, 4, cdecl=True)


def _pow(e):  # _CIpow: st1 to the power st0, popping both; the recorder sees them as the env's (x, y)
    y, x = e.st.pop(0), e.st.pop(0)
    sp = e.r['ESP']
    e.r['ESP'] = sp - 16
    e.write(sp - 16, struct.pack('<2d', x, y))
    POW(e)
    e.r['ESP'] = sp


B = Bench([(0x5eb8a0, 0x5ebe30), (0x5ebe30, 0x5ebef0), (0x5e80d0, 0x5e8420), (0x55f1e0, 0x55f210), (0x67a830, 0x67a860), (0x67acb0, 0x67acb1), (0x67acc0, 0x67acc1)], hooks={0x67c5d0: _pow})
SELF, HEIGHTS, HDATA, MASK, OUT, COLOURS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x600, RAM + 0xa00


def case(r):
    w, h = r.randrange(1, 9), r.randrange(1, 6)
    tw, th = r.randrange(1, 7), r.randrange(1, 7)
    rows = r.choice([0, 1, 1, th, th, r.randrange(1, 7)])
    log = log_region()
    set_float_return(log, 10, r.choice([1.0, 0.25, r.uniform(0, 1), r.uniform(0, 100)]))
    this = bytearray(r.bytes(0x80))
    put(this, 4, 'I', r.f(300))
    put(this, 0xc, 'III', r.f(2), r.f(2), r.f(2))
    put(this, 0x18, 'I', r.f(30))
    put(this, 0x1c, 'II', r.f(360), r.f(360))
    put(this, 0x24, 'Ii', rows, r.choice([0, 0, 1, -1, r.randrange(-20, 20)]))
    put(this, 0x38, 'I', MASK)
    put(this, 0x3c, 'IIII', tw, th, 4, COLOURS)
    put(this, 0x68, 'I', OUT)
    heights = put(bytearray(r.bytes(0x10)), 0, 'III', w, h, r.getrandbits(32))
    put(heights, 0xc, 'I', HDATA)
    data = bytearray(r.bytes(w * h + w + 2))
    for k in range(len(data)):  # often level ground
        if r.random() < 0.3:
            data[k] = 0x80
    mask = bytes(r.choice([0, 0, r.randrange(1, 256), 0xff]) for _ in range(w * h))
    regions = {LOG: log, HEAP: heap(fail=r.choice([0] * 30 + [2])), SELF: this, HEIGHTS: heights, HDATA: data, MASK: mask, OUT: r.bytes(w * h * 4), COLOURS: r.bytes(tw * th * 4 + 4)}
    return Call(0x5eb8a0, [SELF, HEIGHTS], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

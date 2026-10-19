# The image grow and shrink filters of src/game/core/image_filters (0x5ea510, 0x5ea6d0) against the originals: images of 0 to 7 x 0 to 6 pixels of 1 to 3 bytes (the filters pack the
# bytes of a row, sic), radii 0 to 4, data of random bytes with many extremes.
# The erosion ramp (0x5eb270): see ramp().
# usage: image_filters.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Dilate,Erode)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

CALLS = {'Dilate': 0x5ea510, 'Erode': 0x5ea6d0, 'Mark': 0x5ea1f0, 'Ramp': 0x5eb270}
B = Bench([(0x5ea510, 0x5ea6e0), (0x5ea6d0, 0x5ea8b0), (0x5ea1f0, 0x5ea400), (0x67acb0, 0x67acb1), (0x5eb270, 0x5eb3d7), (0x67a830, 0x67a860), (0x67acc0, 0x67acc1)])
DATA = RAM + 0x1000


def ramp(r):
    """0x5eb270: a 0-6 x 0-5 image a of bytes (many 0) and b of its size, count 0-4 or 7 (the mark ramp's steps), b's data now and then a's."""
    w, h = r.randrange(0, 7), r.randrange(0, 6)
    a, b = bytearray(r.bytes(0x10)), bytearray(r.bytes(0x10))
    put(a, 0, 'IIII', w, h, 1, DATA)
    put(b, 0, 'IIII', w, h, 1, DATA + 0x200 if r.random() < 0.9 else DATA)
    data = bytearray(r.bytes(w * h + 0x20))
    for i in range(len(data)):
        if r.random() < 0.5:
            data[i] = 0
    regions = {RAM: a, RAM + 0x40: b, RAM + 0x80: r.bytes(0x20), DATA: data, DATA + 0x200: r.bytes(w * h + 0x20), HEAP: heap()}
    return Call(0x5eb270, [RAM + 0x80, RAM, r.choice([0, 1, 2, 3, 4, 7]), RAM + 0x40], regions, conv='t', ret='v')


def case(r):
    name = r.deal(OPS)
    if name == 'Ramp':
        return ramp(r)
    w, h, bpp = r.randrange(0, 8), r.randrange(0, 7), r.choice([1, 1, 2, 3])
    if name == 'Mark':
        w, h = r.randrange(0, 7), r.randrange(0, 6)
        img = bytearray(r.bytes(0x10))
        put(img, 0, 'IIII', w, h, 2, DATA + 0x100)
        mask = bytearray(r.bytes(0x10))
        put(mask, 0, 'IIII', w, h, 1, DATA + 0x400)
        other = bytearray(r.bytes(0x10))
        data = bytearray(r.bytes(2 * w * h + 0x40))
        for i in range(len(data)):
            if r.random() < 0.4:
                data[i] = r.choice([0, 1, 2, 0x80, 0xff])
        pad = bytearray(r.bytes(2 * 12 + 0x10))  # (the rows above the first and below the last are read when the guards allow)
        return Call(CALLS[name], [RAM, RAM + 0x40, r.choice([0, 1, 0x7f, 0xff]), RAM + 0x80], {RAM: img, RAM + 0x40: other, RAM + 0x80: mask, DATA: pad, DATA + 0x100: data, DATA + 0x400: r.bytes(w * h + 0x20)}, ret='v')
    img = bytearray(r.bytes(0x10))
    put(img, 0, 'IIII', w, h, bpp, DATA)
    data = bytearray(r.bytes(w * h * bpp + 0x40))
    for i in range(len(data)):
        if r.random() < 0.3:
            data[i] = r.choice([0, 0xff, 0x80, 1])
    return Call(CALLS[name], [RAM, r.choice([0, 1, 2, 3, 4, 10])], {RAM: img, DATA: data, HEAP: heap()}, ret='b')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

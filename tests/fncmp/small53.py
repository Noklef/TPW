# The image grow and shrink filters of src/game/core/small53 (0x5ea510, 0x5ea6d0) against the originals: images of 0 to 7 x 0 to 6 pixels of 1 to 3 bytes (the filters pack the
# bytes of a row, sic), radii 0 to 4, data of random bytes with many extremes.
# usage: small53.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Dilate,Erode)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

CALLS = {'Dilate': 0x5ea510, 'Erode': 0x5ea6d0}
B = Bench([(0x5ea510, 0x5ea6e0), (0x5ea6d0, 0x5ea8b0), (0x67acb0, 0x67acb1)])
DATA = RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    w, h, bpp = r.randrange(0, 8), r.randrange(0, 7), r.choice([1, 1, 2, 3])
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

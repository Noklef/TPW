# An image's box-average shrink (src/game/gfx/imageops: 0x5d4260) against the original: sources of 1 to 40 x 1 to 30 pixels of random bytes (many extremes),
# new sizes from 1 to the source's (now and then larger: a count of 0 divides by 0 in both), formats 1 and 2 (and others, which write nothing), and the
# heap failing one of the three scratch blocks now and then.
# usage: imageshrink.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x5d4260, 0x5d45b0), (0x67acb0, 0x67acb1)])
PIXELS, DEST = RAM + 0x1000, RAM + 0x8000


def size(r, n):
    return r.choice([1, n, n, max(1, n // 2), max(1, n // 3), r.randrange(1, n + 1), r.randrange(1, n + 1)] + ([n + 1] if r.random() < 0.05 else []))


def case(r):
    w, h = r.choice([1, 2, 3, 4, 8, r.randrange(1, 41)]), r.choice([1, 2, 3, 5, r.randrange(1, 31)])
    img = put(bytearray(r.bytes(0x10)), 0, 'II', w, h)
    put(img, 0xc, 'I', PIXELS)
    data = bytearray(r.bytes(w * h * 4 + 0x40))
    for i in range(len(data)):
        if r.random() < 0.3:
            data[i] = r.choice([0, 0xff, 0x80, 0xf0, 0x0f, 1])
    nw, nh = size(r, w), size(r, h)
    fmt = r.choice([1, 1, 2, 2, 0, 3])
    fail = r.choice([0] * 12 + [1, 2, 3])
    return Call(0x5d4260, [RAM, DEST, nw, nh, fmt], {RAM: img, PIXELS: data, DEST: r.bytes(nw * nh * 4 + 0x20), HEAP: heap(fail=fail)}, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)

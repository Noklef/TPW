# The Gaussian blur of one-byte images (Image_GaussianBlur, src/game/gfx/imageops: 0x5eb3e0) against the original: sizes from 0 x 0 to 24 x 14, sigma above 0 as its one caller tests (small: the kernel's edges vanish;
# up to 5: 15 x 15), data with runs and extremes, the mask's width at, or past, the image's, its bytes mostly on. __ftol and the bench's heap real.
# usage: imageblur.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x5eb3e0, 0x5eb813), (0x67a830, 0x67a860), (0x67acb0, 0x67acb1), (0x67acc0, 0x67acc1)])
THIS, IMAGE, MASK, DATA, ON = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x1000, RAM + 0x3000


def case(r):
    w, h = r.choice([0, 1, 2, 3, 5, 8, 12, 24]), r.choice([0, 1, 2, 3, 4, 7, 14])
    sigma = r.choice([r.uniform(0.01, 0.4), r.uniform(0.3, 2), r.uniform(0.3, 2), r.uniform(2, 5), 0.4, 0.8])
    stride = w + r.choice([0, 0, 0, 1, 3])
    image, mask = bytearray(r.bytes(0x10)), bytearray(r.bytes(0x10))
    put(image, 0, 'I', w)
    put(image, 4, 'I', h)
    put(image, 0xc, 'I', DATA)
    put(mask, 0, 'I', stride)
    put(mask, 0xc, 'I', ON)
    data = bytearray(r.bytes(w * h + 0x20))
    style = r.choice(['noise', 'noise', 'flat', 'edge'])
    for i in range(w * h):
        if style == 'flat':
            data[i] = data[0]
        elif style == 'edge':
            data[i] = 0xff if i % max(w, 1) < w // 2 else 0
        elif r.random() < 0.3:
            data[i] = r.choice([0, 0xff, 0x80, 1])
    on = bytearray(r.bytes(stride * h + 0x20))
    if r.random() < 0.7:
        for i in range(len(on)):
            if r.random() < 0.9:
                on[i] = on[i] | 1
    for i in range(len(on)):
        if r.random() < 0.1:
            on[i] = 0
    regions = {THIS: r.bytes(0x10), IMAGE: image, MASK: mask, DATA: data, ON: on, HEAP: heap()}
    return Call(0x5eb3e0, [THIS, IMAGE, MASK, struct.unpack('<I', struct.pack('<f', sigma))[0]], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

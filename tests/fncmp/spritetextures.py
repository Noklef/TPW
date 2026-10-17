# The sprite images' texture release (src/game/gfx/spriteframe: 0x588ba0, 0x588c20, 0x588ca0, and the frames' size setters 0x588d90,
# 0x588e40 that use it) against the originals: images with no array, or one of 0-4 textures (as texturemem.py: flags, cache items,
# tracked blocks), the u16 count at +0x16 the array's, fewer, 0 or more than there are (reading on into the array's tail), pixels at +4 or
# none; frames with or without an image, their sizes the ones given (as s16, or only by the low words) or not.
# usage: spritetextures.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Free,FreeIfSet,Release,SetSize,SetSize30)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

CALLS = {'Free': 0x588ba0, 'FreeIfSet': 0x588c20, 'Release': 0x588ca0, 'SetSize': 0x588d90, 'SetSize30': 0x588e40}
B = Bench([(0x588ba0, 0x588d6e), (0x588d90, 0x588ee8), (0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e),
           (0x67afa0, 0x67b060), (0x67acb0, 0x67acb1)])
FRAME, IMAGE, ARRAY = RAM + 0x4000, RAM + 0x4100, RAM + 0x5000   # (the textures' own regions below RAM + 0x4000, texturemem.py)
SIZES = [0, 1, 5, 0x40, 0x7fff, 0x8000, 0xffff, 0x10005, 0xffff8000, 0xffffffff]


def image(r, regions):
    img = bytearray(r.bytes(0x18))
    n = r.choice([0, 1, 1, 2, 3, 4])
    if r.random() < 0.85:
        tex = texturemem.textures(r, regions, ARRAY + 4, n + 1)   # (one more after them: the count may run past the array)
        regions[ARRAY] = struct.pack('<I', n) + tex
        put(img, 0, 'I', ARRAY + 4)
    else:
        put(img, 0, 'I', 0)
    put(img, 0x16, 'H', r.choice([n, n, n, max(n - 1, 0), n + 1, 0]))
    put(img, 4, 'I', r.choice([0, HEAP + 0x1000, HEAP + 0x1100, r.getrandbits(32)]))
    regions[IMAGE] = img


def case(r):
    name = r.choice(OPS)
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    image(r, regions)
    if name in ('Free', 'FreeIfSet', 'Release'):
        return Call(CALLS[name], [IMAGE], regions, conv='t', ret='a')
    frame = bytearray(r.bytes(0x48))
    put(frame, 0x28, 'I', IMAGE if r.random() < 0.85 else 0)
    w, h = r.choice(SIZES), r.choice(SIZES)
    at = 0x34 if name == 'SetSize' else 0x30
    if r.random() < 0.5:   # the sizes already set (both, or one)
        put(frame, at, 'HH', r.choice([w, r.getrandbits(16)]) & 0xffff, r.choice([h, h, w, r.getrandbits(16)]) & 0xffff)
    regions[FRAME] = frame
    return Call(CALLS[name], [FRAME, w, h], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

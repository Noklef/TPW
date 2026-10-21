# The sprite images' texture release (src/game/gfx/spriteframe: 0x588ba0, 0x588c20, 0x588ca0, the image destructor 0x588350, the frame
# set's release 0x588230, and the frames' size setters 0x588d90, 0x588e40 that use it) against the originals: images with no array, or one
# of 0-4 textures (as texturemem.py: flags, cache items, tracked blocks), the u16 count at +0x16 the array's, fewer, 0 or more than there
# are (reading on into the array's tail), pixels at +4 or none; frames with or without an image, their sizes the ones given (as s16, or
# only by the low words) or not; frame sets with or without a texture array, 0-3 images (the set's count theirs, fewer, one more, 0 or
# negative) and a tracked block or none. Also the renderer part's thunks to 0x588c20 for its image at +0x9c (0x48f3c0, 0x48f410).
# An image's texture set (0x588410: one given or a new one, the heap failing now and then) and its tiled textures (0x5889b0: sizes
# 0-0x60 or any, tiles of 8-0x40 (or 0x18), the heap failing now and then).
# A WCT or run-length image read and set as the texture (0x588510, 0x588660; see load()).
# usage: spritetextures.py [CASES [SEED]] [--wild|--both] [--show N]
#        (env OPS=Free,FreeIfSet,Release,Dtor,FramesFree,SetSize,SetSize30,PartFree,PartFree2,SetTexture,Make,LoadWct,LoadRle)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

CALLS = {'Free': 0x588ba0, 'FreeIfSet': 0x588c20, 'Release': 0x588ca0, 'Dtor': 0x588350, 'FramesFree': 0x588230, 'SetSize': 0x588d90,
         'SetSize30': 0x588e40, 'PartFree': 0x48f3c0, 'PartFree2': 0x48f410, 'SetTexture': 0x588410, 'Make': 0x5889b0,
         'LoadWct': 0x588510, 'LoadRle': 0x588660}
B = Bench([(0x48f3c0, 0x48f3ce), (0x48f410, 0x48f41c), (0x588ba0, 0x588d6e),(0x588d90, 0x588ee8), (0x588230, 0x5882d4), (0x588350, 0x588410), (0x590a70, 0x590a75),
           (0x590b90, 0x590c39), (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1),
           (0x588410, 0x58850d), (0x5889b0, 0x588b9c), (0x590a00, 0x590a70), (0x590d40, 0x590f7e), (0x67d830, 0x67d8f0), (0x56af20, 0x56af9c),
           (0x588510, 0x58865e), (0x588660, 0x588813), (0x564ba0, 0x564bf0), (0x564d00, 0x564e12), (0x564ff0, 0x565070), (0x5c4f60, 0x5c4f7c),
           (0x67a830, 0x67a860), (0x5646c0, 0x564708), (0x564730, 0x564741), (0x564750, 0x564790), (0x564a80, 0x564af0), (0x41a840, 0x41a84a)])
STREAM, SVT = RAM + 0x7000, RAM + 0x7100   # the loaders' stream: slot 2 (the reads) a recorder
FRAME, IMAGE, ARRAY, IMAGES = RAM + 0x4000, RAM + 0x4100, RAM + 0x5000, RAM + 0x6000   # (the textures' own regions below RAM + 0x4000)
SIZES = [0, 1, 5, 0x40, 0x7fff, 0x8000, 0xffff, 0x10005, 0xffff8000, 0xffffffff]
BLOCKS = [0, HEAP + 0x1000, HEAP + 0x1100]


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
    put(img, 4, 'I', r.choice(BLOCKS + [r.getrandbits(32)]))
    regions[IMAGE] = img


def frames(r, regions):
    """a frame set at FRAME: its textures at ARRAY, its images at IMAGES (+0 only set where the set's count clears it)."""
    s = bytearray(r.bytes(0x10))
    n = r.choice([0, 1, 2, 3])
    regions[ARRAY] = struct.pack('<I', n) + texturemem.textures(r, regions, ARRAY + 4, n)
    put(s, 8, 'I', ARRAY + 4 if r.random() < 0.85 else 0)
    m = r.choice([0, 1, 2, 3])
    count = r.choice([m, m, m, max(m - 1, 0), m + 1, 0, 0xffffffff])
    imgs = bytearray(r.bytes(0x18 * (m + 1)))
    for k in range(m + 1):
        put(imgs, 0x18 * k, 'I', r.getrandbits(32) if k < count < 0x80000000 and r.random() < 0.7 else 0)
        put(imgs, 0x18 * k + 4, 'I', r.choice(BLOCKS + [0, r.getrandbits(32)]))
    regions[IMAGES] = struct.pack('<I', m) + imgs
    put(s, 0, 'I', count)
    put(s, 4, 'I', IMAGES + 4 if r.random() < 0.85 else 0)
    put(s, 0xc, 'I', r.choice(BLOCKS + [r.getrandbits(32)]))
    regions[FRAME] = s


def tiles(r, name):
    """0x588410 / 0x5889b0: the image at IMAGE (its row width, sizes and pixels any or usual), the cache's tile size, the format,
    the error list; a texture given (as texturemem.py's) or the heap failing now and then."""
    regions = {HEAP: heap(0x20000, fail=r.choice([0] * 12 + [1])), **global_heap(), LOG: log_region()}
    given = texturemem.textures(r, regions, ARRAY + 4, 1)
    regions[ARRAY] = struct.pack('<I', 1) + given
    cache = bytearray(regions[texturemem.CACHE])
    put(cache, 0x18f0, 'i', r.choice([8, 0x10, 0x10, 0x20, 0x40, 0x18]))
    regions[texturemem.CACHE] = bytes(cache)
    regions[0x8bd554] = struct.pack('<I', r.randrange(4))
    regions[0x879ad0] = put(bytearray(r.bytes(0x204)), 0x200, 'i', r.choice([r.randrange(0x40), 0, 0x3f, 0x40]))
    regions[0x8798c8] = bytes(4)
    img = bytearray(r.bytes(0x18))
    size = [0, 1, 7, 8, 0x10, 0x11, 0x20, 0x40, 0x41, 0x60]
    put(img, 0xc, 'H', r.choice(size + [r.getrandbits(16)]))   # (the row width; +0xe unused)
    put(img, 0x10, 'HH', r.choice(size), r.choice(size))
    regions[IMAGE] = img
    if name == 'Make':
        xy = [r.choice([0, 0, 1, 0x10, r.getrandbits(8), r.getrandbits(32)]) for _ in range(2)]
        return Call(CALLS[name], [IMAGE] + xy, regions, conv='t', ret='a')
    blocks = [0, 0, HEAP + 0x1000, r.getrandbits(32)]
    args = [r.choice(blocks), r.choice(blocks), r.choice(blocks), r.choice(SIZES), r.choice(SIZES),
            ARRAY + 4 if r.random() < 0.4 else 0, r.choice([0, 0, 0x8000, r.getrandbits(32)])]
    return Call(CALLS[name], [IMAGE] + args, regions, conv='t', ret='a')


def load(r, name):
    """0x588510 (thiscall (stream, texture)) / 0x588660 (thiscall (stream, language, texture)): tiles()'s world and texture (given
    or not, the heap failing now and then: the first allocation is the WCT / RLE image's), a stream whose reads (slot 2) log their
    destination as 0 (stack addresses) and now and then write four bytes through it; the WCT header at 0x879a38 (its floats and block
    sizes as wctimage.py's read()); the tracked blocks as wctimage.py's."""
    from fncmp import fill, strings, vtable
    from wctimage import tracked
    c = tiles(r, 'SetTexture')
    regions = c.regions
    put(regions[HEAP], 0, 'I', HEAP + 0x10 + r.choice([0] * 10 + [1, 2, 3]))   # (tiles()'s heap, its failing allocation chosen again)
    tracked(r, regions)
    log = regions[LOG]
    strings(log, 2, 0, out=1)
    # (the RLE size is read through the stream, so its reads always write: a size left as the block's address would be a GlobalAlloc
    # of gigabytes; small sizes then)
    if name == 'LoadRle' or r.random() < 0.6:
        small = r.choice([0, 1, 4, 0x10, 0x40, 0x100])
        fill(log, 2, 0, struct.pack('<I', small) if name == 'LoadRle' or r.random() < 0.5 else r.bytes(4))
    regions[STREAM], regions[SVT] = struct.pack('<I', SVT), vtable({2: 2})
    header = bytearray(r.bytes(0x2c))
    for at in range(0x14, 0x24, 4):
        put(header, at, 'f', r.choice([r.uniform(0, 100), float(r.randrange(0, 40)), 0.5, 1.0, -3.0, 1e20]))
    put(header, 0x24, '2I', *(r.choice([0, 1, 0x10, 0x100, r.randrange(0x400)]) for _ in range(2)))
    put(header, 4, '2H', *(r.choice([0, 1, 0x10, 0x40, r.getrandbits(16)]) for _ in range(2)))
    regions[0x879a38] = header
    texture = c.args[6]
    args = [STREAM, texture] if name == 'LoadWct' else [STREAM, r.getrandbits(32), texture]
    return Call(CALLS[name], [IMAGE] + args, regions, conv='t', ret='a')


def case(r):
    name = r.deal(OPS)
    if name in ('SetTexture', 'Make'):
        return tiles(r, name)
    if name in ('LoadWct', 'LoadRle'):
        return load(r, name)
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    if name == 'FramesFree':
        frames(r, regions)
        return Call(CALLS[name], [FRAME], regions, conv='t', ret='a')
    image(r, regions)
    if name in ('Free', 'FreeIfSet', 'Release', 'Dtor'):
        return Call(CALLS[name], [IMAGE], regions, conv='t', ret='v' if name == 'Dtor' else 'a')
    if name == 'PartFree':   # (the renderer part 0x6ff2d8 with its image at +0x9c; one unused argument)
        return Call(CALLS[name], [IMAGE - 0x9c, r.getrandbits(32)], regions, conv='t', ret='a')
    if name == 'PartFree2':
        return Call(CALLS[name], [IMAGE - 0x9c], regions, conv='t', ret='a')
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

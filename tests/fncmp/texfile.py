# A texture's image file read into pixels (src/game/gfx/texfile: 0x574bd0) against the original: the file through vfs.py's mounts and
# memory source, its bytes a TGA header (sizes 1-8 mostly, bits 8 / 16 / 24 / 32, the descriptor's bit 5 or not) and pixels, or a WCT image
# (wctimage.py's Load: a valid kind or not) followed by such a TGA for the second open; flags 0-3; the format table given (0x1000 random
# bytes) or the current one (0x8bd550's, index 0-3); the pixel size given or the format's; the device kind ([0x879ab0]+0x24) 1, 3 or other;
# the mip levels after the first ([0x87a264]) -1 to 2; the tracked blocks as wctimage.py's; the heap failing now and then (not a string's
# allocation, nor the image's: both fault in both).
# usage: texfile.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, Bench, Call, RAM, global_heap, heap, log_region, put  # noqa: E402
import vfs  # noqa: E402
import wctimage  # noqa: E402
from gamestring import LOCALE  # noqa: E402

RANGES = wctimage.merged(vfs.RANGES + [(0x564ba0, 0x5650b0), (0x574bd0, 0x575600), (0x5839e0, 0x583a00), (0x579b40, 0x579c30), (0x5c4d50, 0x5c5020),
                                        (0x67a400, 0x690000)])
B = Bench(RANGES)
TABLE, OUT, DEV = RAM + 0x8000, RAM + 0x7f00, RAM + 0x7f80


def tga(r, flags):
    """A TGA header and its pixel bytes (one level and some): the rows of a top-first one (descriptor bit 5) with flag 1 at least a
    dword (a row of 0 dwords would swap 2^32)."""
    h = bytearray(r.bytes(0x12))
    w, ht = r.choice([1, 2, 3, 4, 8, r.randrange(1, 9)]), r.choice([1, 2, 3, 4, r.randrange(1, 9)] * 9 + [0, -1])
    bits = r.choice([8, 16, 24, 32, 32, r.choice([0, 4, 40])])
    descriptor = r.randrange(256)
    if flags & 1 and descriptor & 0x20 and w * bits < 32:
        bits, w = 32, max(w, 1)
    put(h, 0xc, '2h', w, ht)
    h[0x10], h[0x11] = bits, descriptor
    return bytes(h) + r.bytes(max(w * ht, 0) * 4 * 3)


def exact(r):
    """A file's bytes that one read (flags without 1, the heap not failing) takes whole, so that another can follow: a valid WCT image
    and its blocks, or a WCT header of kind 0 and a TGA with one level of pixels (its height 0-8)."""
    if r.random() < 0.5:
        header = bytearray(r.bytes(0x1c))
        header[0], header[2] = r.choice([0x12, 0x13]), r.choice([0x20, 0x10])
        sizes = [r.choice([0, 1, 4, 0x10, r.randrange(0x40)]) for _ in range(2)]
        put(header, 0x10, '2I', *sizes)
        return bytes(header) + r.bytes(sizes[0] + (sizes[1] if header[2] == 0x20 else 0))
    h = bytearray(tga(r, 0)[:0x12])
    w, ht = struct.unpack_from('<2h', h, 0xc)
    ht = max(ht, 0)
    put(h, 0xe, 'h', ht)
    return bytes(0x1c) + bytes(h) + r.bytes(h[0x10] * w * ht >> 3)


def scene(r, flags, first=0):
    """The regions for a file read with these flags: the heap (failing now and then; first: the allocations before the load's), the
    mounts serving the file's bytes, the name at vfs.NAME, 0x87a24c's globals, the device and the current format. Returns them."""
    fail = r.choice([0] * 6 + [1, 3] if flags & 1 else [0] * 8 + [2, 4, 6, 7])   # (no string's: those fault in both)
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x10000, fail=fail + first if fail else 0), LOCALE: bytes(4), **global_heap()}
    wctimage.tracked(r, regions)
    root = vfs.mounts(r, log, regions)
    if flags & 1 or fail == 2:   # (2: the WCT image's stream not got, nothing read for it)
        data = tga(r, flags)
    elif r.random() < 0.5:   # (the WCT image not valid: kind 0)
        data = bytes(0x1c) + tga(r, flags)
    else:
        header = bytearray(r.bytes(0x1c))
        header[0] = r.choice([0x12, 0x13, 0x13, 0x11])
        header[2] = r.choice([0x20, 0x10])
        sizes = [r.choice([0, 1, 4, 0x10, r.randrange(0x40)]) for _ in range(2)]
        put(header, 0x10, '2I', *sizes)
        read = sizes[0] + (sizes[1] if header[2] == 0x20 else 0) if header[0] in (0x12, 0x13) else 0
        data = bytes(header) + r.bytes(read) + tga(r, flags)   # (the TGA after what is read)
    vfs.memory_source(r, log, regions, data)
    name = vfs.name_under(r, root) + r.choice([b'.tga', b'.TGA', b'.t', b'.tgax', b'.', b'..x\\a.bmp'])
    if r.random() < 0.15:
        name = r.choice([b'..', b'.', b'...']) + name
    regions[vfs.NAME] = name + bytes(1) + r.bytes(7)
    regions[0x87a24c] = put(bytearray(r.bytes(0x20)), 0x18, 'i', r.choice([0, 0, 1, 2, -1]))
    regions[0x879ab0] = struct.pack('<I', DEV)
    regions[DEV] = put(bytearray(r.bytes(0x28)), 0x24, 'I', r.choice([1, 3, 3, 1, 2, 0]))
    index = r.randrange(4)
    regions[0x8bd550] = put(bytearray(r.bytes(0x88 + 0x1000 * 4)), 4, 'I', index)
    return regions


def case(r):
    flags = r.choice([0, 1, 2, 3, 0, 2, r.getrandbits(32)])
    regions = scene(r, flags)
    regions[TABLE] = r.bytes(0x1000)
    regions[OUT] = r.bytes(8)
    args = [vfs.NAME, OUT, OUT + 4, flags, r.choice([TABLE, 0]), r.choice([0, 0, 2, 4, 3])]
    return Call(0x574bd0, args, regions, ret='a')


if __name__ == '__main__':
    B.main(case)

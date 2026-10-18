# A run-length image's scaled draw (src/game/gfx/rleimage: 0x564790) against the original: images of 1 to 40 x 1 to 20 pixels whose rows are runs and
# literal spans of random palette indices (zero-length runs too; each row's length byte right), palettes of random u16 / u32 entries by the format
# (0x8bd554 indexing pixel sizes 2, 4 and 3 at 0x701158), and destinations the same size or narrower, wider, shorter and taller, with a pitch that
# fits the row, exceeds it or is 0. The codes' read (0x564750: the tracked blocks' table as in trackedglobal.py, GlobalAlloc failing now and then,
# a recorder stream) and the deleting destructor (0x564a90 on the part at +0x18: virtual base offsets 0x18 and others, codes tracked, untracked or
# none, flags with and without bit 0); the constructor (0x5646c0: most derived or not, virtual base offsets 0x18 and others).
# usage: rleimage.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Draw,Read,Dtor,Ctor)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, global_heap, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x564790, 0x564a45), (0x5da3c0, 0x5da3c1), (0x564750, 0x564790), (0x564a90, 0x564af0), (0x579b40, 0x579c13), (0x5c4f60, 0x5c4f7c), (0x5646c0, 0x564708)])
OBJ, CODES, PALETTE, DEST, OUT = RAM, RAM + 0x100, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000


def row(r, width):
    out = bytearray()
    x = 0
    while x < width:
        n = min(r.choice([1, 1, 2, 3, 5, 8]), width - x) if r.random() < 0.97 else r.choice([1, 2, 4])  # (sometimes past the width)
        if r.random() < 0.5:
            out += struct.pack('<b', n) + bytes(r.randrange(256) for _ in range(n))
        else:
            if r.random() < 0.05:
                out += bytes([0, r.randrange(256)])  # a run of none
            out += struct.pack('<b', -n) + bytes([r.randrange(256)])
        x += n
    return bytes([len(out)]) + bytes(out) if len(out) < 0x80 else row(r, width)  # (a length byte over 0x7f skips back, sic: not made)


def draw(r):
    w, h = r.choice([1, 2, 4, 8, r.randrange(1, 41)]), r.choice([1, 2, 3, 6, r.randrange(1, 21)])
    codes = b''.join(row(r, w) for _ in range(h)) + r.bytes(0x40)
    mode = r.randrange(4)
    size = [2, 4, 3, 2][mode]
    dw = r.choice([w, w, max(1, w // 2), w * 2, r.randrange(1, 2 * w + 1)])
    dh = r.choice([h, h, max(1, h // 2), h * 2, r.randrange(1, 2 * h + 1)])
    pitch = r.choice([size * dw, size * dw, size * dw + r.choice([2, 4, 8]), 0])
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x20 - 0x14, '4I', CODES, PALETTE, w, h)
    dest = put(bytearray(r.bytes(0x20)), 0, '4I', OUT, dw, dh, pitch)
    regions = {OBJ: obj, CODES: codes, PALETTE: r.bytes(0x400), DEST: dest, OUT: r.bytes((h + 2) * max(pitch, size * dw) + 0x40),
               0x701158: struct.pack('<4I', 2, 4, 3, 2), 0x8bd554: struct.pack('<I', mode)}
    return Call(0x564790, [OBJ + 0x20, DEST], regions, conv='t', ret='a')


COUNT, TABLE = 0x8bb214, 0x87b1f8   # the tracked blocks: their count and (block, block) pairs
VBT, STREAM, SVT = RAM + 0x5000, RAM + 0x5100, RAM + 0x5200


def tracked(r, regions, blocks):
    n = r.choice([0, 1, 2, 3, 5])
    table = bytearray(8 * 8)
    for k in range(8):
        b = r.choice(blocks + [r.getrandbits(32) | 0x80000000])
        put(table, 8 * k, '2I', b, r.choice([b, b, b + 4]) & 0xffffffff)
    regions[TABLE] = table
    regions[COUNT] = struct.pack('<I', r.choice([n, n, n, 0x8000]))


def read(r):
    regions = {HEAP: heap(fail=r.choice([0] * 6 + [1])), **global_heap()}
    tracked(r, regions, [HEAP + 0x10])
    log = log_region()
    regions.update({OBJ: bytearray(r.bytes(0x40)), STREAM: struct.pack('<I', SVT), SVT: vtable({2: 2}), LOG: log})
    size = r.choice([0, 1, 4, 0x10, 0x100, r.randrange(0x200)])
    return Call(0x564750, [OBJ, STREAM, size], regions, conv='t', ret='a')


def dtor(r):
    regions = {HEAP: heap(), **global_heap()}
    block = r.choice([0, HEAP + 0x20, HEAP + 0x40, HEAP + 0x44, r.getrandbits(32) | 0x80000000])
    tracked(r, regions, [HEAP + 0x20, HEAP + 0x40, HEAP + 0x3c])
    base = r.choice([0x18, 0x18, 0x18, 0x1c, 0x20, 0x24, 4, 0x10])
    obj = bytearray(r.bytes(0x40))
    put(obj, 0, 'I', VBT)
    put(obj, 4, 'I', block)
    regions[VBT] = put(bytearray(r.bytes(8)), 4, 'I', base)
    image = HEAP + 0x100 if r.random() < 0.5 else OBJ
    if image == OBJ:
        regions[OBJ] = obj
    else:   # (inside the heap region: a freed image shows in its hash)
        regions[HEAP][0x100:0x140] = obj
    flags = r.choice([0, 1, 2, 3, 0xffffffff, r.getrandbits(32)])
    return Call(0x564a90, [image + 0x18, flags], regions, conv='t', ret='a')


def ctor(r):
    obj = bytearray(r.bytes(0x400))
    put(obj, 0, 'I', RAM + 0x300)  # the vtable it starts with (when not most derived): +4 the virtual base offset
    put(obj, 0x304, 'I', r.choice([0x18, 0x18, 0x1c, 0x20]))
    return Call(0x5646c0, [RAM, r.choice([0, 1, 1])], {RAM: obj}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Draw,Read,Dtor,Ctor').split(',') if k]


def case(r):
    return {'Draw': draw, 'Read': read, 'Dtor': dtor, 'Ctor': ctor}[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)

# The pointer array's sprite images (src/game/core/ptrlist: 0x56b450 the removal of one, 0x56b210 the release of all) against the originals:
# 0-4 images (an empty texture array or none, a tracked pixel block or none) in a tracked array at +0xc (count +0x18), the one to remove
# among them, not or (now and then) a stray; the release with bit 0 of +0 set or not, the renderer at 0x87a6d0 or none (the part in its tag
# 0x60 list or not, as holder.py), 0-3 frames at +8 (count +0x1c) and the virtual base object at +4 (its deleting destructor a recorder) or
# none (the tracked blocks' table as in trackedglobal.py).
# usage: ptrarray_images.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Remove,Release)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, global_heap, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x56b210, 0x56b2e4), (0x56b450, 0x56b4b3), (0x588ca0, 0x588d6e), (0x588350, 0x588410), (0x590a70, 0x590a75),
           (0x590b90, 0x590c39), (0x67afa0, 0x67b0d0), (0x579b80, 0x579c13), (0x576540, 0x5765d0)])
COUNT, TABLE = 0x8bb214, 0x87b1f8   # the tracked blocks: their count and (block, block) pairs
OBJ, IMAGES, ARRAY, EMPTY, BASE, VBT, VT, RENDERER, NODES, FRAMES = (RAM + 0x100 * k for k in range(10))
BLOCKS = [HEAP + 0x20, HEAP + 0x40, HEAP + 0x3c]


def block(r):
    return r.choice([0] + BLOCKS + [r.getrandbits(32) | 0x80000000])


def common(r, regions):
    table = bytearray(8 * 8)
    for k in range(8):
        b = r.choice(BLOCKS + [ARRAY, r.getrandbits(32) | 0x80000000])
        put(table, 8 * k, '2I', b, r.choice([b, b, b + 4]) & 0xffffffff)
    regions[TABLE] = table
    regions[COUNT] = struct.pack('<I', r.choice([0, 1, 2, 3, 5]))
    regions[EMPTY] = struct.pack('<2I', 0, 0)
    images = bytearray(r.bytes(0x18 * 4))
    for k in range(4):
        put(images, 0x18 * k, 'I', r.choice([0, EMPTY + 4]))
        put(images, 0x18 * k + 4, 'I', block(r))
        put(images, 0x18 * k + 0x16, 'H', 0)
    regions[IMAGES] = images
    n = r.choice([0, 1, 2, 3, 4])
    regions[ARRAY] = struct.pack('<4I', *[IMAGES + 0x18 * k for k in range(4)])
    obj = bytearray(r.bytes(0x20))
    put(obj, 0xc, 'I', ARRAY if r.random() < 0.9 else 0)
    put(obj, 0x18, 'I', r.choice([n, n, n, 0, max(n - 1, 0)]))
    regions[OBJ] = obj
    return obj, n


def remove(r):
    regions = {HEAP: heap(), **global_heap()}
    obj, n = common(r, regions)
    put(obj, 0xc, 'I', ARRAY)
    image = IMAGES + 0x18 * r.randrange(4) if r.random() < 0.85 else r.getrandbits(32)
    return Call(0x56b450, [OBJ, image], regions, conv='t', ret='a')


def release(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    obj, n = common(r, regions)
    put(obj, 0, 'I', r.getrandbits(32))
    offset = r.choice([8, 0x10, 0x18])
    put(obj, 4, 'I', BASE if r.random() < 0.8 else 0)
    regions[BASE] = put(put(bytearray(r.bytes(0x20)), 0, 'I', VBT), offset, 'I', VT)
    regions[VBT] = put(bytearray(r.bytes(8)), 4, 'I', offset)
    regions[VT] = vtable({0: 1})
    m = r.choice([0, 1, 2, 3])
    put(obj, 8, 'I', FRAMES)
    put(obj, 0x1c, 'I', r.choice([m, m, 0]))
    regions[FRAMES] = struct.pack('<3I', *[r.getrandbits(32) for _ in range(3)])
    renderer = bytearray(r.bytes(0x40))
    nodes = bytearray(0x10)
    put(nodes, 0, 'II', BASE + offset if r.random() < 0.6 else r.getrandbits(32), 0)
    put(renderer, 0x28, 'I', NODES if r.random() < 0.8 else 0)
    put(renderer, 4, 'I', 0)
    put(renderer, 0xc, 'I', 5)
    regions.update({RENDERER: renderer, NODES: nodes, 0x8bd500: r.bytes(0x20),
                    0x87a6d0: put(bytearray(r.bytes(0x10)), 0, 'I', RENDERER if r.random() < 0.8 else 0)})
    return Call(0x56b210, [OBJ], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Remove,Release').split(',') if k]


def case(r):
    return {'Remove': remove, 'Release': release}[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)

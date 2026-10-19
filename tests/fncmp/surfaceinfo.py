# SurfaceInfo as a bitmap of its own (src/game/gfx/surface: 0x5fa680-0x5fa897, thiscall) against the originals: infos of random masks, a bit
# count of the game's depths or any, sizes small or wild, pitches positive, negative or none; Alloc and Free through the game's GlobalAlloc /
# GlobalFree import slots (fncmp's global heap; now and then the allocation fails); sub-rectangles in and around the bitmap, empty or in either
# order.
# usage: surfaceinfo.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, global_heap, heap, put  # noqa: E402

B = Bench([(0x5f5660, 0x5f56a0), (0x5fa680, 0x5fa8b0), (0x61af60, 0x61b000)])
INFO, DST, OUT, ARG, ARG2 = RAM, RAM + 0x40, RAM + 0x80, RAM + 0xc0, RAM + 0x100
CALLS = {'Ctor': 0x5fa680, 'Init': 0x5fa6a0, 'Alloc': 0x5fa6f0, 'Free': 0x5fa740, 'SubRectAt': 0x5fa760, 'SubRect': 0x5fa7b0, 'Size': 0x5fa890}


def small(r, top):
    return r.choice([r.randrange(0, top), r.randrange(0, top), 0, 1, top, r.randrange(-20, 0), r.getrandbits(32) - (1 << 31)])


def info(r):
    i = bytearray(r.bytes(0x28))
    put(i, 0x10, 'I', r.choice([8, 16, 24, 32, 15, 1, 0, r.getrandbits(32)]))
    put(i, 0x18, 'I', r.choice([0, 0, HEAP + 0x40, RAM + 0x1000, r.getrandbits(32)]))
    put(i, 0x1c, 'II', small(r, 200) & 0xffffffff, small(r, 200) & 0xffffffff)
    put(i, 0x24, 'i', r.choice([0, 64, 640, 800, -640, r.getrandbits(32) - (1 << 31)]))
    return i


def case(r):
    op = r.deal(OPS)
    regions = {INFO: info(r), DST: r.bytes(0x28), OUT: r.bytes(4)}
    at = CALLS[op]
    if op in ('Ctor', 'Size', 'Free'):
        if op == 'Free':
            regions.update({HEAP: heap(), **global_heap()})
        return Call(at, [INFO], regions, conv='t', ret='v' if op == 'Free' else 'a')
    if op == 'Init':
        regions[ARG] = struct.pack('<2I', small(r, 200) & 0xffffffff, small(r, 200) & 0xffffffff)
        regions[ARG2] = info(r)[:0x18]
        return Call(at, [INFO, ARG, ARG2, r.getrandbits(32), r.choice([0, 0, 64, 640, -640 & 0xffffffff, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op == 'Alloc':
        regions.update({HEAP: heap(fail=r.choice([0, 0, 0, 1])), **global_heap()})
        return Call(at, [INFO, OUT], regions, conv='t', ret='a')
    if op == 'SubRectAt':
        regions[ARG] = struct.pack('<2i', small(r, 200), small(r, 200))
        regions[ARG2] = struct.pack('<2i', small(r, 200), small(r, 200))
        return Call(at, [INFO, OUT, DST, ARG, ARG2], regions, conv='t', ret='a')
    xs, ys = [small(r, 200), small(r, 200)], [small(r, 200), small(r, 200)]
    if r.random() < 0.7:
        xs.sort()
        ys.sort()
    regions[ARG] = struct.pack('<4i', xs[0], ys[0], xs[1], ys[1])
    return Call(at, [INFO, OUT, DST, ARG], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

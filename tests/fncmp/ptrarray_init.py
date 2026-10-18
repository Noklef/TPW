# The pointer array's set-up (src/game/core/ptrlist: 0x56b0c0 PtrArray_Init) against the original: an array object as the static one at
# 0x879ce0 starts (its fields clear) with a capacity already now and then (the error pushed onto the engine's list at 0x879ad0, as in
# ptrarray_add.py); any frame and image counts; fncmp's heap failing the set, its tracked array or the image block now and then (the failures
# release everything: PtrArray_ReleaseAll with the renderer at 0x87a6d0 or none, the set deleted through its virtual base); the live objects
# list and the tracked blocks' table as rendererpart.py's.
# usage: ptrarray_init.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, global_heap, heap, log_region, put, strings  # noqa: E402

B = Bench([(0x491f10, 0x491f90), (0x56a770, 0x56afb0), (0x56b0c0, 0x56b300), (0x576540, 0x5765d0), (0x5775d0, 0x577660), (0x579b40, 0x579c30),
           (0x588350, 0x588420), (0x588ca0, 0x588d80), (0x58a4f0, 0x58a560), (0x58dc50, 0x58dd70), (0x590b90, 0x590c50), (0x67a400, 0x690000)],
          hooks={0x564650: _cdecl_log})
OBJ, RENDERER, NODES = RAM, RAM + 0x100, RAM + 0x200
LIVE, LIVE_COUNT, COUNT, PAIRS = 0x87a6d8, 0x87aed8, 0x8bb214, 0x87b1f8


def case(r):
    log = log_region()
    strings(log, 0x3f, 1)
    obj = bytearray(0x20)
    put(obj, 0, 'I', r.getrandbits(32))
    put(obj, 0x10, 'I', r.choice([0] * 6 + [1, r.getrandbits(32)]))
    frames = r.choice([0, 1, 2, 5, 0x10, 0x40])
    images = r.choice([0, 0, 1, 3, 0x20])
    errors = bytearray(r.bytes(0x204))
    struct.pack_into('<i', errors, 0x200, r.choice([r.randrange(0x41), 0, 0x3f, 0x40]))
    flags = r.getrandbits(32) & ~0x1000000 | (0x1000000 if r.random() < 0.5 else 0)
    count = r.choice([0, 1, 2, 3, 0xf])
    pairs = bytearray(r.bytes(0x80))
    live_count = r.choice([0, 1, 2, 5, 0x1ff, 0x200, r.randrange(0x200)])
    renderer = bytearray(r.bytes(0x40))
    put(renderer, 0x28, 'I', NODES if r.random() < 0.8 else 0)
    put(renderer, 4, 'I', 0)
    put(renderer, 0xc, 'I', 5)
    regions = {OBJ: obj, HEAP: heap(0x800, r.choice([0, 0, 0, 1, 2, 3])), **global_heap(), 0x879ad0: errors, 0x8bd508: struct.pack('<I', flags), LOG: log,
               COUNT: struct.pack('<I', count), PAIRS: pairs, LIVE: r.bytes(0x800), LIVE_COUNT: struct.pack('<I', live_count),
               RENDERER: renderer, NODES: put(bytearray(0x10), 0, 'I', r.choice([HEAP + 0x30, r.getrandbits(32)])), 0x8bd500: r.bytes(0x20),
               0x87a6d0: put(bytearray(r.bytes(0x10)), 0, 'I', RENDERER if r.random() < 0.5 else 0)}
    return Call(0x56b0c0, [OBJ, frames, images], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

# The landscape's levels (src/game/terrain/land: 0x55f040, 0x55f470, 0x55f530, 0x55f5a0) and Array3D_Read (src/game/gfx/array3d, 0x5e78b0) against the
# originals: a record stream of array headers (sizes 0-3, depths 1-4, now and then a wild one) and their bytes, sometimes cut short in the last array's
# bytes; old blocks or none for the heap to take back; a failing allocation now and then; objects of random bytes for the constructors and the destructor.
# usage: land_levels.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Read,Level,TileCtor,Ctor,Dtor)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put, record_stream  # noqa: E402

CALLS = {'Read': 0x5e78b0, 'Level': 0x55f470, 'TileCtor': 0x55f040, 'Ctor': 0x55f530, 'Dtor': 0x55f5a0}
B = Bench([(0x55f040, 0x55f1f0), (0x55f1e0, 0x55f730), (0x5c4f60, 0x5c4f90), (0x5d3860, 0x5d38a0), (0x5e78b0, 0x5e79a0), (0x67acf0, 0x67ada0),
           (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1), (0x67acc0, 0x67acc1)])
OBJ, STREAM, DATA = RAM, RAM + 0x400, RAM + 0x1000
TILES = 0x8798c8


def array(r, last):
    """One array's stream bytes: its header and its pixel bytes. Only the last array of a read gets a wild header: a wild size reads what is left of
    the stream, and the headers after it would read nothing (the original then uses uninitialised stack)."""
    if last and r.wild and r.random() < 0.1:
        w, h, d = r.choice([0xffffffff, 0x10000, 0x80000001]), r.randrange(1, 3), r.randrange(1, 3)
        return struct.pack('<3I', w, h, d), 0
    w, h, d = r.randrange(0, 4), r.randrange(0, 4), r.randrange(1, 5)
    if last and r.random() < 0.2:  # a read of 256 bytes or more (its count shows in the upper bytes of the return)
        w, h, d = r.randrange(8, 17), r.randrange(4, 9), 4
    return struct.pack('<3I', w, h, d) + r.bytes(w * h * d), w * h * d


def old_blocks(r, obj, offsets):
    for off in offsets:
        put(obj, off + 0xc, 'I', r.choice([0, 0, HEAP + 0x2000 + off]))


def case(r):
    op = r.choice(OPS)
    if op in ('Read', 'Level'):
        obj = bytearray(r.bytes(0xe0))
        data, last = (b'', 0) if op == 'Read' else (r.bytes(5), 0)
        offsets = [0] if op == 'Read' else list(range(8, 0xd8, 0x10))
        old_blocks(r, obj, offsets)
        for k in range(len(offsets)):
            chunk, last = array(r, k == len(offsets) - 1)
            data += chunk
        left = len(data) + r.choice([0, 0, 4])
        if last and r.random() < 0.2:
            left = len(data) - r.randrange(1, last + 1)
        data += r.bytes(4)
        fail = r.randrange(1, len(offsets) + 1) if r.random() < 0.05 else 0
        regions = {OBJ: obj, STREAM: record_stream(STREAM, DATA, left), DATA: data, HEAP: heap(0x600, fail)}
        return Call(CALLS[op], [OBJ, STREAM], regions, conv='t', ret='a')
    obj = bytearray(r.bytes(0x300))
    if op == 'Dtor':
        offsets = [0x14] + [0x24 + 0xd8 * k + off for k in range(3) for off in range(8, 0xd8, 0x10)]
        old_blocks(r, obj, offsets)
        regions = {OBJ: obj, TILES: struct.pack('<I', r.choice([0, HEAP + 0x1f00])), HEAP: heap(0x40)}
        return Call(CALLS[op], [OBJ], regions, conv='t', ret='v')
    return Call(CALLS[op], [OBJ], {OBJ: obj, HEAP: heap(0x40)}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

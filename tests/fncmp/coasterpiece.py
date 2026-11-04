# A piece of a coaster type loaded (src/game/coaster/setup: 0x430130) against the original: the ride resource's load a recorder answering a slot of the table 0x7ae6a8 (0, an empty slot or a model), the model's parts with flag 0x80 listed
# in a GlobalMem block, its bounds and the biggest face's widened by 4 (flag 1 of the piece; a NaN now and then); faces 0-3.
# usage: coasterpiece.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_region, put, set_return  # noqa: E402

LOAD = 12
B = Bench([(0x430130, 0x4305a0), (0x44b270, 0x44b2f0), (0x45b9d0, 0x45ba30), (0x5da3c0, 0x5da3c1)], hooks={0x4629d0: _recorder(LOAD, 11, cdecl=True)})
PIECE, HOLDER, INNER, MODEL, EXTRA = RAM, RAM + 0x100, RAM + 0x140, RAM + 0x180, RAM + 0x300
FACES, PARTS = RAM + 0x1000, RAM + 0x2000
TABLE = 0x7ae6a8
BOX = range(0, 0x18, 4)


def box(r, buf, at):
    r.floats(buf, [at + k for k in BOX], r.choice([1.0, 50.0, 1000.0, 1e20]))
    if r.randrange(16) == 0:
        put(buf, at + 4 * r.randrange(6), 'I', r.choice([0x7fc00001, 0xffc00002, 0x7f800001, 0x7f800000, 0xff800000]))


def case(r):
    log = log_region()
    faces = r.choice([0, 1, 2, 3])
    parts = r.choice([0, 1, 2, 5])
    model = bytearray(r.bytes(0xa0))
    put(model, 0x44, 'H', faces)
    put(model, 0x48, 'H', parts)
    put(model, 0x70, 'I', FACES)
    put(model, 0x78, 'I', EXTRA)
    put(model, 0x7c, 'I', PARTS)
    box(r, model, 0x80)
    array = bytearray(r.bytes(0xa0 * 4))
    for i in range(4):
        put(array, i * 0xa0 + 0x58, 'H', r.choice([0, 0, 1, 5, 5, r.getrandbits(16)]))
        box(r, array, i * 0xa0 + 0x78)
    table = bytearray(0x20)
    put(table, 4, 'I', HOLDER)
    put(table, 8, 'I', 0)
    set_return(log, LOAD, r.choice([1, 1, 1, 1, 1, 1, 1, 1, 0, 2]))
    part_bytes = bytearray(r.bytes(0x14 * 5))
    for i in range(5):
        put(part_bytes, i * 0x14, 'I', r.getrandbits(32) & ~0x80 | r.choice([0, 0x80]))
    extra = bytearray(r.bytes(0x60))
    r.floats(extra, [0x48], 50.0)
    piece = bytearray(r.bytes(0x40))
    r.floats(piece, [0x10, 0x14], 100.0)
    regions = {LOG: log, PIECE: piece, HOLDER: put(bytearray(r.bytes(0x20)), 8, 'I', INNER), INNER: put(bytearray(r.bytes(0x10)), 4, 'I', MODEL), MODEL: model, EXTRA: extra, FACES: array, PARTS: part_bytes,
               TABLE: table, HEAP: heap(0x1000), **global_heap()}
    return Call(0x430130, [PIECE, r.getrandbits(32) | 0x80000000], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

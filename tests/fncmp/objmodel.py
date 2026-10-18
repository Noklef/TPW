# An engine object's model unload (src/game/model/objmodel: 0x57a100) against the original: no model now and then; else 0-3 meshes
# (flag 0x200 or not, 0-3 faces whose first pointers are null or point at words with 0x1000 or not) under the renderer flag 0x20 or
# not, 0-3 texture sets (or none) with 1, 2 or 0 users (their own counts 0: the tracked blocks only), the saved pairs off, the block
# at +0x13c none or one without a texture, the tracked blocks (+0x144, the meshes' +0x74, the sets' +8) as in spritesfree.py's
# table, the model's flag 2 with the procedural texture object (no cache; a tracked block or none) there or not.
# usage: objmodel.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, global_heap, heap, log_region, put  # noqa: E402

B = Bench([(0x57a100, 0x57a258), (0x579b80, 0x579c13), (0x57b2e0, 0x57b393), (0x57b070, 0x57b130), (0x57b5b0, 0x57b608),
           (0x575f00, 0x575f8d), (0x56e4e0, 0x56e549), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1)])
OBJ, MODEL, SETS, SET, MESHES, FACES, WORDS, BLOCK, TEX = (RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x1000, RAM + 0x2000,
                                                        RAM + 0x2400, RAM + 0x2600, RAM + 0x3000)
COUNT, TABLE = 0x8bb214, 0x87b1f8
BLOCKS = [HEAP + 0x20, HEAP + 0x40, HEAP + 0x3c]


def block(r):
    return r.choice([0] + BLOCKS + [r.getrandbits(32) | 0x80000000])


def case(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    table = bytearray(8 * 8)
    for k in range(8):
        b = r.choice(BLOCKS + [r.getrandbits(32) | 0x80000000])
        put(table, 8 * k, '2I', b, r.choice([b, b, b + 4]) & 0xffffffff)
    regions[TABLE] = table
    regions[COUNT] = struct.pack('<I', r.choice([0, 1, 2, 3, 5]))
    obj = bytearray(r.bytes(0x150))
    put(obj, 4, 'I', r.getrandbits(32) & ~0x60000000)
    put(obj, 8, 'I', MODEL if r.random() < 0.9 else 0)
    put(obj, 0x13c, 'I', r.choice([0, BLOCK]))
    put(obj, 0x144, 'I', block(r))
    model = bytearray(r.bytes(0x80))
    sets, meshes = r.choice([0, 1, 2, 3]), r.choice([0, 1, 2, 3])
    put(model, 0x30, 'B', r.getrandbits(8))
    put(model, 0x36, 'H', sets)
    put(model, 0x44, 'H', meshes)
    put(model, 0x50, 'I', SETS)
    put(model, 0x70, 'I', MESHES)
    pairs, sets_ = bytearray(r.bytes(0x20)), bytearray(r.bytes(0x40))
    for k in range(4):
        put(pairs, 8 * k + 4, 'I', SET + 0x10 * k if r.random() < 0.8 else 0)
        put(sets_, 0x10 * k + 8, 'I', block(r))
        put(sets_, 0x10 * k + 0xc, 'hh', 0, r.choice([1, 1, 2, 0]))
    faces = bytearray(r.bytes(0x10 * 12))
    words = bytearray(r.bytes(0x20))
    for k in range(12):
        put(faces, 0x10 * k, 'I', r.choice([0, WORDS + 2 * r.randrange(16)]))
    for k in range(16):
        put(words, 2 * k, 'H', r.getrandbits(16) & ~0x1000 | r.choice([0, 0, 0x1000]))
    mesh = bytearray(r.bytes(0xa0 * 4))
    for k in range(4):
        put(mesh, 0xa0 * k, 'I', r.getrandbits(32) & ~0x200 | r.choice([0, 0, 0x200]))
        put(mesh, 0xa0 * k + 0x5a, 'H', r.choice([0, 1, 2, 3]))
        put(mesh, 0xa0 * k + 0x6c, 'I', FACES + 0x30 * k)
        put(mesh, 0xa0 * k + 0x74, 'I', block(r))
    blk = bytearray(r.bytes(0x20))
    put(blk, 0x18, 'I', 0)
    tex = bytearray(r.bytes(0x234))
    put(tex, 0x230, 'I', 0)
    regions.update({OBJ: obj, MODEL: model, SETS: pairs, SET: sets_, MESHES: mesh, FACES: faces, WORDS: words, BLOCK: blk, TEX: tex,
                    TEX + 0x12970: struct.pack('<I', block(r)), 0x879d60: struct.pack('<I', r.choice([0, TEX])),
                    0x8bd550: struct.pack('<I', r.getrandbits(32))})
    return Call(0x57a100, [OBJ], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

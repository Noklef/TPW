# Freeing every loaded sprite (src/game/sprite/loading: 0x541ed0) against the original: 0-4 sprites (the count also 0 or negative now
# and then), each with or without its frame set (+0x204 set or not; a frame set with or without a texture array of no textures, no images
# and a tracked block or none), the 16 draw list objects at 0x8751f0 and the overlay one at 0x876420 with tracked blocks at +0x14 or none
# (the tracked blocks' table as in trackedglobal.py).
# usage: spritesfree.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

B = Bench([(0x541ed0, 0x541f5d), (0x588230, 0x5882d4), (0x58a520, 0x58a548), (0x590a70, 0x590a75), (0x67afa0, 0x67b0d0),
           (0x579b80, 0x579c13)])
COUNT, TABLE = 0x8bb214, 0x87b1f8   # the tracked blocks: their count and (block, block) pairs
SPRITES, FRAMES, EMPTY = RAM, RAM + 0x1000, RAM + 0x2000
BLOCKS = [HEAP + 0x20, HEAP + 0x40, HEAP + 0x3c]


def block(r):
    return r.choice([0] + BLOCKS + [r.getrandbits(32) | 0x80000000])


def case(r):
    regions = {HEAP: heap(), **global_heap(), EMPTY: struct.pack('<2I', 0, 0)}
    n = r.choice([0, 1, 2, 3, 4])
    table = bytearray(8 * 8)
    for k in range(8):
        b = r.choice(BLOCKS + [r.getrandbits(32) | 0x80000000])
        put(table, 8 * k, '2I', b, r.choice([b, b, b + 4]) & 0xffffffff)
    regions[TABLE] = table
    regions[COUNT] = struct.pack('<I', r.choice([0, 1, 2, 3, 5]))
    m = r.choice([0, 1, 2, 3, 4])
    regions[0x87626c] = struct.pack('<i', r.choice([m, m, m, 0, -1]))
    sprites = bytearray(0x210 * 4)
    frames = bytearray(r.bytes(0x10 * 4))
    for k in range(4):
        sprite = bytearray(r.bytes(0x210))
        has = r.random() < 0.7
        put(sprite, 0x204, 'B', r.choice([1, 0xff, r.getrandbits(8)]) if has else 0)
        put(sprite, 0, 'I', FRAMES + 0x10 * k if (has and r.random() < 0.95) or r.random() < 0.5 else 0)   # (a set flag with no set: the first free crashes in both)
        put(frames, 0x10 * k, '4I', r.getrandbits(32), 0, EMPTY + 4 if r.random() < 0.3 else 0, block(r))
        sprites[0x210 * k:0x210 * (k + 1)] = sprite
    regions[SPRITES] = sprites
    regions[FRAMES] = frames
    regions[0x876b48] = struct.pack('<4I', *[SPRITES + 0x210 * k for k in range(4)])
    lists = bytearray(r.bytes(0x24 * 16))
    for k in range(16):
        put(lists, 0x24 * k + 0x14, 'I', block(r))
    regions[0x8751f0] = lists
    regions[0x876420] = put(bytearray(r.bytes(0x24)), 0x14, 'I', block(r))
    return Call(0x541ed0, [], regions)


if __name__ == '__main__':
    B.main(case)

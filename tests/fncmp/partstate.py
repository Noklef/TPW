# The part states' activation (src/game/model/partstate: 0x44b410) against the original, with its callees run: an instance's part states (flags,
# entries, the active count), a thing handle (0, or one of three things in the slot table 0x7cfb90, its upper half random) whose sprite choice
# (Thing_SpriteChoice: folder 3 empty or not, the thing's kind 2 or not, else a random sprite of folder 1 by the world's seed) starts a sprite
# script (SpriteScript_Start: the list ready or not, a slot table full or not (then grown by 50), the scripts from the heap through the
# GlobalAlloc import (global_heap), the game clock (0x785970) frozen so that its time needs no timer).
# usage: partstate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

B = Bench([(0x402d70, 0x403220), (0x4033a0, 0x4033f0), (0x44b410, 0x44b4d0), (0x4758f0, 0x475b90), (0x4fcac0, 0x4fcb20), (0x50b350, 0x50b370), (0x516330, 0x516390),
           (0x540c60, 0x540da0), (0x541f60, 0x541fb0), (0x5f5660, 0x5f56a0), (0x5f5e20, 0x5f5e90), (0x5f5fa0, 0x5f5fc0), (0x5da3c0, 0x5da3c1)])
INSTANCE, PS, ENTRIES, THINGS, TABLE, SPRITES, WORLD = (RAM + o for o in (0, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x10000))
SCRIPTS = 0x7b49f0   # the list: slot table, slots, ..., ready (+0x24)
FOLDERS, GSPRITES, SLOTS, CLOCK = 0x763f88, 0x876b48, 0x7cfb90, 0x785970
NSPRITES = 16


def case(r):
    regions = {HEAP: heap()}
    regions.update(global_heap())
    # the part states
    instance = bytearray(r.bytes(0x40))
    put(instance, 0x28, 'I', PS)
    ps = bytearray(r.bytes(0x14))
    put(ps, 4, 'I', ENTRIES)
    put(ps, 8, 'I', r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]))
    regions.update({INSTANCE: instance, PS: ps, ENTRIES: r.bytes(0x14 * 4)})
    # the things (sprite flags 0-3, or with the top bit, at +0x20; the kind at +0x24) in slots 1-3
    things = bytearray(r.bytes(0x40 * 3))
    slots = bytearray(r.bytes(20 * 4))
    for k in range(3):
        put(things, 0x40 * k, 'H', k + 1)
        put(things, 0x40 * k + 0x20, 'I', r.randrange(4) | r.choice([0, 0, 0x80000000]))
        put(things, 0x40 * k + 0x24, 'I', r.choice([2, 2, 0, 1, 5]))
        put(slots, 20 * (k + 1), 'I', THINGS + 0x40 * k)
    regions.update({THINGS: things, SLOTS: slots})
    # the sprite folders (name, first sprite, count): folder 1 has 1-4 sprites, folder 3 none or 1-4; gSprites' frame counts (+0x222)
    folders = bytearray(12 * 4)
    put(folders, 12 + 4, 'II', r.randrange(8), r.choice([1, 2, 3, 4]))
    put(folders, 36 + 4, 'II', r.randrange(8), r.choice([0, 0, 1, 2, 4]))
    sprites = bytearray(r.bytes(0x240 * 4))
    regions.update({FOLDERS: folders, SPRITES: sprites, GSPRITES: b''.join(struct.pack('<I', SPRITES + 0x240 * r.randrange(4)) for _ in range(NSPRITES))})
    world = WORLD - 0x1da708
    regions.update({0x80239c: struct.pack('<I', world), 0x7cf83c: struct.pack('<I', world), WORLD: r.bytes(0x20)})
    # the script list: a slot table of 1-4 slots, full or with a free one; the clock frozen
    n = r.choice([1, 2, 3, 4])
    table = bytearray(struct.pack('<4I', *[r.choice([0, RAM + 0x3000]) for _ in range(4)]))
    if r.random() < 0.4:
        table[4:] = struct.pack('<3I', RAM + 0x3000, RAM + 0x3000, RAM + 0x3000)
    regions[TABLE] = table
    regions[SCRIPTS] = struct.pack('<11I', TABLE, n, 0, 0, 0, 0, 0, 0, 0, r.choice([1, 1, 1, 0]), 0)
    clock = bytearray(r.bytes(0x80))
    put(clock, 0x38, 'I', 1)
    regions[CLOCK] = clock
    handle = r.choice([0, 1, 2, 3, 3]) | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)
    return Call(0x44b410, [INSTANCE, r.randrange(4), handle], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

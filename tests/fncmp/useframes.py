# The sprites' frame sets swapped for the ground / park ones (src/game/sprite/loading: Sprites_UseGroundFrames 0x542420, Sprites_UseParkFrames
# 0x542640) against the originals. Their calls are recorders (the driver's SpriteLoadingEnv, thiscall): NamedList_Find 0x421770 (1 of 2:
# answering 1 mostly, the folder written through its out argument), SpriteFrames_Free 0x588230 (2), 0x41e740 (3 of 2: the stream and the
# name, a stack String logged as 0; 1 mostly), SpriteFrames_Load 0x587db0 (4 of 2) and SpriteFrames_MarkTextures 0x5882e0 (5); the Strings,
# StreamBase_Ctor and the heap run as they are (the names built land in the heap, which is compared). The fixture: the frames in use
# (0x763f80: 0, 1 or another), the byte 0x763f28, the four suffixes (wide Strings), the sprite table (0x876b48, count 0x876b28: up to 6,
# negative now and then) with names, frame sets (or none) and the bytes +0x204 to +0x207; now and then an allocation fails.
# usage: useframes.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fill, heap, log_region, set_return, strings  # noqa: E402

HOOKS = {0x421770: _recorder(1, 2), 0x588230: _recorder(2, 0), 0x41e740: _recorder(3, 2), 0x587db0: _recorder(4, 2), 0x5882e0: _recorder(5, 0)}
RANGES = [(0x542420, 0x542634), (0x542640, 0x542858), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)]
B = Bench(RANGES, hooks=HOOKS)
SPRITES, TEXT = RAM, RAM + 0x4000
SUFFIXES = [0x875fe8, 0x875598, 0x876270, 0x8757a8]


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x4000, fail=r.randrange(1, 12) if r.random() < 0.05 else 0)}
    set_return(log, 1, 1 if r.random() < 0.85 else r.choice([0, 2]))
    fill(log, 1, 1, struct.pack('<I', r.getrandbits(32)))
    strings(log, 1, 0, out=1 << 1)
    set_return(log, 3, 1 if r.random() < 0.8 else r.choice([0, 2]))
    strings(log, 3, 0, out=1 << 1)
    set_return(log, 4, r.choice([0, 1, 1, 2]))
    for k, at in enumerate(SUFFIXES):
        gamestring.string(r, 1, at, TEXT + 0x100 * k, regions)
    n = r.randrange(0, 7) if r.random() < 0.9 else r.choice([-1, -5])
    table = []
    sprites = bytearray()
    for k in range(7):
        sprite = bytearray(r.bytes(0x20c))
        struct.pack_into('<I', sprite, 0, r.getrandbits(32) | 1 if r.random() < 0.6 else 0)
        name = bytes(r.choice(b'abAZ.\x80') for _ in range(r.randrange(0, 12)))
        sprite[4:4 + len(name) + 1] = name + b'\0'
        for b in range(0x204, 0x208):
            sprite[b] = r.choice([0, 1, r.getrandbits(8)])
        table.append(SPRITES + len(sprites))
        sprites += sprite + bytes(0x214 - 0x20c)
    regions.update({SPRITES: sprites, 0x876b48: struct.pack('<7I', *table), 0x876b28: struct.pack('<i', n),
                    0x763f80: struct.pack('<I', r.choice([0, 1, 0, 1, 2, r.getrandbits(32)])), 0x763f28: bytes([r.choice([0, 1, r.getrandbits(8)])]),
                    0x786b9c: struct.pack('<I', r.getrandbits(32)), 0x876960: r.bytes(0x10), gamestring.LOCALE: bytes(4)})
    return Call(r.choice([0x542420, 0x542640]), [], regions, ret='b')


if __name__ == '__main__':
    B.main(case)

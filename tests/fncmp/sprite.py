# The sprite definitions' constructor and lookups (src/game/sprite/sprite: 0x540bd0 thiscall, 0x541f70-0x54200a and 0x5423a0 cdecl) against the
# originals: gSpriteFolders (0x763f88: name, first, count) of small firsts and counts (now and then 0: the division faults in both), gSprites
# (0x876b48) pointing at four sprites of random counts, animations and directions, the world (gWorldPtr 0x7cf83c) with a random seed for World_Rand;
# FrameEntry over folder 13's sprites with animations past 16, negative ones, directions any.
# usage: sprite.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,RandomSprite,ActionCount,RandomAnim,FrameEntry)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x540bd0, 0x540d90), (0x541f70, 0x54200b), (0x5423a0, 0x542412), (0x516330, 0x516365)])
SPRITES = [RAM + 0x300 * i for i in range(4)]
IMAGES = RAM + 0x1000
WORLD = 0x31000000
FOLDERS, TABLE, WORLD_PTR = 0x763f88, 0x876b48, 0x7cf83c
CALLS = {'Ctor': 0x540bd0, 'RandomSprite': 0x541f70, 'ActionCount': 0x541fa0, 'RandomAnim': 0x541fd0, 'FrameEntry': 0x5423a0}


def sprite(r, image):
    s = bytearray(r.bytes(0x260))
    put(s, 0, 'I', image)
    if r.random() < 0.9:
        put(s, 0x21e, 'H', r.choice([1, 2, 5, 16, r.randrange(1, 0x10000)]))
        put(s, 0x20c, 'H', r.randrange(5))
    for n in range(16):
        if r.random() < 0.7:
            s[0x222 + 4 * n] = r.choice([0, 1, 4, 8, r.randrange(256)])   # frames a direction
            s[0x223 + 4 * n] = r.choice([0, 0, 1, 8, r.randrange(256)])   # directions
    return s


def case(r):
    op = r.deal(OPS)
    if op == 'Ctor':
        return Call(0x540bd0, [SPRITES[0]], {SPRITES[0]: r.bytes(0x260)}, conv='t', ret='a')
    folders = bytearray(r.bytes(12 * 16))
    for f in range(16):
        put(folders, 12 * f + 4, '2I', r.randrange(8), r.choice([1, 2, 3, 7, 40, r.getrandbits(32)]) if r.random() < 0.97 else 0)
    regions = {FOLDERS: folders, TABLE: b''.join(struct.pack('<I', r.choice(SPRITES)) for _ in range(16 + 8)), WORLD_PTR: struct.pack('<I', WORLD),
               WORLD + 0x1da708: r.bytes(4), IMAGES: r.bytes(8)}
    for i, s in enumerate(SPRITES):
        regions[s] = sprite(r, IMAGES)
    folder = r.randrange(16)
    if op == 'RandomSprite':
        return Call(0x541f70, [folder], regions, ret='a')
    if op in ('ActionCount', 'RandomAnim'):
        return Call(CALLS[op], [folder, r.randrange(16)], regions, ret='a')
    anim = r.choice([r.randrange(16), r.randrange(16 * 16), r.randrange(-15, 0)])
    return Call(0x5423a0, [anim & 0xffffffff, r.choice([0, 1, 5, r.getrandbits(16)]), r.choice([0, 3, 7, 8, r.getrandbits(32)])], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

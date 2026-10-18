# The decoded texture sprite's constructor and shown flag (src/game/gfx/texsprite: 0x5f4370 TexSprite_Ctor, 0x5f4490) against the originals.
# Ctor: the pointer array at 0x879ce0 with 0 to 0x20 frames, the global list at [0xfa6e38] empty or with a tail node, fncmp's heap for the
# new frame and the list's node. Shown: the flag +0x2c 0, 1 or 5 and values of those or anything, the frame +0x28 random.
# Update (0x5f44d0): modes 0-4 with arguments matching +0x20 often, colour kinds 0-6 (and any), the source +0x18 null or not with
# kinds 1 / 6 often and positions about -10..110 (a few huge or NaN), the colour records at 0xf86a04-0xf86d1c random.
# usage: texsprite.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Shown,Update)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, heap, put  # noqa: E402

B = Bench([(0x5f4370, 0x5f43fa), (0x56b4c0, 0x56b56e), (0x588d70, 0x588d90), (0x56b620, 0x56b628), (0x5e42d0, 0x5e4320), (0x5f4490, 0x5f44d0), (0x5f44d0, 0x5f46b0), (0x5f09a0, 0x5f0ae0),
           (0x67a830, 0x67a860)])
OBJ, ITEMS, HEADER, LIST, NODE = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100
SOURCE = RAM + 0x4000
OPS = [k for k in os.environ.get('OPS', 'Ctor,Shown,Update').split(',') if k]


def shown(r):
    obj = bytearray(r.bytes(0x400))
    put(obj, 0x28, 'I', RAM + 0x100)
    put(obj, 0x2c, 'I', r.choice([0, 1, 5]))
    return Call(0x5f4490, [RAM, r.choice([0, 1, 5, r.getrandbits(32)])], {RAM: obj}, conv='t', ret='a')


def position(r):
    return r.choice([r.uniform(-10, 110), float(r.randrange(-2, 103)), 255.9, 256.0, 300.5, -1e12, 1e30, float('nan')])


def update(r):
    obj, source = bytearray(r.bytes(0x400)), bytearray(r.bytes(0x200))
    put(obj, 0x28, 'I', RAM + 0x100)
    put(obj, 0x2c, 'I', r.choice([0, 1, 5]))
    mode, key = r.choice([0, 1, 1, 2, 2, 3, 3, 4, r.getrandbits(32)]), r.choice([0, 1, 4, 0x80, r.getrandbits(32)])
    put(obj, 0x1c, 'II', mode, key)
    put(obj, 0x14, 'II', r.choice([0, 0, 1, 2, 3, 4, 4, 5, 6, r.getrandbits(32)]), r.choice([SOURCE, SOURCE, SOURCE, 0]))
    source[2] = r.choice([1, 6, 1, 6, 0, 2, 0xff])
    put(source, 0x19c, 'f', position(r))
    put(source, 0x1f8, 'f', position(r))
    args = [r.choice([key, key | r.getrandbits(32), ~key & 0xffffffff, r.getrandbits(32)]) for _ in range(3)]
    return Call(0x5f44d0, [RAM] + args, {RAM: obj, SOURCE: source, 0xf86a00: r.bytes(0x320)}, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'Shown':
        return shown(r)
    if op == 'Update':
        return update(r)
    arr = bytearray(r.bytes(0x20))
    put(arr, 4, 'I', HEADER)
    put(arr, 8, 'I', ITEMS)
    put(arr, 0x1c, 'I', r.choice([0, 1, 5, r.randrange(0x20)]))
    lst = bytearray(r.bytes(0x10))
    empty = r.random() < 0.4
    put(lst, 4, 'I', 0 if empty else NODE)
    put(lst, 8, 'I', 0 if empty else NODE)
    put(lst, 0xc, 'I', 0 if empty else 1)
    regions = {OBJ: r.bytes(0x40), ITEMS: r.bytes(0x100), HEADER: r.bytes(0x10), LIST: lst, NODE: r.bytes(0xc), 0x879ce0: arr,
               0xfa6e38: struct.pack('<I', LIST), HEAP: heap(0x200)}
    return Call(0x5f4370, [OBJ] + [r.getrandbits(32) for _ in range(5)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

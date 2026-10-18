# The decoded texture sprite's constructor and shown flag (src/game/gfx/texsprite: 0x5f4370 TexSprite_Ctor, 0x5f4490) against the originals.
# Ctor: the pointer array at 0x879ce0 with 0 to 0x20 frames, the global list at [0xfa6e38] empty or with a tail node, fncmp's heap for the
# new frame and the list's node. Shown: the flag +0x2c 0, 1 or 5 and values of those or anything, the frame +0x28 random.
# usage: texsprite.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Shown)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, heap, put  # noqa: E402

B = Bench([(0x5f4370, 0x5f43fa), (0x56b4c0, 0x56b56e), (0x588d70, 0x588d90), (0x56b620, 0x56b628), (0x5e42d0, 0x5e4320), (0x5f4490, 0x5f44d0)])
OBJ, ITEMS, HEADER, LIST, NODE = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100
OPS = [k for k in os.environ.get('OPS', 'Ctor,Shown').split(',') if k]


def shown(r):
    obj = bytearray(r.bytes(0x400))
    put(obj, 0x28, 'I', RAM + 0x100)
    put(obj, 0x2c, 'I', r.choice([0, 1, 5]))
    return Call(0x5f4490, [RAM, r.choice([0, 1, 5, r.getrandbits(32)])], {RAM: obj}, conv='t', ret='a')


def case(r):
    if r.deal(OPS) == 'Shown':
        return shown(r)
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

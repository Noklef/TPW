# The thing preview (src/game/ui/thingpreview: 0x486530, 0x486630) against the originals. Show: a rectangle with words small, wide or
# edges, a thing whose script +0xc is a live slot, an empty one, 0 or out of range, the depth small or wide, the scale floats (whole,
# fractions, huge past __ftol's range, infinities, NaNs), [0x74f898] (Sprite_FlipY's height) a few or random; the sprite scripts
# ready or not, their slot table (now and then full, so it grows: the heap) and the shown copy [0x7c24d8] / [0x7c24a0] set or not.
# Hide: the same globals. The heap region serves GlobalAlloc / GlobalFree.
# usage: thingpreview.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Show,Hide)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, heap, put  # noqa: E402

CALLS = {'Show': 0x486530, 'Hide': 0x486630}
B = Bench([(0x486530, 0x486601), (0x486630, 0x486652), (0x475010, 0x476a30), (0x476a70, 0x476a81), (0x67a830, 0x67a857),
           (0x5f5660, 0x5f5690)])
SCRIPTS, TABLE, RECT, THING = RAM, RAM + 0x1000, RAM + 0x1100, RAM + 0x1200
NSCRIPTS, STRIDE = 6, 0x120


def f32(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def scale(r):
    return r.choice([f32(8.0), f32(12.0), f32(float(r.randrange(-50, 50))), f32(r.uniform(-1e4, 1e4)), f32(r.uniform(-4, 4)),
                     f32(3e19), f32(-3e19), f32(1e30), 0x7f800000, 0xff800000, 0x7fc00000, 0x7f800001, 0xffc12345, f32(-0.0),
                     r.getrandbits(32)])


def word(r):
    return r.choice([r.randrange(-50, 700), r.randrange(-50, 700), 0, -1, 0x7fff, -0x8000, r.getrandbits(16)]) & 0xffff


def case(r):
    op = r.choice(OPS)
    nslots = r.choice([4, 8, 12, r.randrange(1, 13)])
    slots = [0] + [r.choice([0] + [SCRIPTS + STRIDE * k for k in range(NSCRIPTS)] * 3) for _ in range(11)]
    if r.random() < 0.15:  # full: the table grows
        slots = [0] + [SCRIPTS + STRIDE * r.randrange(NSCRIPTS) for _ in range(11)]
    g = bytearray(r.bytes(0x2c))
    put(g, 0, 'Ii', TABLE, nslots)
    put(g, 0x24, 'I', r.choice([1, 1, 1, 0]))
    live = [k for k in range(1, nslots) if k < 12 and slots[k]]
    shown = bytearray(r.bytes(0x40))  # 0x7c24a0: the copy's slot; 0x7c24d8: the thing shown
    put(shown, 0, 'i', r.choice(live + [0, -1, nslots, r.randrange(13)]) if live else r.choice([0, -1, r.randrange(13)]))
    put(shown, 0x38, 'I', r.choice([THING, THING, 0, 0, r.getrandbits(32)]))
    regions = {SCRIPTS: r.bytes(STRIDE * NSCRIPTS), TABLE: struct.pack('<12I', *slots), 0x7b49f0: g, 0x7c24a0: shown, HEAP: heap()}
    if op == 'Hide':
        return Call(CALLS[op], [], regions)
    rect = bytearray(r.bytes(0x10))
    put(rect, 4, 'HHHH', *(word(r) for _ in range(4)))
    thing = bytearray(r.bytes(0x20))
    put(thing, 0xc, 'i', r.choice(live * 3 + [0, 0]) if live else 0)
    regions.update({RECT: rect, THING: thing})
    if r.random() < 0.7:
        regions[0x74f898] = struct.pack('<I', r.choice([f32(480.0), f32(600.0), f32(768.0), 0, f32(-1.0), r.getrandbits(32)]))
    depth = r.choice([0, 1, 2, 5, 0x100, r.randrange(-300, 300), r.getrandbits(32)]) & 0xffffffff
    return Call(CALLS[op], [RECT, THING, depth, scale(r)], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

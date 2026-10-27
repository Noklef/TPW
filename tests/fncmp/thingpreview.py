# The thing preview (src/game/ui/thingpreview: 0x486530, 0x486610, 0x486630) against the originals. Show: a rectangle with words small, wide or
# edges, a thing whose script +0xc is a live slot, an empty one, 0 or out of range, the depth small or wide, the scale floats (whole,
# fractions, huge past __ftol's range, infinities, NaNs), [0x74f898] (Sprite_FlipY's height) a few or random; the sprite scripts
# ready or not, their slot table (now and then full, so it grows: the heap) and the shown copy [0x7c24d8] / [0x7c24a0] set or not.
# Hide: the same globals. Copy (0x486610): those globals with the shown thing's script slot. The heap region serves GlobalAlloc /
# GlobalFree (global_heap).
# Preview408 / Preview3b0 (0x4b6b90, 0x4b5330: ui/panel7cc408's, ui/panel7cc3b0's): Show's surroundings, the thing [0x7c2658] or 0 now and
# then, the panel's window +0x18 with its saved rectangle +0x2c's words small, wide or edges and its depth +0xe0 small or any.
# usage: thingpreview.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Show,Hide,Copy,Preview408,Preview3b0)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, global_heap, heap, put  # noqa: E402

CALLS = {'Show': 0x486530, 'Hide': 0x486630, 'Copy': 0x486610, 'Preview408': 0x4b6b90, 'Preview3b0': 0x4b5330}
B = Bench([(0x486530, 0x486601), (0x486630, 0x486652), (0x486610, 0x486630), (0x475010, 0x476a30), (0x476a70, 0x476a81), (0x67a830, 0x67a857),
           (0x5f5660, 0x5f5690), (0x4b6b90, 0x4b6c1c), (0x4b5330, 0x4b53bb), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00),
           (0x65aa20, 0x65aaa0), (0x65d4f6, 0x65d521), (0x66bff7, 0x66c00b)])
SCRIPTS, TABLE, RECT, THING, PANEL, WINDOW = RAM, RAM + 0x1000, RAM + 0x1100, RAM + 0x1200, RAM + 0x1300, RAM + 0x1400
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
    op = r.deal(OPS)
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
    regions = {SCRIPTS: r.bytes(STRIDE * NSCRIPTS), TABLE: struct.pack('<12I', *slots), 0x7b49f0: g, 0x7c24a0: shown, HEAP: heap(), **global_heap()}
    if op == 'Hide':
        return Call(CALLS[op], [], regions)
    if op == 'Copy':  # the shown thing's script (+0xc) a live slot or 0, copied to the live slot [0x7c24a0] (a dead one faults in both)
        thing = bytearray(r.bytes(0x20))
        put(thing, 0xc, 'i', r.choice(live * 4 + [0]) if live else 0)
        if live:
            put(shown, 0, 'i', r.choice(live))
        regions[THING] = thing
        put(shown, 0x38, 'I', r.choice([THING, THING, THING, 0]))  # a random pointer would only fault in both
        return Call(CALLS[op], [], regions)
    rect = bytearray(r.bytes(0x10))
    put(rect, 4, 'HHHH', *(word(r) for _ in range(4)))
    thing = bytearray(r.bytes(0x20))
    put(thing, 0xc, 'i', r.choice(live * 3 + [0, 0]) if live else 0)
    regions.update({RECT: rect, THING: thing})
    if r.random() < 0.7:
        regions[0x74f898] = struct.pack('<I', r.choice([f32(480.0), f32(600.0), f32(768.0), 0, f32(-1.0), r.getrandbits(32)]))
    if op in ('Preview408', 'Preview3b0'):   # (the panel's window +0x18: its saved rectangle +0x2c and depth +0xe0)
        window = bytearray(r.bytes(0x100))
        put(window, 0x2c, 'IHHHH', 0x705920, *(word(r) for _ in range(4)))
        put(window, 0xe0, 'H', r.choice([0, 1, 2, 5, 0x100, r.randrange(-300, 300) & 0xffff, r.getrandbits(16)]))
        regions[PANEL] = put(bytearray(r.bytes(0x20)), 0x18, 'I', WINDOW)
        regions[WINDOW] = window
        regions[0x7c2658] = struct.pack('<I', r.choice([THING] * 6 + [0]))
        return Call(CALLS[op], [PANEL], regions, conv='t', ret='v')   # (EAX the rectangle's destructor's: a stack address)
    depth = r.choice([0, 1, 2, 5, 0x100, r.randrange(-300, 300), r.getrandbits(32)]) & 0xffffffff
    return Call(CALLS[op], [RECT, THING, depth, scale(r)], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

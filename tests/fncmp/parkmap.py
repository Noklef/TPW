# The park map's squares classified (src/game/world/parkmap: 0x5f2050) against the original: a map of up to 8 x 8 squares (now and then 0 wide or
# high) over the map cells from (0, 0) laid out, their 0x44-byte cells (types 0 to 0x15 and a few others, ids mostly their own, owners among the
# laid ids, flag 0x40 now and then) and 0x28-byte entries (track types, 0xc / 0x11 with parents among the laid ids, 0x19, others), the owners'
# thing lists (heads at +0x24, linked by +0xa without cycles) over the park of thingenv (things of models 3 mostly, definitions found by id whose
# map kind +0x4ac is 0 to 3 mostly) and the entries' old contents random. The sprite placed on it (0x5f4180): see place().
# usage: parkmap.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Classify,PlaceSprite)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, POOL, THING, park  # noqa: E402

B = Bench([(0x5f2050, 0x5f2363), (0x536310, 0x536490), (0x535db0, 0x535dd0), (0x4d0af0, 0x4d0b10), (0x53ad30, 0x53adb0), (0x4dd4e0, 0x4dd510),
           (0x412e90, 0x412f00), (0x5da3c0, 0x5da3c1), (0x5f4180, 0x5f42c6), (0x56b570, 0x56b5d0), (0x56b620, 0x56b628)])
MAP = RAM + 0x40000
SPRITE, FRAME, ARRAY, KEYS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
OPS = [k for k in os.environ.get('OPS', 'Classify,PlaceSprite').split(',') if k]
WORLD = RAM + 0x80000
CELL_TYPES = [0, 1, 2, 3, 4, 9, 0xa, 0x15, 0xb, 0xd, 0x10, 0x19, 0xc, 0x5, 0x1e, 0x104]
ENTRY_TYPES = [0xb, 0xd, 0x10, 0x12, 0x19, 0xc, 0x11, 0xc, 0x11, 0x19, 0, 1, 0xe, 0x10b]


def ident(x, y):
    return ((y << 7) + x + 1) & 0xffff


def place(r):
    # PlaceSprite: the map's width and height (+0x24, +0x28; 0 or negative now and then), view origin and zoom (+0x54-+0x5c), the sprite's
    # position (+4, +8), layer (+0x14 about 0-2) and size (+0x24) floats or anything; the screen size (0x7a1968 / 0x7a1930) the usual ones
    # or anything; the frame among the array's 0-3 keys at 0x879ce0 or not, its +0x28 an image whose word +0xc is random.
    m = bytearray(r.bytes(0x60))
    put(m, 0x24, 'ii', *(r.choice([r.randrange(1, 0x81), 0x80, 0x40, 0, -3, r.getrandbits(31)]) for _ in range(2)))
    r.floats(m, (0x54, 0x58))
    put(m, 0x5c, 'I', r.choice([r.f(4), r.pos(4), 0x3f800000]))
    sprite = bytearray(r.bytes(0x30))
    r.floats(sprite, (4, 8), r.choice([128, 1000]))
    put(sprite, 0x14, 'i', r.choice([-1, 0, 1, 2, 3, r.getrandbits(31) - (1 << 30)]))
    put(sprite, 0x24, 'I', r.f(r.choice([1, 50])))
    put(sprite, 0x28, 'I', FRAME)
    frame = put(bytearray(r.bytes(0x40)), 0x28, 'I', FRAME + 0x30)
    n = r.choice([0, 1, 2, 3])
    keys = [FRAME, KEYS, KEYS + 0x20]
    r.shuffle(keys)
    array = put(bytearray(r.bytes(0x20)), 8, 'I', KEYS + 0x40)
    put(array, 0x1c, 'I', n)
    screen = r.choice([(640, 480), (800, 600), (1024, 768), (r.getrandbits(32), r.getrandbits(32)), (0, 0)])
    regions = {MAP: m, SPRITE: sprite, FRAME: frame, KEYS: put(bytearray(r.bytes(0x50)), 0x40, '3I', *keys), 0x879ce0: array,
               0x7a1968: struct.pack('<I', screen[0]), 0x7a1930: struct.pack('<I', screen[1])}
    return Call(0x5f4180, [MAP, SPRITE], regions, conv='t', ret='v')


def case(r):
    if r.deal(OPS) == 'PlaceSprite':
        return place(r)
    w = r.choice([r.randrange(1, 9), r.randrange(1, 9), r.randrange(1, 4), 0])
    h = r.choice([r.randrange(1, 9), r.randrange(1, 9), r.randrange(1, 4), 0, -1])
    regions, handles, _addrs = park(r, count=r.choice([2, 3, 4, 6, 6]), kinds=(3, 3, 3, 3, 1, 2, 0))
    things = bytearray(regions[POOL])
    order = handles[:]
    r.shuffle(order)
    for i, hd in enumerate(order):   # (a list without cycles: each thing's next is a later one, or 0)
        put(things, THING * handles.index(hd) + 0xa, 'H', order[i + 1] if i + 1 < len(order) and r.random() < 0.8 else 0)
    regions[POOL] = things
    items = bytearray(regions[ITEMS])
    for i in range(NITEMS):
        put(items, i * ITEM + 0x4ac, 'I', r.choice([0, 1, 2, 3, 3, 0, 4, 0xffffffff]))
    regions[ITEMS] = items
    rows = max(h, 1)
    laid = [ident(x, y) for y in range(rows) for x in range(max(w, 1))]
    heads = [0, 0] + order
    for y in range(rows):
        cells = bytearray(r.bytes(max(w, 1) * 0x44))
        entries = bytearray(r.bytes((max(w, 1) + 1) * 0x28))   # (from id y * 128, one before the row)
        put(entries, 0x10, 'H', r.choice(laid))
        for x in range(max(w, 1)):
            c = x * 0x44
            put(cells, c, 'H', ident(x, y) if r.random() < 0.9 else r.choice(laid))
            put(cells, c + 8, 'I', r.choice(CELL_TYPES))
            put(cells, c + 0xe, 'H', r.choice([0, 0x40, 0, r.getrandbits(16)]))
            put(cells, c + 0x10, 'H', r.choice(laid))
            put(cells, c + 0x24, 'H', r.choice(heads))
            e = (x + 1) * 0x28
            put(entries, e + 8, 'I', r.choice(ENTRY_TYPES))
            put(entries, e + 0x10, 'H', r.choice(laid))
        regions[WORLD + 0x2d8 + (ident(0, y) - 1) * 0x44] = cells
        regions[WORLD + (ident(0, y) - 1 + 0x6cde) * 0x28] = entries
    regions[0x7cf83c] = struct.pack('<I', WORLD)
    regions[0x8023a0] = struct.pack('<I', WORLD + 0x2d8)
    regions[MAP] = put(bytearray(r.bytes(0x60 + rows * 0x400)), 0x24, 'ii', w, h)
    return Call(0x5f2050, [MAP], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

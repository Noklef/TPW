# The park map's squares classified (src/game/world/parkmap: 0x5f2050) against the original: a map of up to 8 x 8 squares (now and then 0 wide or
# high) over the map cells from (0, 0) laid out, their 0x44-byte cells (types 0 to 0x15 and a few others, ids mostly their own, owners among the
# laid ids, flag 0x40 now and then) and 0x28-byte entries (track types, 0xc / 0x11 with parents among the laid ids, 0x19, others), the owners'
# thing lists (heads at +0x24, linked by +0xa without cycles) over the park of thingenv (things of models 3 mostly, definitions found by id whose
# map kind +0x4ac is 0 to 3 mostly) and the entries' old contents random. The sprite placed on it (0x5f4180): see place(). The view
# origin applied (0x5f1050): see view(). The overview laid out (0x5f3230): see lay_out(). Its sprites added (0x5f3940): see
# add_sprites().
# usage: parkmap.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Classify,PlaceSprite,ApplyView,LayOut,AddSprites)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import CDECL_LOG4, HEAP, LOG, Bench, Call, RAM, fb, heap, log_region, put  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, POOL, THING, park  # noqa: E402

B = Bench([(0x5f2050, 0x5f2363), (0x536310, 0x536490), (0x535db0, 0x535dd0), (0x4d0af0, 0x4d0b10), (0x53ad30, 0x53adb0), (0x4dd4e0, 0x4dd510),
           (0x412e90, 0x412f00), (0x5da3c0, 0x5da3c1), (0x5f4180, 0x5f42c6), (0x56b570, 0x56b5d0), (0x56b620, 0x56b628),
           (0x5f1050, 0x5f1123), (0x42aab0, 0x42ab1d), (0x658d9f, 0x658e04), (0x659710, 0x659740), (0x67a830, 0x67a860),
           (0x65d7bc, 0x65d832), (0x65f697, 0x65f6e1), (0x5f3230, 0x5f35aa), (0x577940, 0x577990),
           (0x5f3940, 0x5f4175), (0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360), (0x56af20, 0x56afa0), (0x4e2650, 0x4e266d), (0x5f4370, 0x5f43fa), (0x56b4c0, 0x56b56e),
           (0x588d70, 0x588d90), (0x5e42d0, 0x5e4320), (0x5f4490, 0x5f44d0), (0x67a830, 0x67a860)])
MAP = RAM + 0x40000
SPRITE, FRAME, ARRAY, KEYS, WINDOW = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
OPS = [k for k in os.environ.get('OPS', 'Classify,PlaceSprite,ApplyView,LayOut,AddSprites').split(',') if k]
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


LIST, NODES = RAM + 0x500, RAM + 0x600


def lay_out(r):
    # LayOut: place()'s map, screen and frame; the map's +0x48 / +0x4c (the limits less 1, unsigned) small, 0 or anything; the origin now and
    # then about the edges (the clamps), or NaN or huge with a small width or zero or NaN zoom; 0-3 sprites on the list at +0x20080 (place()'s, sharing its frame), and the backdrop at 0x87aee8 any.
    c = place(r)
    regions = dict(c.regions)
    m = bytearray(regions[MAP])
    put(m, 0x48, 'II', *(r.choice([r.randrange(1, 0x81), 0x80, 1, 0, r.getrandbits(32)]) for _ in range(2)))
    if r.random() < 0.3:
        put(m, 0x54, 'II', *(fb(r.choice([0.0, -1.0, 1.0, 64.0, 127.5, r.uniform(-50, 200)])) for _ in range(2)))
    if r.random() < 0.25:   # (the edges NaN or past the float range: the centre huge, the zoom 0 or NaN)
        put(m, 0x54, 'II', *(r.choice([0x7fc00000, 0x7f800000, 0xff800000, fb(3e38), fb(-3e38), fb(1e37), fb(64.0)]) for _ in range(2)))
        put(m, 0x24, 'ii', *(r.choice([1, 2, -1, 0x100]) for _ in range(2)))
        if r.random() < 0.7:
            put(m, 0x5c, 'I', r.choice([0, 0, 0, 0x80000000, 0x7fc00000, fb(1e-30), fb(-1.0)]))
    regions[MAP] = m
    n = r.choice([0, 1, 2, 3])
    nodes = bytearray(r.bytes(0x18))
    for i in range(n):
        sprite = bytearray(r.bytes(0x30))
        r.floats(sprite, (4, 8), r.choice([128, 1000]))
        put(sprite, 0x14, 'i', r.choice([-1, 0, 1, 2, 3]))
        put(sprite, 0x24, 'I', r.f(r.choice([1, 50])))
        put(sprite, 0x28, 'I', FRAME)
        regions[RAM + 0x700 + 0x40 * i] = sprite
        put(nodes, 8 * i, 'II', RAM + 0x700 + 0x40 * i, NODES + 8 * (i + 1) if i + 1 < n else 0)
    regions[NODES] = nodes
    regions[LIST] = put(bytearray(r.bytes(0x10)), 4, 'I', NODES if n else 0)
    regions[MAP + 0x20080] = struct.pack('<I', LIST)
    regions[0x87aee8] = r.bytes(0x40)
    return Call(0x5f3230, [MAP], regions, conv='t', ret='v')


def view(r):
    # ApplyView: the map's width and height (+0x24, +0x28) small, 0, 2^24 + 3 (the limit unrounded) or anything; the origin (+0x54, +0x58)
    # floats about them (below 0, on the limit, past it by a little or much, NaN, infinities); the camera's globals and the window anything,
    # its handler (+0x114) the logging one mostly.
    m = bytearray(r.bytes(0x60))
    for off in (0x24, 0x28):
        n = r.choice([r.randrange(1, 0x81), 0x80, 1, 0, 0x1000003, 0x80000000, r.getrandbits(32)])
        put(m, off, 'I', n)
        v = (n - 1 if n < 0x80000001 else n - 1 - (1 << 32))
        put(m, off + 0x30, 'I', r.choice([fb(v), fb(v + 0.5), fb(v - 0.25), fb(v + 1), fb(r.uniform(-2, 0x90)), r.f(), fb(-0.0)]))
    win = bytearray(r.bytes(0x120))
    win[4] = r.choice([0, 0, 0, 1])
    put(win, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, CDECL_LOG4, 0]))
    put(m, 4, 'I', WINDOW)
    regions = {MAP: m, WINDOW: win, LOG: log_region(), 0x7908f0: r.bytes(0x1a0), 0x74c9bc: r.bytes(4)}
    return Call(0x5f1050, [MAP], regions, conv='t', ret='a')


IMAGES, SHEADER, SITEMS, SLIST, SNODE = RAM + 0x800, RAM + 0x1000, RAM + 0x1100, RAM + 0x1800, RAM + 0x1900
THINGS = RAM + 0x10000


def add_sprites(r):
    # AddSprites: place()'s map and screen; the map's six images (+0x20068-+0x2007c) among three images whose word +0xc is random; a park
    # (thingenv's, the things at THINGS) of kinds 1, 4-7 mostly (3 and 0 now and then); the map's 0-5 x 0-5 squares (now and then 0 wide
    # or high, or -1 high) over the world's cells, their kinds (+0x2c) 0 mostly, 7, 8, 1 or any; the list from the world's +0x1da746
    # (the park's things linked by +0xc, flags +0x32 with 0x10 now and then); the sizes' ints at 0x785610 / 58 / 88 / 40 / 70 small or
    # any; the sprite array at 0x879ce0 with 0-3 frames and the list at [0xfa6e38] empty or not; the heap failing now and then.
    c = place(r)
    regions = dict(c.regions)
    images = bytearray(r.bytes(0x30))
    regions[IMAGES] = images
    regions[MAP + 0x20068] = struct.pack('<6I', *(IMAGES + 0x10 * r.randrange(3) for _ in range(6)))
    w = r.choice([r.randrange(1, 6), r.randrange(1, 6), r.randrange(1, 3), 0])
    h = r.choice([r.randrange(1, 6), r.randrange(1, 6), r.randrange(1, 3), 0, -1])
    m = bytearray(regions[MAP])
    put(m, 0x24, 'ii', w, h)
    regions[MAP] = m
    park_regions, handles, _addrs = park(r, count=r.choice([0, 1, 2, 3, 4, 6]), kinds=(1, 4, 5, 6, 7, 1, 4, 5, 6, 7, 3, 0), defs=False, pool=THINGS)
    regions.update(park_regions)
    things = bytearray(regions[THINGS])
    order = handles[:]
    r.shuffle(order)
    for i, hd in enumerate(order):   # (a list without cycles: each thing's next is a later one, or 0)
        t = THING * handles.index(hd)
        put(things, t + 0xc, 'H', order[i + 1] if i + 1 < len(order) and r.random() < 0.85 else 0)
        things[t + 0x32] = r.choice([0x10, 0x10, 0, 0xef, r.getrandbits(8)])
    regions[THINGS] = things
    world = bytearray(r.bytes(4))
    regions[WORLD + 0x1da744] = put(world, 2, 'H', order[0] if order and r.random() < 0.9 else 0)
    for y in range(max(h, 1)):
        cells = bytearray(r.bytes(max(w, 1) * 0x44))
        for x in range(max(w, 1)):
            put(cells, x * 0x44 + 0x2c, 'I', r.choice([0, 0, 0, 0, 1, 7, 8, r.getrandbits(32)]))
        regions[WORLD + 0x2d8 + (ident(0, y) - 1) * 0x44] = cells
    regions[0x7cf83c] = struct.pack('<I', WORLD)
    for g in (0x785610, 0x785658, 0x785688, 0x785640, 0x785670):
        regions[g] = struct.pack('<i', r.choice([0, 1, 2, 5, 16, -3, r.getrandbits(31)]))
    n = r.choice([0, 1, 3])
    arr = put(put(bytearray(r.bytes(0x20)), 4, 'II', SHEADER, SITEMS), 0x1c, 'I', n)
    regions[0x879ce0] = arr
    regions[SHEADER] = r.bytes(0x10)
    regions[SITEMS] = r.bytes(0x400)
    empty = r.random() < 0.4
    regions[SLIST] = put(bytearray(r.bytes(0x10)), 4, 'III', *((0, 0, 0) if empty else (SNODE, SNODE, 1)))
    regions[SNODE] = r.bytes(0xc)
    regions[0xfa6e38] = struct.pack('<I', SLIST)
    regions[HEAP] = heap(0x6000, r.choice([0] * 20 + [1, 2, 4, 7]))
    return Call(0x5f3940, [MAP], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'PlaceSprite':
        return place(r)
    if op == 'ApplyView':
        return view(r)
    if op == 'LayOut':
        return lay_out(r)
    if op == 'AddSprites':
        return add_sprites(r)
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

# The mouse modes' constructors, destructors and empty methods (src/game/world/mousemode:
# 0x46c450-0x46d000) and the statics and frame step before them (0x46c070-0x46c100) against
# the originals: each on a random object (the destructors' frees through heap()); and TileAt, the pointer's tile from a position
# (0x45d560) on a map's grid of random origins, cell sizes and dimensions; Hover (0x46c480): a sprite slot or none, ready or not, at a
# highlight point of any floats, over a cell (and its handle's entry and parent) of any type and flags, with a face queue part full or full.
# Fps (0x46c0d0): the FPS text printed into the pointer state and handed to 0x8bba50. CMMOnline's constructor and the modes' Assign
# (0x46ce80, 0x46d590, 0x46c730, 0x46cbf0): see assign().
# usage: mousemode.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import struct

from fncmp import HEAP, LOG, Bench, Call, RAM, fb, global_heap, heap, log_region, put  # noqa: E402

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cursor  # noqa: E402

B = Bench([(0x46c450, 0x46c453), (0x46c460, 0x46c480), (0x46c580, 0x46c5c6), (0x46c650, 0x46c653),
           (0x46c6a0, 0x46c6b0), (0x46c6d0, 0x46c702), (0x46c710, 0x46c72e), (0x46cbc0, 0x46cbe1),
           (0x46ced0, 0x46ced7), (0x46cfc0, 0x46cfd0), (0x46cff0, 0x46d000), (0x46c070, 0x46c10e),
           (0x45d560, 0x45d60e), (0x4d8300, 0x4d8328), (0x46c480, 0x46c578), (0x45d610, 0x45d616), (0x475480, 0x4754d4),
           (0x535db0, 0x535dc3), (0x536300, 0x536484), (0x53ad20, 0x53aefc), (0x53c8d0, 0x53c9c0), (0x5da3c0, 0x5da3c1),
           (0x4d0a70, 0x4d0a86), (0x4d0a90, 0x4d0ab1), (0x4d0ac0, 0x4d0ae1), (0x4d0af0, 0x4d0b06), (0x4d0b10, 0x4d0b27), (0x522700, 0x5228b5),
           (0x46c0d0, 0x46c0ff), (0x57cb10, 0x57cb28), (0x67a400, 0x6902c0),
           (0x46ce80, 0x46ceca), (0x46d590, 0x46d59d), (0x46c730, 0x46c87a), (0x46cbf0, 0x46cc37), (0x402d70, 0x403220), (0x4033a0, 0x4033f0),
           (0x475550, 0x4755a0), (0x4758f0, 0x475b90), (0x540c60, 0x540da0), (0x5f5660, 0x5f56a0), (0x5f5e20, 0x5f5e90), (0x5f5fa0, 0x5f5fc0),
           (0x4814a0, 0x4814e3), (0x4a2aa0, 0x4a2abf), (0x6590f7, 0x6591d4), (0x65f11d, 0x65f14e), (0x66190a, 0x66191b), (0x661d96, 0x661dca)])
HTYPES = [0, 0, 1, 1, 3, 9, 2, 4, 0xb, 0xc, 0xd, 0x10, 0x11, 0x12, 0x19, 0x1e]
CELLS = RAM + 0x1000   # the 0x44-byte cells: [0x7cf83c] + 0x294
ENTRIES = CELLS - 0x294 + 0x1102b0   # the 0x28-byte entries of handles 0-5
# op: (address, how many arguments after self)
CALLS = {'Nop4': (0x46c450, 4), 'Nop2': (0x46c650, 2), 'Dtor': (0x46ced0, 0), 'DelDtor': (0x46c460, 1),
         'DerivedDelDtor': (0x46c710, 1), 'Tile': (0x46c580, 1), 'TileKind': (0x46c5a0, 2),
         'OnlineBase': (0x46c6a0, 0), 'PlaceStaff': (0x46c6d0, 4), 'StaffDrop': (0x46cbc0, 2),
         'Aa0': (0x46cfc0, 0), 'Ad0': (0x46cff0, 0), 'NewFrame': (0x46c100, 0)}
STATICS = {'One': 0x46c070, 'Epsilon': 0x46c090, 'Tile': 0x46c0b0}  # 0x7b05a0-0x7b05d3


def hover(r):
    def num():
        return fb(r.choice([r.uniform(-50, 1300), r.uniform(-50, 1300), 0.0, -5.0, 5.0, -0.0, 1e-40, 3e38, float('inf'), float('nan')]))
    self = put(bytearray(r.bytes(0x20)), 8, 'i', r.choice([1, 2, 3, 1, 2, 3, 0, 0, 4, 5, -1]))
    g = put(bytearray(r.bytes(0x2c)), 0, 'Ii', RAM + 0x400, r.choice([4, 4, 4, 2, 1]))
    put(g, 0x24, 'I', r.choice([1, 1, 1, 1, 0]))
    tile = r.choice([0, 1, 2, 3, 0])
    cells = bytearray(r.bytes(4 * 0x44))
    entries = bytearray(r.bytes(6 * 0x28))
    for c, k in [(cells, tile * 0x44)] + [(entries, i * 0x28) for i in range(6)]:
        put(c, k + 8, 'I', r.choice(HTYPES + [r.getrandbits(32)]))
        put(c, k + 0xe, 'H', r.choice([0, 0, 0x40, 0x400, 0x8, r.getrandbits(16)]))
        put(c, k + 0x10, 'H', r.choice([0, 1, 2, 5, r.randrange(6)]))
    put(cells, tile * 0x44, 'H', r.randrange(6))
    if r.random() < 0.3:   # a parented entry (parent 0 too, which only the type-0x19 test follows) of a parent of type 0x19 or not
        h, p = r.randrange(1, 6), r.choice([0, 0, r.randrange(6)])
        put(entries, h * 0x28 + 8, 'I', r.choice([0xc, 0x11]))
        put(entries, h * 0x28 + 0x10, 'H', p)
        put(entries, p * 0x28 + 8, 'I', r.choice([0x19, 0x19, 0xd, 1]))
        put(cells, tile * 0x44, 'H', h)
    n = r.choice([0, 1, 2, 5, 0x806])
    tile_word = put(bytearray(r.bytes(4)), 0, 'H', tile)
    regions = {RAM: self, 0x7b49f0: g, RAM + 0x400: struct.pack('<5I', 0, RAM + 0x500, RAM + 0x600, RAM + 0x700, RAM + 0x800),
               RAM + 0x500: r.bytes(0xc4), RAM + 0x600: r.bytes(0xc4), RAM + 0x700: r.bytes(0xc4), RAM + 0x800: r.bytes(0xc4),
               0x7a1c90: struct.pack('<3I', num(), num(), num()), 0x7b05cc: tile_word, CELLS: cells, ENTRIES: entries,
               0x7cf83c: struct.pack('<I', CELLS - 0x294), 0x8023a0: struct.pack('<I', ENTRIES - 0x10ffd8),
               0x81b4cc: bytes([r.choice([0, 1, r.getrandbits(8)])]) + r.bytes(3),
               0x820b00: r.bytes(0xa0 * 7), 0x872168: r.bytes(0x40), 0x871ee0: struct.pack('<I', n),
               0x763c98: bytes([r.choice([0, 1, 1])]) + bytes(3), 0x874fc4: struct.pack('<I', r.choice([0, 0, 1]))}
    return Call(0x46c480, [RAM] + [r.getrandbits(32) for _ in range(4)], regions, conv='t', ret='v')


ASSIGNS = {'Online': 0x46ce80, 'AssignAd0': 0x46d590, 'AssignPlace': 0x46c730, 'AssignDrop': 0x46cbf0}
SCRIPTS, STABLE, SPRITES = 0x7b49f0, RAM + 0x3000, RAM + 0x4000


def assign(r, op):
    """CMMOnline's constructor 0x46ce80 and the modes' Assign (slot 10): the cursor set through 0x4814a0 (the windows as cursor.py's ObjSet),
    and for the staff modes a sprite script started (the list ready or not, its slot table full (then grown by 50) or not, scripts from the
    heap, the clock frozen; folders 0-9 of 1-4 sprites, kinds about 4-8) with the slot at +8 (0, or one of the table's) killed."""
    regions = {LOG: log_region(), HEAP: heap(), **global_heap()}
    cursor.cursor_env(r, regions)
    wins = [cursor.WINDOW + 0x80 * i for i in range(3)]
    for w in wins:
        regions[w] = bytearray(r.bytes(0x60))
    put(regions[0xfaa5e0], 0, 'I', r.choice(wins + [0]))
    regions[0x7cb2a8] = put(bytearray(r.bytes(0x38)), 4, 'I', r.choice([wins[0], wins[0], 0]))
    regions[0x7cc8bc] = put(bytearray(8), 0, 'II', r.choice([wins[1], 0]), r.choice([wins[2], 0]))
    self = bytearray(r.bytes(0x20))
    n = r.choice([1, 2, 3, 4])
    put(self, 8, 'I', r.choice([0, 0, 1, 2, 3, n - 1]))
    put(self, 0x10, 'BB', *(r.choice([0, 1, 2, 3, 0, 1, r.getrandbits(8)]) for _ in range(2)))   # (the sprite: mostly one the folder has)
    put(self, 0xc, 'I', r.choice([4, 5, 6, 7, 8, 4, 5, 6, 7, 8, 3, 9, 0, r.getrandbits(32)]) if op == 'AssignPlace' else r.randrange(10))
    regions[RAM] = self
    folders = bytearray(12 * 10)
    for k in range(10):
        put(folders, 12 * k + 4, 'II', r.randrange(8), r.choice([1, 2, 3, 4]))
    regions[0x763f88] = folders
    regions[SPRITES] = r.bytes(0x240 * 4)
    regions[0x876b48] = b''.join(struct.pack('<I', SPRITES + 0x240 * r.randrange(4)) for _ in range(16))
    table = bytearray(struct.pack('<4I', *[r.choice([0, RAM + 0x3800]) for _ in range(4)]))
    if r.random() < 0.4:
        table[4:] = struct.pack('<3I', RAM + 0x3800, RAM + 0x3800, RAM + 0x3800)
    regions[STABLE] = table
    regions[RAM + 0x3800] = r.bytes(0x120)
    regions[SCRIPTS] = struct.pack('<11I', STABLE, n, 0, 0, 0, 0, 0, 0, 0, r.choice([1, 1, 1, 0]), 0)
    regions[0x785970] = put(bytearray(r.bytes(0x80)), 0x38, 'I', 1)
    return Call(ASSIGNS[op], [RAM], regions, conv='t', ret='a' if op == 'Online' else 'v')


def case(r):
    op = r.choice(OPS)
    if op in ASSIGNS:
        return assign(r, op)
    if op == 'Hover':
        return hover(r)
    if op == 'TileAt':   # a map with a grid (or not): origins and cell sizes whole or odd (0, NaN), dims about 128 (0, 1 or huge)
        def num(*usual):
            return fb(r.choice(list(usual) * 3 + [0.0, -10.0, r.uniform(-50, 50), 1e10, float('nan')]))
        grid = bytearray(r.bytes(0x20))
        struct.pack_into('<4I', grid, 8, num(0.0, 0.0, 5.0), num(0.0, 0.0, -5.0), num(10.0, 10.0, 2.5), num(10.0, 10.0, 4.0))
        struct.pack_into('<2I', grid, 0x18, *(r.choice([128, 128, 128, 100, 1, 0, 200, r.getrandbits(32)]) for _ in range(2)))
        mp = put(bytearray(r.bytes(0x70)), 0x6c, 'I', r.choice([RAM + 0x200] * 9 + [0]))
        pos = [fb(r.choice([r.uniform(-30, 1400), r.uniform(-30, 1400), r.uniform(0, 1280), 0.0, 5.0, -5.0, 1279.5, 2e3, -3e9, 1e20,
                            float('nan')])) for _ in range(3)]
        dims = [r.choice([128, 128, 128, 100, 1, 0, -1, 200]) for _ in range(2)]
        regions = {0x7a0854: struct.pack('<I', r.choice([RAM + 0x100] * 9 + [0])), RAM + 0x100: mp, RAM + 0x200: grid,
                   RAM + 0x300: struct.pack('<3I', *pos), 0x7a0314: struct.pack('<i', dims[0]), 0x7a0374: struct.pack('<i', dims[1]),
                   0x7b05a0: bytearray(r.bytes(0x34))}
        return Call(0x45d560, [RAM + 0x300], regions, ret='v')
    if op == 'Fps':   # (fps where the driver's stand-in sprintf, the host's, prints %g as MSVC6 does: no exponent, no NaN or infinity)
        fps = r.choice([0.0, 30.0, 60.0, 59.94, 0.5, -12.25, r.uniform(0, 200), r.uniform(-1000, 1000), float(r.randrange(100000))])
        # (the CRT's float conversions table as its startup sets it, 0x67a610: _cfltcvt, _cropzeros, _fassign, _forcdecpt, _positive, _cldcvt)
        cfltcvt = struct.pack('<6I', 0x680a30, 0x6805d0, 0x680660, 0x680570, 0x680640, 0x680a30)
        regions = {RAM: bytearray(r.bytes(0x40)), 0x8bba50: bytearray(r.bytes(0x20)), 0x77eca0: cfltcvt}
        return Call(0x46c0d0, [RAM, fb(fps)], regions, conv='t', ret='a')
    if op in STATICS:
        return Call(STATICS[op], [], {0x7b05a0: bytearray(r.bytes(0x34))})
    at, n = CALLS[op]
    args = [r.choice([0, 1, 2, 3, r.getrandbits(32)]) for _ in range(n)]
    regions = {RAM: bytearray(r.bytes(0x40)), HEAP: heap(0x100)}
    ret = 'v' if op in ('Nop4', 'Nop2', 'Dtor', 'NewFrame') else 'a'
    return Call(at, [RAM] + args, regions, conv='t', ret=ret)


OPS_ALL = list(CALLS) + list(STATICS) + ['TileAt'] * 3 + ['Hover'] * 10 + ['Fps'] * 2 + list(ASSIGNS) * 3
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

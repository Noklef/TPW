# The coaster editor's adjust tool toggles (src/game/coaster/edittools: 0x441cd0, 0x441e80, 0x442010) against the originals: the editor's
# flags (no tool, tool 2 or 4, another, with 0x200 / 0x1e00 / 0x2000 or random high bits), a piece being edited or none (its definition's
# flag 2 or not, its type's flags 0x40 / 0x80 / 0x100 / 0x200 / 0x180 or others, values about the thresholds, odd floats), the game's +0x40
# set or not (Toggle 0x4421e0 picks F8 or Angles by the piece's flag 0x40), the action recorder at 0x785a08 recording into a record stream or not, a current highlight or none.
# The drag tool 0x441a00 (tool 1) with drag()'s world: neighbours, the train, the type's lengths and costs, the money the cost goes to.
# usage: edittools.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=F8,E0,Angles,Toggle,Drag)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, STUBS, Bench, Call, put  # noqa: E402

CALLS = {'F8': 0x441cd0, 'E0': 0x441e80, 'Angles': 0x442010, 'Toggle': 0x4421e0, 'Drag': 0x441a00}
B = Bench([(0x441cd0, 0x441e78), (0x441e80, 0x442002), (0x442010, 0x4421d6), (0x4421e0, 0x44221b), (0x403400, 0x403fc6), (0x45d630, 0x45d660), (0x5da3c0, 0x5da3d0),
           (0x441a00, 0x441cc8), (0x433280, 0x433310), (0x434650, 0x4346d0), (0x43e160, 0x43e1c0), (0x447830, 0x4479a0), (0x4d0190, 0x4d0270), (0x519510, 0x519560),
           (0x5195d0, 0x519620), (0x6ad810, 0x6ad830)])
PIECE, DEF, TYPE, GAME, VT, BUF = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600
TOOLS = [0, 0, 0, 2, 2, 4, 4, 4, 1, 6, 0x20, 0x3f]
TYPES = [0x40, 0x80, 0x100, 0x180, 0x200, 0x240, 0x380, 0]


def value(r):
    return struct.unpack('<f', struct.pack('<I', r.getrandbits(32)))[0] if r.random() < 0.1 else \
        r.choice([0.5, 0.75, 1.5, 0.25, -0.3, 0.0, 3.0, 1e-38, -1e-38, 5e-39, 3e38, r.uniform(-4, 4), r.uniform(-4, 4)])


OTHER, FORCE, FLAGS, FALL, MONEY, WOBJ, THING, TIME = RAM + 0x800, RAM + 0x920, RAM + 0x940, RAM + 0xa00, RAM + 0xc00, RAM + 0x1000, RAM + 0x40000, RAM + 0x80000
OTHER2 = RAM + 0xe00   # (the second neighbour: +0x64 and +0x68 can differ)


def drag(r, regions):
    """0x441a00's world, over case()'s: the piece's neighbours (+0x64 / +0x68: OTHER or none; +0x68 is also Car_TravelCost's previous piece), its
    definition's train (+0x30 a count 0-3 of cars from +0x34: the piece, OTHER or none) and next owner (+0x2c), the type's lengths and costs, the
    game's fallback type (+4 -> +0xa8), the kept cost 0x79c698, and the money of [0x7cf83c] (its table slot, balance, flags, the time)."""
    piece, definition, kind, game = (bytearray(regions[a]) for a in (PIECE, DEF, TYPE, GAME))
    piece += bytes(0x110 - len(piece))
    put(piece, 4, 'I', r.choice([0, 1, 2, 3]))
    put(piece, 8, 'I', r.getrandbits(32) & ~0xc0 | r.choice([0, 0x80]) | r.choice([0, 0x40, 0x40]))
    put(piece, 0x64, 'I', r.choice([OTHER, OTHER, OTHER2, 0]))
    put(piece, 0x68, 'I', r.choice([OTHER, OTHER, OTHER2, OTHER2, 0]))
    for off in (0x4c, 0x54, 0xe8):
        put(piece, off, 'f', value(r))
    definition += bytes(0x40 - len(definition))
    n = r.choice([0, 1, 2, 3])
    put(definition, 0x2c, 'I', r.choice([FORCE, 0]))
    put(definition, 0x30, 'I', n)
    for k in range(3):
        put(definition, 0x34 + 4 * k, 'I', r.choice([PIECE, OTHER, OTHER, 0]))
    kind += bytes(0xd0 - len(kind))
    put(kind, 0, 'I', r.choice([0x20, 0x20, 0x20, 0, 0x4020, 0xc020, 0x8000]) | (r.getrandbits(32) & ~0xc020 if r.random() < 0.2 else 0))
    for off in (0x5c, 0x60, 0x64, 0x68, 0x6c, 0xbc, 0xc0, 0xc4):
        put(kind, off, 'f', value(r))
    if r.random() < 0.25:   # (length == shortest: Unlerp's 0 / x a signed zero, and 0 / 0 with longest == shortest a NaN)
        kind[0x68:0x6c] = kind[0xbc:0xc0]
        if r.random() < 0.4:
            kind[0xc0:0xc4] = kind[0xbc:0xc0]
    others = []
    for _ in range(2):
        other = put(put(put(bytearray(r.bytes(0x110)), 0x100, 'I', r.choice([DEF] * 6 + [0])), 4, 'I', r.choice([0, 1, 2])), 8, 'I', r.getrandbits(32))
        for off in (0x4c, 0x54, 0xe8):
            put(other, off, 'f', value(r))
        others.append(other)
    put(game, 4, 'I', FALL)
    fall = bytearray(r.bytes(0x110))
    put(fall, 0xa8, 'I', r.choice([0, 0x4000, 0xc000]))
    for off in (0x104, 0x108, 0x10c):
        put(fall, off, 'f', value(r))
    glob = bytearray(regions[0x79c600])
    put(glob, 0x98, 'f', r.choice([0.0, 1.0, 2.5, -3.75, 1e10, 100.0, value(r)]))
    money = bytearray(r.bytes(0x200))
    put(money, 0xc, 'i', r.choice([0, 1, 50, -1, 0x7fffffff, r.getrandbits(32) - (1 << 31)]))
    put(money, 0x114, 'I', r.choice([0, 1, 1, 1]))
    put(money, 0x11c, 'i', r.choice([-1, 0, 5, -0x80000000]))
    h1, h2 = r.choice([(1, 2), (2, 1), (3, 1)])   # (the thing's and the money's slots)
    slots = bytearray(r.bytes(20 * 4))
    put(slots, 20 * h1, 'I', THING)
    put(slots, 20 * h2, 'I', MONEY)
    regions.update({PIECE: piece, DEF: definition, TYPE: kind, GAME: game, OTHER: others[0], OTHER2: others[1], FORCE: put(bytearray(r.bytes(0x20)), 0x18, 'I', r.choice([FLAGS, 0])),
                    FLAGS: struct.pack('<I', r.choice([0, 0x4000, 0xc000, r.getrandbits(32)])), FALL: fall, 0x79c600: glob, MONEY: money,
                    0x7cf83c: struct.pack('<I', WOBJ - 0x1da720), WOBJ: struct.pack('<HHHH', h1, 0, 0, h2), 0x7cfb90: slots,
                    0x80239c: struct.pack('<I', TIME - 0x1da70c), TIME: r.bytes(4), THING + 0x1fc90: r.bytes(4), THING + 0x1f5a0: r.bytes(4)})


def case(r):
    op = r.deal(OPS)
    tool = {'E0': 2, 'Drag': 1}.get(op, 4)   # (the op's own tool, or none, more often; Toggle's are F8's and Angles')
    flags = r.choice(TOOLS + [0, 0, tool, tool]) | r.choice([0, 0, 0x200, 0x1e00, 0x2000, 0x400]) | (r.getrandbits(32) & 0xffffc000 if r.random() < 0.2 else 0)
    glob = bytearray(r.bytes(0x110))
    put(glob, 0x40, 'I', PIECE if r.random() < 0.85 else 0)
    put(glob, 0x58, 'I', flags)
    piece = bytearray(r.bytes(0x110))
    put(piece, 8, 'I', r.getrandbits(32) & ~0xc0 | r.choice([0, 0x80]) | r.choice([0, 0x40]))
    for off in (0xd4, 0xd8, 0xdc, 0xe0, 0xf8):
        put(piece, off, 'f', value(r))
    put(piece, 0x100, 'I', DEF)
    definition = bytearray(r.bytes(0x20))
    put(definition, 4, 'B', r.getrandbits(8) & ~2 | (2 if r.random() < 0.2 else 0))
    put(definition, 0x18, 'I', TYPE)
    kind = bytearray(r.bytes(0x98))
    put(kind, 0, 'I', r.choice(TYPES) | (r.getrandbits(32) & ~0x3c0 if r.random() < 0.2 else 0))
    put(kind, 0x94, 'f', value(r))
    if r.random() < 0.05:   # (two NaNs: which one comes out of the x87's bounds)
        put(piece, 0xd4, 'I', r.choice([0x7fc00001, 0xffc12345, 0x7f800001]))
        put(kind, 0x94, 'I', r.choice([0x7fc54321, 0xffe00000, 0x7f812345]))
    game = bytearray(r.bytes(0x44))
    put(game, 0x40, 'I', r.choice([0, 1, 1, r.getrandbits(32)]))
    rec = bytearray(r.bytes(0x28))   # (recording, mode, playback; its first stream a record stream at +0xc)
    put(rec, 0, 'I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    put(rec, 0x24, 'I', r.choice([0, 0, 0, 1]))
    put(rec, 0xc, '3I', VT, BUF, r.choice([0, 4, 9, 0x20, 0x20]))
    vt = bytearray(0x10)
    put(vt, 0xc, 'I', STUBS + 0x10)
    regions = {0x79c600: glob, PIECE: piece, DEF: definition, TYPE: kind, GAME: game, 0x785a08: rec, VT: vt, BUF: r.bytes(0x20),
               0x790fec: struct.pack('<I', GAME if r.random() < 0.95 else 0), 0x790ac8: r.bytes(8),
               0x7a1ce0: struct.pack('<I', r.choice([0, RAM + 0x700])), 0x7a1c7c: r.bytes(4)}
    if op == 'Drag':
        drag(r, regions)
    return Call(CALLS[op], [], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The build tools' checks of a map cell (src/game/world/cellbuild: MapCell_CanBuild 0x535670) against the original, over the originals of
# their lookups (the cell accessors, Kinds_Compatible, Cell_FirstObject, ObjectDefs_Find, Map_StepCellInDirection, the cost globals and
# the money's Source_Field0c): one world W ([0x7cf83c] = [0x80239c], gMapCells [0x8023a0] = W + 0x2d8) laid out only where a case reads it
# (the cell's 3 x 3 window of 0x44-byte cells, its 0x28-byte entry and its parent's, the anchor cells named by +0x10): the cell anywhere on
# the map (edges mostly, so steps go off it), its id word its own mostly; types the kinds the checks name (0-0x19) and others; flags
# +0xe / +0xf any of 0x10, 0x20, 0x40, 0x100 and the low nibble; the owner word (+0x10) the player's (0x818698) or [0x8186c0]'s now and
# then, else a window cell's id (the anchor of a type 4 / 9 / 0xa cell), whose thing chain (+0x24, links +0xa through 0x7cfb90) holds a
# kind 3 thing whose definition (0x7890a0: four, cached or not) has a priority (+0x54) about 0x818c10's; entries of types 0xc / 0x11 with
# a parent (0 now and then), track types 0xb-0x19 and others, neighbour bits making corners, straights and junctions (+0xc, or +0x22
# under 0x81b4cc); the direction pairs 0x7cdba0-0x7cdbd8; every flag global the checks read 0 or 1, the cost (0x81af4c) and the money
# about each other, the game free (0x785a2c) or not; kinds 0-0x3a with the named ones mostly, alone 0 or 1.
# usage: cellbuild.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=CanBuild)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

RANGES = [(0x412e90, 0x412f00), (0x4d0af0, 0x4d0b40), (0x4d8300, 0x4d8340), (0x4d8c00, 0x4d8c30), (0x4d96f0, 0x4d9a70), (0x50c320, 0x50c3f0),
          (0x5195d0, 0x519620), (0x522770, 0x5227f0), (0x522810, 0x522860), (0x527940, 0x527960), (0x52fb20, 0x52fbe0), (0x535600, 0x535de0),
          (0x536320, 0x5363a0), (0x53ad30, 0x53ad70), (0x53ad90, 0x53adc0), (0x53ae00, 0x53ae60), (0x53ae90, 0x53aed0), (0x6ad810, 0x6ad830),
          (0x5da3c0, 0x5da3c1)]
B = Bench(RANGES)
W = 0x32000000
THINGS, DEFS, MONEY, SEEN = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x2100
NSLOT = 16   # handles 0-13 things, 14 the money record, 15 none
CELL_TYPES = [0, 1, 1, 3, 3, 4, 9, 0xa, 0x15, 0x15, 0xb, 0x10, 0x19, 2, 7, 0xd, 0x12, 0x1e, 0x109]
ENTRY_TYPES = [0xc, 0x11, 0xc, 0xb, 0xd, 0x10, 0x12, 0x19, 0, 1, 3, 4, 0x15, 0x111]
BITS = [0x11, 0x44, 0x05, 0x14, 0x41, 0x50, 0x15, 0x55, 0x01, 0x04, 0x10, 0x40, 0, 0x22, 0xff]
PAIRS = [(0, -1), (1, -1), (1, 0), (1, 1), (0, 1), (-1, 1), (-1, 0), (-1, -1)]   # 0x7cdba0 + 8k (k = 0..6 used)


class Sparse:
    """Memory laid out piece by piece: pieces that overlap or touch are merged, so no two regions overlap."""

    def __init__(self, r):
        self.r, self.parts = r, {}

    def ensure(self, addr, n):
        lo, hi = addr, addr + n
        hits = [s for s, b in self.parts.items() if s <= hi and s + len(b) >= lo]
        if len(hits) == 1 and hits[0] <= lo and hits[0] + len(self.parts[hits[0]]) >= hi:
            return hits[0]
        start = min([lo] + hits)
        end = max([hi] + [s + len(self.parts[s]) for s in hits])
        buf = bytearray(self.r.bytes(end - start))
        for s in hits:
            b = self.parts.pop(s)
            buf[s - start:s - start + len(b)] = b
        self.parts[start] = buf
        return start

    def put(self, addr, fmt, *values):
        n = struct.calcsize('<' + fmt)
        s = self.ensure(addr, n)
        put(self.parts[s], addr - s, fmt, *values)

    def put8(self, addr, value):
        s = self.ensure(addr, 1)
        self.parts[s][addr - s] = value & 0xff


def cell44(ident):
    return W + 0x294 + (ident & 0xffff) * 0x44


def flags(r):
    return r.choice([0, 0, 0x10, 0x20, 0x40, 0x100, 0x120, 0x30, r.randrange(0x10), r.getrandbits(16)])


def lay_cell(r, mem, ident, window, own):
    a = cell44(ident)
    mem.ensure(a, 0x44)
    mem.put(a, 'H', ident if r.random() < 0.9 else r.choice(window))
    mem.put(a + 8, 'I', r.choice(CELL_TYPES) if r.random() < 0.95 else r.getrandbits(32))
    mem.put8(a + 0xc, r.choice(BITS))
    mem.put8(a + 0xd, r.choice([1, 4, 0x10, 0x40, 0, 0x11, r.getrandbits(8)]))
    mem.put(a + 0xe, 'H', flags(r))
    mem.put(a + 0x10, 'H', r.choice([own, own, r.choice(window), r.choice(window), 0]))
    mem.put8(a + 0x22, r.choice(BITS))
    mem.put(a + 0x24, 'H', r.choice([0, r.randrange(1, 14), r.randrange(1, 14), r.randrange(1, 14)]))


def lay_entry(r, mem, at, window, own):
    mem.ensure(at, 0x28)
    mem.put8(at + 2, r.choice([2, 2, 0, 1]))
    mem.put(at + 8, 'I', r.choice(ENTRY_TYPES) if r.random() < 0.95 else r.getrandbits(32))
    mem.put8(at + 0xc, r.choice(BITS))
    mem.put(at + 0x10, 'H', r.choice([r.choice(window), r.choice(window), own, 0]))
    mem.put8(at + 0x22, r.choice(BITS))


def world(r):
    """(regions, the cell's address): the world about one cell, the things, definitions, money and flag globals."""
    x = r.choice([0, 1, 0x7e, 0x7f, r.randrange(0x80), r.randrange(0x80)])
    y = r.choice([0, 1, 0x7f, r.randrange(0x80), r.randrange(0x80)])
    ident = ((y << 7) + x + 1) & 0xffff
    window = [(ident + dy * 128 + dx) & 0xffff for dy in (-1, 0, 1) for dx in (-1, 0, 1)]
    own = r.choice([r.getrandbits(16), r.choice(window)])
    mem = Sparse(r)
    for k in window:
        lay_cell(r, mem, k, window, own)
    # the cells' 0x28-byte entries (MapHandle_Entry28: W + 0x1102b0 + id * 0x28) and the parents' (W + 0x10ffb0 + id * 0x28)
    for k in window:
        lay_entry(r, mem, W + 0x1102b0 + k * 0x28, window, own)
        lay_entry(r, mem, W + 0x10ffb0 + k * 0x28, window, own)
    lay_entry(r, mem, W + 0x10ffb0 + own * 0x28, window, own)
    lay_entry(r, mem, W + 0x10ffb0, window, own)   # (parent 0)
    mem.put(W + 0x1da726, 'H', 14)
    regions = dict(mem.parts)
    # the things: kind 3 mostly, chained downwards (no cycles), their definitions' ids
    slots = bytearray(r.bytes(20 * NSLOT))
    things = bytearray(r.bytes(0x20 * NSLOT))
    ids = [r.choice([1, 2, 3, 7, 0x100]) for _ in range(4)]
    for h in range(14):
        put(slots, 20 * h, 'I', THINGS + 0x20 * h if r.random() < 0.98 else 0)
        t = 0x20 * h
        things[t + 2] = r.choice([3, 3, 3, 1, 2, 0xff])
        put(things, t + 0xa, 'H', r.choice([0, r.randrange(h)]) if h else 0)
        put(things, t + 0xe, 'H', r.choice(ids) if r.random() < 0.95 else 0x55)
    put(slots, 20 * 14, 'I', MONEY)
    put(slots, 20 * 15, 'I', 0)
    cost = r.choice([0, 0, 1, 100, 5000, 0x7fffffff, r.getrandbits(32)])
    money = put(bytearray(r.bytes(0x10)), 0xc, 'i', r.choice([0, 100, 4999, 5000, 5001, -1, cost & 0x7fffffff, r.getrandbits(31)]))
    # the definitions: four (the id 0x55 none), the find cache hit or not
    defs = bytearray(0x738)
    blocks = bytearray(r.bytes(0x60 * 4))
    level = r.choice([0, 1, 5, -1, 100])
    for k in range(4):
        put(defs, 4 * k, 'I', DEFS + 0x60 * k)
        put(blocks, 0x60 * k, 'H', ids[k])
        put(blocks, 0x60 * k + 0x54, 'i', level + r.choice([-1, 0, 1, 2]))
    put(defs, 0x528, 'I', 4)
    c = r.randrange(5)
    put(defs, 0x730, 'HHI', ids[c] if c < 4 else 0xfffe, 0, DEFS + 0x60 * (c % 4))
    pairs = bytearray(0x40)
    for k, (dx, dy) in enumerate(PAIRS):
        put(pairs, 8 * k, 'ii', *((dx, dy) if r.random() < 0.95 else (r.randrange(-1, 2), r.randrange(-1, 2))))
    flag = lambda: r.choice([0, 0, 1, 1, 0, 1, 2])  # noqa: E731
    globals_ = {
        0x816d48: struct.pack('<7I', *[flag() for _ in range(7)]),
        0x818c10: struct.pack('<2i', level + r.choice([-1, 0, 1, 2]), r.choice([level, -5, 50])),
        0x818688: put(put(put(put(bytearray(r.bytes(0x50)), 0, 'I', flag()), 0x10, 'H', own if r.random() < 0.7 else r.getrandbits(16)),
                          0x38, 'I', SEEN), 0x48, 'IB', flag(), r.choice([0, 0, 1])),
        SEEN: struct.pack('<H', own if r.random() < 0.3 else r.choice(window)),
        0x8187f8: struct.pack('<I', flag()),
        0x820aa4: put(put(bytearray(r.bytes(0x18)), 0, 'I', flag()), 0x14, 'I', flag()),
        0x785a2c: struct.pack('<I', r.choice([0, 0, 0, 1])),
        0x81af4c: struct.pack('<I', cost),
        0x81b6dc: struct.pack('<I', r.choice([0, 10, 5000, r.getrandbits(32)])),
        0x81ea14: struct.pack('<I', r.choice([0, 10, 5000, r.getrandbits(32)])),
        0x81aebc: struct.pack('<I', r.choice([0, 10, 5000, r.getrandbits(32)])),
        0x81b43c: struct.pack('<I', r.choice([0, 10, 5000, r.getrandbits(32)])),
        0x81b4cc: struct.pack('<I', r.choice([0, 0, 1])),
        0x7cdba0: pairs,
        0x7cf83c: struct.pack('<I', W),
        0x80239c: struct.pack('<II', W, W + 0x2d8),
        0x7cfb90: slots, THINGS: things, MONEY: money, 0x7890a0: defs, DEFS: blocks,
    }
    regions.update(globals_)
    return regions, cell44(ident)


KINDS = [0x1f, 0x3a, 4, 9, 0xa, 0x18, 0x36, 1, 1, 3, 3, 0xb, 0x10, 0x15, 0x15, 0x16, 0x14, 0, 2, 0x19, 7]


def can_build(r):
    regions, cell = world(r)
    kind = r.choice(KINDS) if r.random() < 0.95 else r.randrange(0x40)
    return Call(0x535670, [cell, kind, r.choice([0, 1, 1, 2])], regions, conv='t', ret='a')


CASES = {'CanBuild': can_build}
OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]


def case(r):
    return CASES[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)

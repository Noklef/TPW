# The track circuit walked (src/game/world/trackcircuit: TrackCircuit_Walk 0x52a970) against the original. The world ([0x7cf83c] and
# [0x80239c] the same; [0x8023a0] its 0x44-byte cells) has the cells within eight of the walk's start (the station's cell 0x7639e0 /
# 0x7639e4 moved by the angle 0x7639ac: 0, 0x5a, 0xb4, 0x10e or another) laid out, both the 0x28-byte cells (own id first, types
# 0xb / 0x10 mostly, then 0xd, 0x12, 0x19, 0xc / 0x11 with a parent, others; a thing (+4) or none; flag 0x10 now and then; neighbour bits
# (+0xc, and +0x22 for the hoardings, 0x81b4cc and +2 of 2) and directions (+0xd) random) and the 0x44-byte ones (types 4, 9, 0xa or
# others, a parent, the first of a chain of things). Most cases lay a closed loop (a rectangle two cells a step through the start,
# now and then a bit flipped or a junction's third side), the direction byte 0x818c50 along it or not. A walk that would not stop
# (a state seen again) or would leave the cells within six of the start, or would step in no direction at three neighbours (a null
# cell read: a crash on both sides), is redrawn by a model of it. The things (slots 0x7cfb90, 12 handles, each chain's next a lower
# handle) have definitions (0x7890a0, the cache now and then hit) and their thing's model (0x7a4610, parts keyed 1 / 0x10 with mask 1
# or not, faces in either table). The entrance (0x7855bc / 0x7855c0 / 0x7855c4, the map's +0x1da732) on the walk's row now and then.
# Agent B's TrakRide_Strip and TrakRide_AddSection are cdecl recorders 28 of 1 and 29 of 7, MapCell_ApplyModel the thiscall recorder 24
# of none (driver.cpp's kTrackCircuitEnv). check 0, 1, 0x100 or any.
# usage: trackcircuit.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

B = Bench([(0x52a970, 0x52b410), (0x4d0af0, 0x4d0b40), (0x4d96f0, 0x4d9a70), (0x53ad30, 0x53adc0), (0x53ae00, 0x53aed0), (0x53aef0, 0x53af10),
           (0x535db0, 0x535de0), (0x527d60, 0x527e90), (0x4dd4e0, 0x4dd510), (0x4e2290, 0x4e2410), (0x522700, 0x5228c0), (0x50c320, 0x50c5f0),
           (0x4d8c00, 0x4d8c60), (0x44b3c0, 0x44b420), (0x4d8300, 0x4d8340), (0x412e90, 0x412f00), (0x5da3c0, 0x5da3c1)],
          hooks={0x545450: _recorder(28, 1, cdecl=True), 0x54b2f0: _recorder(29, 7, cdecl=True), 0x53afa0: _recorder(24, 0)})
WORLD = RAM + 0x1000000
CELLS28, CELLS44 = WORLD + 0x1102b0, WORLD + 0x294
SLOTS, THINGS, DEFS, DEFITEMS, MODELS = 0x7cfb90, RAM, 0x7890a0, RAM + 0x1000, RAM + 0x2000
NH, THING = 12, 0x40
WALK, LAID = 6, 8
STEPS = {1: (0, -1), 2: (1, -1), 4: (1, 0), 8: (1, 1), 0x10: (0, 1), 0x20: (-1, 1), 0x40: (-1, 0), 0x80: (-1, -1)}
TABLE = {1: 0, 0x10: 8, 0x20: 0x10, 8: 0x18, 0x80: 0x20, 4: 0x28, 0x40: 0x30, 2: 0x38}
PAIRS = bytearray(0x40)
for _d, _at in TABLE.items():
    struct.pack_into('<ii', PAIRS, _at, *STEPS[_d])
MOVES = {1: (0, -2), 4: (2, 0), 0x10: (0, 2), 0x40: (-2, 0)}
REVERSE = {1: 0x10, 0x10: 1, 4: 0x40, 0x40: 4}
BD = (0xb, 0xd, 0x10, 0x12, 0x19)
ANGLE = {0: (-4, 0), 0x5a: (0, 4), 0xb4: (4, 0), 0x10e: (0, -4)}


def ident(x, y):
    return ((y << 7) + x + 1) & 0xffff


def nib(d):
    return (d << 4 if d < 0x10 else d >> 4) & 0xff if d else 0


def count(v, mask=0xff):
    return bin(v & mask).count('1')


def u32(cell, at):
    return struct.unpack_from('<I', cell, at)[0]


def walk(cells, x0, y0, d, hoarding):
    """The walk modelled: True when it ends within the cells within WALK of the start, never stepping in no direction at three neighbours."""
    def nbs(cell):
        return cell[0x22] if hoarding and cell[2] == 2 else cell[0xc]

    x, y, seen = x0, y0, set()
    while True:
        if abs(x - x0) > WALK or abs(y - y0) > WALK or (x, y, d) in seen:
            return False
        seen.add((x, y, d))
        cell = cells[ident(x, y)]
        nb = nbs(cell)
        if u32(cell, 8) not in BD:
            d = 0
        elif count(nb, 0x55) == 2 and nb & 0x11 and nb & 0x44:
            t = nib(d)
            d = nb ^ (t if t & nb else d)
        fail = d == 0 or count(nb) < 2
        if count(nb) == 3:
            if d not in STEPS:
                return False
            dx, dy = STEPS[d]
            nxt = cells[ident(x + 2 * dx, y + 2 * dy)]
            if u32(nxt, 8) in BD and d & (1 if nbs(nxt) == 0 else 0) or u32(nxt, 8) == 0:
                fail = True
        mx, my = MOVES.get(d, (0, 0))
        x, y = x + mx, y + my
        if (x, y) == (x0, y0) or fail:
            return True


def side_bits(r):
    sides = [1, 4, 0x10, 0x40]
    return r.choice([r.choice(sides) | r.choice(sides), r.choice(sides), r.getrandbits(8), 0x11, 0x44, 0x14, 0x41, 0x50, 5])


def lay_loop(r, cells, x0, y0, flag_rate):
    """A rectangle of track two cells a step with the start on it; returns the direction along it at the start."""
    w, h = r.randrange(1, 4), r.randrange(1, 4)
    corners = [(0, 0), (w, 0), (w, h), (0, h)]
    ring = []   # (the rectangle's cells in steps, round from its top left)
    for (ax, ay), (bx, by) in zip(corners, corners[1:] + corners[:1]):
        dx, dy = (bx > ax) - (bx < ax), (by > ay) - (by < ay)
        x, y = ax, ay
        while (x, y) != (bx, by):
            ring.append((x, y))
            x, y = x + dx, y + dy
    if r.random() < 0.5:   # (the other way round)
        ring.reverse()
    px, py = r.choice(ring)
    ring = [(x0 + 2 * (x - px), y0 + 2 * (y - py)) for x, y in ring]
    way = {(2, 0): 4, (-2, 0): 0x40, (0, 2): 0x10, (0, -2): 1}
    path = [(x, y, way[(ring[(i + 1) % len(ring)][0] - x, ring[(i + 1) % len(ring)][1] - y)]) for i, (x, y) in enumerate(ring)]
    out = {}
    for i, (x, y, d) in enumerate(path):
        back = REVERSE[path[i - 1][2]]
        nb = back | d
        if r.random() < 0.06:
            nb |= r.choice([1, 4, 0x10, 0x40, 2, 8])
        if r.random() < 0.03:
            nb ^= 1 << r.randrange(8)
        cell = cells[ident(x, y)]
        cell[0xc] = cell[0x22] = nb
        put(cell, 8, 'I', r.choice([0xb, 0x10, 0xb, 0x10, 0xd, 0x12, 0x19]))
        put(cell, 0xe, 'H', (cell[0xe] | cell[0xf] << 8) & ~0x10 | (0x10 if r.random() < flag_rate else 0))
        out[(x, y)] = d
    return out[(x0, y0)]


def model(r, regions, base):
    """A thing's model: parts keyed 1 / 0x10 (mask 1 or not) among others, faces in either table."""
    n = r.randrange(0, 6)
    first, start = r.randrange(0, 5), r.randrange(0, 3)
    holder, p, m = base, base + 0x10, base + 0x20
    parts, faces_a, faces_b = base + 0x100, base + 0x200, base + 0x800
    hold = bytearray(r.bytes(0x10))
    put(hold, 8, 'I', p)
    regions[holder] = hold
    regions[p] = put(bytearray(r.bytes(8)), 4, 'I', m)
    mod = bytearray(r.bytes(0x80))
    put(mod, 0x44, 'HHH', first, start, n)
    put(mod, 0x70, 'I', faces_a)
    put(mod, 0x74, 'I', faces_b)
    put(mod, 0x7c, 'I', parts)
    regions[m] = mod
    table = bytearray(r.bytes(0x14 * max(n, 1)))
    for i in range(n):
        put(table, 0x14 * i, 'I', r.getrandbits(32) | 1 if r.random() < 0.8 else r.getrandbits(32) & ~1)
        put(table, 0x14 * i + 8, 'I', r.choice([1, 0x10, 1, 0x10, 2, 0, r.getrandbits(32)]))
    regions[parts] = table
    regions[faces_a] = r.bytes(0xa0 * max(first, 1))
    regions[faces_b] = r.bytes(0x58 * max(start + n - first, 1))


def case(r):
    hoarding = r.random() < 0.3
    angle = r.choice([0, 0x5a, 0xb4, 0x10e, 0, 0x5a, 0xb4, 0x10e, 0x2d, r.getrandbits(32)])
    while True:
        regions = {}
        x0, y0 = r.randrange(LAID + 2, 126 - LAID), r.randrange(LAID + 2, 125 - LAID)
        cells, ids = {}, []
        for y in range(y0 - LAID, y0 + LAID + 1):
            row = bytearray(r.bytes((2 * LAID + 1) * 0x28))
            rows44 = bytearray(r.bytes((2 * LAID + 1) * 0x44))
            for i in range(2 * LAID + 1):
                own = ident(x0 - LAID + i, y)
                cells[own] = memoryview(row)[i * 0x28:(i + 1) * 0x28]
                ids.append(own)
            regions[CELLS28 + ident(x0 - LAID, y) * 0x28] = row
            regions[CELLS44 + ident(x0 - LAID, y) * 0x44] = rows44
        for own, cell in cells.items():
            put(cell, 0, 'H', own)
            put(cell, 4, 'I', r.choice([0, 0, r.getrandbits(32)]))
            put(cell, 8, 'I', r.choice([0xb, 0x10] * 4 + [0xd, 0x12, 0x19, 0xc, 0x11, 0, 1, 4, 9]))
            cell[2] = r.choice([2, 2, 0, 1])
            cell[0xc], cell[0x22] = side_bits(r), side_bits(r)
            cell[0xd] = r.choice([1, 4, 0x10, 0x40, 2, 8, 0x20, 0x80, 0, r.getrandbits(8)])
            put(cell, 0xe, 'H', r.getrandbits(16) & ~0x10 | (0x10 if r.random() < 0.3 else 0))
            put(cell, 0x10, 'H', r.choice(ids))
        flag_rate = r.choice([0, 0.3, 0.6, 1])
        d = lay_loop(r, cells, x0, y0, flag_rate) if r.random() < 0.85 else 0
        if not d or r.random() < 0.1:
            d = r.choice([1, 4, 0x10, 0x40, 1, 4, 0x10, 0x40, 0, 2, r.getrandbits(8)])
        if walk(cells, x0, y0, d, hoarding):
            break
    # (the 0x44-byte cells: types, parents, chains)
    for y in range(y0 - LAID, y0 + LAID + 1):
        row = regions[CELLS44 + ident(x0 - LAID, y) * 0x44]
        for i in range(2 * LAID + 1):
            c = 0x44 * i
            put(row, c + 8, 'I', r.choice([4, 9, 0xa, 4, 9, 0xa, 0, 1, 0xb]))
            put(row, c + 0x10, 'H', ident(x0 - LAID + i, y) if r.random() < 0.7 else r.choice(ids))
            put(row, c + 0x24, 'H', r.choice([0, r.randrange(1, NH), r.randrange(1, NH)]))
    sdx, sdy = ANGLE.get(angle, (0, 0))
    regions[0x7639e0] = struct.pack('<ii', x0 - sdx, y0 - sdy)
    regions[0x7639ac] = struct.pack('<I', angle)
    regions[0x818c50] = bytes([d]) + r.bytes(3)
    regions[0x81b4cc] = struct.pack('<I', r.choice([1, 0xff, 0x101]) if hoarding else r.choice([0, 0x100]))
    regions[0x7cf83c] = struct.pack('<I', WORLD)
    regions[0x80239c] = struct.pack('<I', WORLD)
    regions[0x8023a0] = struct.pack('<I', WORLD + 0x2d8)
    regions[0x7cdba0] = PAIRS
    regions[LOG] = log_region()
    # (the things and their definitions)
    ndefs = r.randrange(1, 4)
    def_ids = r.sample(range(1, 0x10000), ndefs)
    slots = bytearray(r.bytes(20 * NH))
    things = bytearray(r.bytes(THING * NH))
    for h in range(NH):
        put(slots, 20 * h, 'I', THINGS + THING * h)
        t = THING * h
        things[t + 2] = r.choice([3, 3, 1, 0])
        put(things, t + 0xa, 'H', r.randrange(0, h) if h and r.random() < 0.5 else 0)
        put(things, t + 0xe, 'H', r.choice(def_ids))
        put(things, t + 0x20, 'I', r.randrange(0, 4))
        put(things, t + 0x34, 'H', r.choice(ids))
    regions[SLOTS] = slots
    regions[THINGS] = things
    dtab = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(0x20 * ndefs))
    for i in range(ndefs):
        put(dtab, 4 * i, 'I', DEFITEMS + 0x20 * i)
        put(items, 0x20 * i, 'H', def_ids[i])
    put(dtab, 0x528, 'i', ndefs)
    k = r.randrange(ndefs)
    put(dtab, 0x730, 'HxxI', def_ids[k] if r.random() < 0.3 else 0, DEFITEMS + 0x20 * k)
    regions[DEFS] = dtab
    regions[DEFITEMS] = items
    holders = bytearray(0x10)
    for t in range(4):
        put(holders, 4 * t, 'I', MODELS + 0x1000 * t)
        model(r, regions, MODELS + 0x1000 * t)
    regions[0x7a4610] = holders
    # (the entrance: on the walk's row now and then)
    ent = bytearray(r.bytes(0xc))
    if r.random() < 0.3:
        put(ent, 0, 'iii', x0 - r.randrange(0, 4), y0 + r.choice([0, 2, -2]), x0 + r.randrange(-2, 4))
    regions[0x7855bc] = ent
    regions[WORLD + 0x1da730] = struct.pack('<HH', r.getrandbits(16), r.choice([0, r.randrange(1, NH)]))
    check = r.choice([0, 0, 0, 1, 0x100, r.getrandbits(32)])
    return Call(0x52a970, [check], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

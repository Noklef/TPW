# Track runs from a thing (src/game/world/cellrun: TraceA 0x530830, TraceB 0x530b70) against the original: the thing's cell (+5 column, +7 row) and
# direction (+0x10: 0, 90, 180 or 270; others give the step's code as the address of a stack slot, not the same on both sides) put the start cell
# anywhere on the map, mostly away from the edges; the cells around it two apart are laid out (each row one region, the cells between random too),
# of types 0xb / 0x10 mostly, some 0xd / 0x12 (the ends), 0xc / 0x11 (their parent at +0x10: mostly one shared id of a laid out cell) and others.
# Most cases lay a track (lay_track) from the start with matching neighbour bits (both kinds: 0x81b4cc and a cell's +2 of 2 pick +0x22), ending
# at an end cell of the same +0x10 that has the track's side, or at a dead end. TraceB's cell 0x7639e0 / 0x7639e4 is in the map or not, now and then
# on the track. A run that would not stop (or leaves the laid out cells) is skipped by a model of the walk, so a case is redrawn; the direction pairs
# (0x7cdba0, set at run time) are laid out. The point list (0x81b740) starts random; its cap (0x401 points) is not reached.
# usage: trackrun.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=A,B)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x530830, 0x530ac2), (0x530b70, 0x530e27), (0x52a3a0, 0x52a4b0), (0x4d8c00, 0x4d8c30), (0x4d8300, 0x4d8328), (0x522700, 0x5228b5),
           (0x53a510, 0x53a53d), (0x53ad20, 0x53aefc), (0x50c320, 0x50c3e0), (0x50c3e0, 0x50c540), (0x536300, 0x53644c), (0x522850, 0x522860),
           (0x4d99c0, 0x4d99e0), (0x4d97b0, 0x4d97e0), (0x5da3c0, 0x5da3c1)])
CELLS = RAM + 0x10000
WORLD = (CELLS - 0x6cde * 0x28) & 0xffffffff
THING = RAM
REACH = 4
STEPS = {1: (0, -1), 2: (1, -1), 4: (1, 0), 8: (1, 1), 0x10: (0, 1), 0x20: (-1, 1), 0x40: (-1, 0), 0x80: (-1, -1)}
TABLE = {1: 0, 0x10: 8, 0x20: 0x10, 8: 0x18, 0x80: 0x20, 4: 0x28, 0x40: 0x30, 2: 0x38}
PAIRS = bytearray(0x40)
for _d, _at in TABLE.items():
    struct.pack_into('<ii', PAIRS, _at, *STEPS[_d])
MOVES = {1: (0, -2), 4: (2, 0), 0x10: (0, 2), 0x40: (-2, 0)}
REVERSE = {1: 0x10, 0x10: 1, 4: 0x40, 0x40: 4}
# Position_StepA / StepB: direction -> (dx, dy, code)
STEP_A = {0: (4, 1, 4), 90: (1, -5, 1), 180: (-5, -2, 0x40), 270: (-2, 4, 0x10)}
STEP_B = {0: (-2, 1, 0x40), 90: (1, 1, 0x10), 180: (1, -2, 4), 270: (-2, -2, 1)}
OPS = [k for k in os.environ.get('OPS', 'A,B').split(',') if k]
TRACK, ENDS, PARENTS = (0xb, 0x10), (0xd, 0x12), (0xc, 0x11)


def ident(x, y):
    return ((y << 7) + x + 1) & 0xffff


def u32(cell, at):
    return struct.unpack_from('<I', cell, at)[0]


def u16(cell, at):
    return struct.unpack_from('<H', cell, at)[0]


def bits(v, mask):
    return bin(v & mask).count('1')


def nib(d):
    return (d << 4 if d < 0x10 else d >> 4) & 0xff if d else 0


def corner(nb):
    return bits(nb, 0x55) == 2 and nb & 0x11 and nb & 0x44


def stops(cells, x, y, d, hoarding, b, done):
    """The walk modelled: True when it stops within the laid out cells."""
    def nbs(cell):
        return cell[0x22] if hoarding and cell[2] == 2 else cell[0xc]

    def dor12(cell):
        if u32(cell, 8) in PARENTS:
            parent = cells.get(u16(cell, 0x10))
            if parent is None:
                return None
            if u32(parent, 8) in ENDS:
                return True
        return u32(cell, 8) in ENDS

    for _ in range(200):
        cell = cells.get(ident(x, y))
        if cell is None:
            return False
        if u32(cell, 8) in TRACK:
            nb = nbs(cell)
            if corner(nb):
                d = nib(d) ^ nb
            elif not nb & d:
                done = True
            if d in STEPS:
                own = u16(cell, 0)
                index = (own - 1) & 0xffffffff
                dx, dy = STEPS[d]
                if 0 <= (index & 0x7f) + dx * 2 < 0x80 and 0 <= (index >> 7) + dy * 2 < 0x80:
                    nxt = cells.get((((dy * 2) << 7) + own + dx * 2) & 0xffff)
                    if nxt is None:
                        return False
                    match = dor12(nxt)
                    if match is None:
                        return False
                    if match and u16(nxt, 0x10) == u16(cell, 0x10) and nib(d) & nbs(nxt):
                        done = True
        if done:   # (A's last move on reaching an end reads nothing more)
            return True
        mx, my = MOVES.get(d, (0, 0))
        x, y = x + mx, y + my
    return False


def side_bits(r):
    sides = [1, 4, 0x10, 0x40]
    return r.choice([r.choice(sides), r.choice(sides) | r.choice(sides), r.choice(sides) | r.choice(sides), r.getrandbits(8), 0])


def lay_track(r, cells, x0, y0, d, parent):
    """A track from the start in direction d (or a corner into it), two apart, turning now and then without crossing, its cells of type 0xb / 0x10
    with their two sides as neighbour bits and +0x10 the shared parent, ending at an end cell (0xd / 0x12, or 0xc / 0x11 with the parent) that has
    the track's side, or at a dead end; now and then a bit flipped."""
    i, j = 0, 0
    first = d if r.random() < 0.7 else r.choice([k for k in MOVES if k not in (d, REVERSE[d])])
    path = [(0, 0, REVERSE[d], first)]   # (straight on, or a corner: Nibble_Scale of d is its reverse, so the turn gives `first`)
    seen = {(0, 0)}
    d = first
    for _ in range(r.randrange(0, 12)):
        mx, my = MOVES[d]
        i, j = i + mx // 2, j + my // 2
        if abs(i) > REACH - 1 or abs(j) > REACH - 1 or (i, j) in seen:
            i, j = i - mx // 2, j - my // 2
            break
        seen.add((i, j))
        nd = d if r.random() < 0.6 else r.choice([k for k in MOVES if k not in (d, REVERSE[d])])
        path.append((i, j, REVERSE[d], nd))
        d = nd
    last = len(path) - 1
    dead = r.random() < 0.25
    for k, (i, j, back, fwd) in enumerate(path):
        cell = cells.get(ident(x0 + 2 * i, y0 + 2 * j))
        if cell is None:
            return
        nb = back if k == last and dead else back | fwd
        if r.random() < 0.04:
            nb ^= 1 << r.randrange(8)
        cell[0xc] = cell[0x22] = nb
        struct.pack_into('<I', cell, 8, r.choice(TRACK))
        struct.pack_into('<H', cell, 0, ident(x0 + 2 * i, y0 + 2 * j))
        struct.pack_into('<H', cell, 0x10, parent)
    i, j, _, fwd = path[last]
    mx, my = MOVES[fwd]
    end = cells.get(ident(x0 + 2 * i + mx, y0 + 2 * j + my))
    if end is not None and not dead:
        struct.pack_into('<I', end, 8, r.choice(ENDS + PARENTS + ENDS))
        struct.pack_into('<H', end, 0x10, parent)
        end[0xc] = end[0x22] = REVERSE[fwd] | (r.getrandbits(8) if r.random() < 0.3 else 0)


def draw(r, b):
    angle = r.choice([0, 90, 180, 270])
    sx, sy, d = (STEP_B if b else STEP_A)[angle]
    while True:
        x0 = r.choice([r.randrange(0, 128), r.randrange(8, 120), r.randrange(8, 120), r.choice([0, 1, 126, 127])])
        y0 = r.choice([r.randrange(0, 128), r.randrange(8, 120), r.randrange(8, 120), r.choice([0, 1, 126, 127])])
        if 0 <= x0 - sx < 0x80 and 0 <= y0 - sy < 0x100:
            break
    hoarding = r.choice([0, 0, 1])
    rows, cells = {}, {}
    ids = []
    for j in range(-2 * REACH, 2 * REACH + 1):   # (the rows between too: a run that moves by 1 lands on them)
        y = y0 + j
        lo = ident(x0 - 2 * REACH, y)
        row = bytearray(r.bytes((4 * REACH + 1) * 0x28))
        for i in range(4 * REACH + 1):
            own = (lo + i) & 0xffff
            if lo + i < 0x10000:
                cells[own] = memoryview(row)[i * 0x28:(i + 1) * 0x28]
                ids.append(own)
        rows[CELLS + lo * 0x28] = row
    parent = r.choice(ids)
    for own, cell in cells.items():
        struct.pack_into('<H', cell, 0, own if r.random() < 0.95 else (own + r.choice([2, -2, 0x100, 1])) & 0xffff)
        struct.pack_into('<I', cell, 8, r.choice(TRACK * 6 + ENDS * 2 + PARENTS * 2 + (0, 1, 0x19, 0x1b, 0x10b)))
        cell[2] = r.choice([2, 2, 0, 1, 3])
        cell[0xc] = side_bits(r)
        cell[0x22] = side_bits(r)
        struct.pack_into('<H', cell, 0x10, parent if r.random() < 0.8 else r.choice(ids))
    if r.random() < 0.75:
        lay_track(r, cells, x0, y0, d, parent)
    thing = put(put(bytearray(r.bytes(0x20)), 0x10, 'I', angle), 5, 'B', x0 - sx)
    thing[7] = y0 - sy
    return x0, y0, d, hoarding, rows, cells, thing


def case(r):
    op = r.deal(OPS)
    b = op == 'B'
    while True:
        x0, y0, d, hoarding, rows, cells, thing = draw(r, b)
        start = cells.get(ident(x0, y0))
        done = b and start is not None and u32(start, 8) not in (0xb, 0xd, 0x10, 0x12, 0x19)
        if stops(cells, x0, y0, d, hoarding, b, done):
            break
    regions = dict(rows)
    regions[THING] = thing
    regions[0x818c50] = r.bytes(4)
    regions[0x81b4cc] = struct.pack('<I', r.choice([1, 0xff, 0x101]) if hoarding else r.choice([0, 0x100]))
    regions[0x7cf83c] = struct.pack('<I', WORLD)
    regions[0x80239c] = struct.pack('<I', WORLD)
    regions[0x7cdba0] = PAIRS
    regions[0x81b740] = r.bytes(0x800)
    regions[0x820a8c] = r.bytes(4)
    regions[0x81ede4] = r.bytes(8)
    if b:
        on = r.random() < 0.4
        sx = x0 + 2 * r.randrange(-2, 3) if on else r.choice([r.randrange(0, 128), -1, 128, r.getrandbits(32) - (1 << 31)])
        sy = y0 + 2 * r.randrange(-2, 3) if on else r.choice([r.randrange(0, 128), -1, 128])
        regions[0x7639e0] = struct.pack('<ii', sx, sy)
        return Call(0x530b70, [THING], regions, conv='c', ret='v')
    return Call(0x530830, [THING], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)

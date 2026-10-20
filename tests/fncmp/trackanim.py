# Animated track runs from a thing (src/game/world/cellrun: AnimateA 0x52a490, AnimateB 0x52a700) against the original: trackrun.py's map (the
# thing's cell and direction put the start cell anywhere, mostly away from the edges; the cells two apart around it laid out, a track laid over
# them mostly; the hoarding neighbours, the direction pairs 0x7cdba0) with the run's cell types 0xb, 0xd, 0x10, 0x12 and 0x19 (and others), and
# now and then a direction that is not a right angle (the start code then start's low byte, B's through Nibble_Scale). A cell's thing (+4) is 0
# mostly, else 1: entry 1 of 0x7a4610 is animator.py's animator, which plays bank 5 or 0xc (start's low byte). A run that would not stop (or
# leaves the laid out cells) is skipped by a model of the walk, so a case is redrawn.
# usage: trackanim.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=A,B)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import animator  # noqa: E402
import trackrun  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402
from trackrun import MOVES, PAIRS, STEP_A, STEP_B, STEPS, corner, ident, nib, side_bits, u16, u32  # noqa: E402


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged([(0x52a490, 0x52a6a4), (0x52a700, 0x52a91c), (0x4d0b10, 0x4d0b27), (0x4d8c00, 0x4d8c30), (0x4d8300, 0x4d8328), (0x522700, 0x5228b5),
                  (0x53a510, 0x53a53d), (0x53ad20, 0x53aefc), (0x50c320, 0x50c3e0), (0x50c3e0, 0x50c540), (0x536300, 0x53644c), (0x4d99c0, 0x4d99e0),
                  (0x4d97b0, 0x4d97e0), (0x5da3c0, 0x5da3c1),
                  # (animator.py's, for Animator_Play)
                  (0x46f150, 0x475010), (0x67a400, 0x690000), (0x402d70, 0x402e00), (0x402e60, 0x4030f0), (0x403150, 0x403172), (0x4031e0, 0x403202),
                  (0x4033a0, 0x4033f0), (0x5f5f10, 0x5f5fc0), (0x429df0, 0x429e17)]), sines=256)
CELLS = RAM + 0x10000
WORLD = (CELLS - 0x6cde * 0x28) & 0xffffffff
THING = RAM + 0xf000
REACH = trackrun.REACH
RUN, ENDS, PARENTS = (0xb, 0xd, 0x10, 0x12, 0x19), (0xd, 0x12), (0xc, 0x11)
OPS = [k for k in os.environ.get('OPS', 'A,B').split(',') if k]


def stops(cells, x, y, d, hoarding):
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
        nb = nbs(cell)
        if u32(cell, 8) in RUN:
            if corner(nb):
                turn = nib(d)
                d = nb ^ (turn if turn & nb else d)
        else:
            d = 0
        done = not corner(nb) and not d & nb
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
        if done:   # (the move after reads nothing)
            return True
        mx, my = MOVES.get(d, (0, 0))
        x, y = x + mx, y + my
    return False


def draw(r, b, start):
    angle = r.choice([0, 90, 180, 270] * 4 + [1, 45, 360, -90, r.getrandbits(32)])
    steps = STEP_B if b else STEP_A
    if angle in steps:
        sx, sy, d = steps[angle]
    else:
        sx, sy, d = 0, 0, nib(start & 0xff) if b else start & 0xff
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
    p_thing = r.choice([0, 0.2, 0.5])
    for own, cell in cells.items():
        struct.pack_into('<H', cell, 0, own if r.random() < 0.95 else (own + r.choice([2, -2, 0x100, 1])) & 0xffff if r.random() < 0.6 else 0)
        struct.pack_into('<I', cell, 8, r.choice(RUN * 4 + ENDS + PARENTS * 2 + (0, 1, 0x1b, 0x10b)))
        cell[2] = r.choice([2, 2, 0, 1, 3])
        cell[0xc] = side_bits(r)
        cell[0x22] = side_bits(r)
        struct.pack_into('<H', cell, 0x10, parent if r.random() < 0.8 else r.choice(ids))
    if r.random() < 0.75 and d in MOVES:
        trackrun.lay_track(r, cells, x0, y0, d, parent)
    for cell in cells.values():   # (after the track: its cells keep theirs)
        struct.pack_into('<I', cell, 4, 1 if r.random() < p_thing else 0)
    thing = put(put(bytearray(r.bytes(0x20)), 0x10, 'I', angle & 0xffffffff), 5, 'B', x0 - sx)
    thing[7] = y0 - sy
    return x0, y0, d, hoarding, rows, cells, thing


def case(r):
    op = r.deal(OPS)
    b = op == 'B'
    start = r.choice([0, 1, 1, 0x100, 0xffffff01, 4, 0x10, 0x40, 0x41, r.getrandbits(32)])
    while True:
        x0, y0, d, hoarding, rows, cells, thing = draw(r, b, start)
        if stops(cells, x0, y0, d, hoarding):
            break
    regions, nlayers, blocks = animator.world(r)
    regions.update(rows)
    regions[THING] = thing
    regions[0x81b4cc] = struct.pack('<I', r.choice([1, 0xff, 0x101]) if hoarding else r.choice([0, 0x100]))
    regions[0x7cf83c] = struct.pack('<I', WORLD)
    regions[0x80239c] = struct.pack('<I', WORLD)
    regions[0x8023a0] = r.bytes(4)
    regions[0x7cdba0] = PAIRS
    regions[0x7a4610] = struct.pack('<II', r.getrandbits(32), animator.ANIM)
    return Call(0x52a700 if b else 0x52a490, [THING, start], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

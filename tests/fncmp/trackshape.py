# The track's piece shapes along a run from a thing (src/game/world/cellrun: UpdateA 0x52c6f0, UpdateB 0x52ccb0) against the original: trackanim.py's
# map and thing (the start anywhere, mostly away from the edges; a track laid from it mostly, its cells two apart, the hoarding neighbours, the
# direction pairs 0x7cdba0; a direction that is not a right angle now and then), the cells' low flags (+0xe) clear now and then (no shape work).
# start (the default direction and the animate flag) as trackanim's; flash and bank5 0, 1, 0x100 or any. A cell's thing (+4) is 0 mostly, else 1:
# entry 1 of 0x7a4610 is animator.py's animator (its flag 0x4000000 set when flash's low byte is, so it never flashes), or, when start's low byte
# is clear (no animation), now and then 2: a holder that flashes (Highlight_Flash: its model's +0x5c not its base's, 0-3 meshes) or stops
# (Highlight_Unflash: listed at 0x7afe14 first or second; no 0x20000, the model without 0x100000; the meshes' blocks copied or not). Agent B's
# unported 0x53afa0 is thiscall recorder 24 (driver.cpp's kCellRunEnv). A run that would not stop (or leaves the laid out cells) is skipped by a
# model of the walk, so a case is redrawn, as is one that would shape a cell from cells two on that are not laid out (or off the map: null).
# usage: trackshape.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=A,B)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import animator  # noqa: E402
import trackanim  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402
from trackrun import MOVES, PAIRS, STEPS, corner, ident, nib, u16, u32  # noqa: E402

B = Bench(trackanim.merged([(0x52c6f0, 0x52d1e8), (0x4d0b10, 0x4d0b27), (0x4d8c00, 0x4d8c30), (0x4d8300, 0x4d8328), (0x522700, 0x5228b5),
                            (0x53a510, 0x53a53d), (0x53ad20, 0x53aefc), (0x50c320, 0x50c540), (0x536300, 0x53644c), (0x4d99c0, 0x4d99e0),
                            (0x4d97b0, 0x4d97e0), (0x5da3c0, 0x5da3c1), (0x469180, 0x469350), (0x469580, 0x469850), (0x45b9d0, 0x45ba30),
                            # (animator.py's, for Animator_Play)
                            (0x46f150, 0x475010), (0x67a400, 0x690000), (0x402d70, 0x402e00), (0x402e60, 0x4030f0), (0x403150, 0x403172),
                            (0x4031e0, 0x403202), (0x4033a0, 0x4033f0), (0x5f5f10, 0x5f5fc0), (0x429df0, 0x429e17)]),
          sines=256, hooks={0x53afa0: _recorder(24, 0)})
CALLS = {'A': 0x52c6f0, 'B': 0x52ccb0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
THING = trackanim.THING
HL, H2, MODEL, BASE, OTHER = RAM + 0xb000, RAM + 0xb100, RAM + 0xb200, RAM + 0xb300, RAM + 0xb400
MESHES, FROM, DST, SRC = RAM + 0xc000, RAM + 0xc400, RAM + 0xd000, RAM + 0xd100
TRACK, RUN, ENDS, PARENTS = (0xb, 0x10), (0xb, 0xd, 0x10, 0x12, 0x19), (0xd, 0x12), (0xc, 0x11)


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

    def ahead(cell, d):   # (the cell two on, False off the map or not laid out)
        if d not in STEPS:
            return False
        own = u16(cell, 0)
        index = (own - 1) & 0xffffffff
        dx, dy = STEPS[d]
        if not (0 <= (index & 0x7f) + dx * 2 < 0x80 and 0 <= (index >> 7) + dy * 2 < 0x80):
            return False
        return cells.get((((dy * 2) << 7) + own + dx * 2) & 0xffff) is not None

    first = cells.get(ident(x, y))
    if first is None:
        return False
    done = u32(first, 8) not in RUN
    for _ in range(200):
        cell = cells.get(ident(x, y))
        if cell is None:
            return False
        if u32(cell, 8) in TRACK:
            nb = nbs(cell)
            if corner(nb):
                turn = nib(d)
                d = nb ^ (turn if turn & nb else d)
            else:
                if cell[0xe] & 0xf and not (ahead(cell, d) and ahead(cell, nib(d))):   # (a shape from cells not laid out, or a null one)
                    return False
                if not d & nb:
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
        if done:   # (A's move on reaching an end reads nothing more)
            return True
        mx, my = MOVES.get(d, (0, 0))
        x, y = x + mx, y + my
    return False


def holder(r, flash, regions):
    """The holder of thing 2 for Highlight_Flash / Unflash."""
    count = r.randrange(4)
    h = bytearray(r.bytes(0x100))
    flags = r.getrandbits(32) & ~0x20000
    if r.random() < 0.85:   # (mostly the one the flag picks: flash when clear, stop when set)
        flags = flags & ~0x4000000 if flash else flags | 0x4000000
    put(h, 4, 'I', flags)
    put(h, 8, 'I', H2)
    m = bytearray(r.bytes(0xa0))
    put(m, 0x30, 'I', r.getrandbits(32) & ~0x100000)
    put(m, 0x44, 'H', count)
    put(m, 0x70, 'I', MESHES)
    put(m, 0x9c, 'I', BASE)
    base = bytearray(r.bytes(0x80))
    put(base, 0x5c, 'I', u32(m, 0x5c) ^ (1 << r.randrange(32)))
    put(base, 0x70, 'I', FROM)
    meshes, frm = bytearray(r.bytes(0x1e0)), bytearray(r.bytes(0x1e0))
    for i in range(3):
        put(meshes, 0xa0 * i + 0x5e, 'H', r.randrange(4))
        put(meshes, 0xa0 * i + 0x64, 'I', r.choice([0, DST + 0x40 * i]))
        put(frm, 0xa0 * i + 0x64, 'I', r.choice([0, SRC + 0x40 * i, SRC + 0x40 * i]))
    other = bytearray(r.bytes(0x100))
    put(other, 0xd0, 'I', HL)
    regions.update({HL: h, H2: put(bytearray(r.bytes(0x10)), 4, 'I', MODEL), MODEL: m, BASE: base, OTHER: other, MESHES: meshes, FROM: frm,
                    DST: r.bytes(0xc0), SRC: r.bytes(0xc0)})
    regions[0x7afe14] = struct.pack('<I', r.choice([HL, HL, OTHER]))


def case(r):
    op = r.deal(OPS)
    b = op == 'B'
    start = r.choice([0, 1, 1, 0x100, 0xffffff01, 4, 0x10, 0x40, 0x41, r.getrandbits(32)])
    while True:
        x0, y0, d, hoarding, rows, cells, thing = trackanim.draw(r, b, start)
        if stops(cells, x0, y0, d, hoarding):
            break
    flash = r.choice([0, 1, 0x100, r.getrandbits(32)])
    bank5 = r.choice([0, 1, 0x100, r.getrandbits(32)])
    p_hl = 0 if start & 0xff else r.choice([0, 0.5, 1])
    for cell in cells.values():
        if r.random() < 0.3:
            cell[0xe] &= 0xf0
        if u32(cell, 4) == 1 and r.random() < p_hl:
            put(cell, 4, 'I', 2)
    regions, nlayers, blocks = animator.world(r)
    anim = bytearray(regions[animator.ANIM])
    put(anim, 4, 'I', u32(anim, 4) | 0x4000000 if flash & 0xff else u32(anim, 4) & ~0x4000000)
    regions[animator.ANIM] = anim
    holder(r, flash & 0xff, regions)
    regions.update(rows)
    regions[THING] = thing
    regions[0x81b4cc] = struct.pack('<I', r.choice([1, 0xff, 0x101]) if hoarding else r.choice([0, 0x100]))
    regions[0x7cf83c] = struct.pack('<I', trackanim.WORLD)
    regions[0x80239c] = struct.pack('<I', trackanim.WORLD)
    regions[0x8023a0] = r.bytes(4)
    regions[0x7cdba0] = PAIRS
    regions[0x7a4610] = struct.pack('<III', r.getrandbits(32), animator.ANIM, HL)
    regions[0x818c50] = r.bytes(4)
    regions[LOG] = log_region()
    return Call(CALLS[op], [THING, flash, start, bank5], regions, conv='c', ret='b' if op == 'A' else 'v')


if __name__ == '__main__':
    B.main(case)

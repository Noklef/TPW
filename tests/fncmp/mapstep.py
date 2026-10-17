# The cell steps of src/game/world/mapcell (Map_StepCell 0x4d96f0, Map_StepCellInDirection 0x4d97e0) against the originals: cell ids over the map and its edges, offsets
# and scales either side of zero, and the eight direction pairs (small or large values) with other direction values; the path nudge (0x523070) at cells
# over the map and its edges, with or without a parent, and entries two cells away that match or not.
# usage: mapstep.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Step,Dir,Nudge,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

CALLS = {'Dir28': 0x50c3e0, 'Step28': 0x50c320, 'Step': 0x4d96f0, 'Dir': 0x4d97e0, 'Spread': 0x4d8480, 'SpreadAdd': 0x4d8440, 'SpreadRemove': 0x4d8460, 'Init': 0x4d7bb0, 'Facing': 0x4de670, 'Nudge': 0x523070}
B = Bench([(0x4d96f0, 0x4d97b0), (0x4d97b0, 0x4d97e0), (0x4d97e0, 0x4d9920), (0x4d99c0, 0x4d99e0), (0x4d8300, 0x4d8328), (0x5da3c0, 0x5da3c1), (0x4d8440, 0x4d8480), (0x4d8480, 0x4d8620), (0x4d7bb0, 0x4d7ea0), (0x4de670, 0x4de7e0), (0x50c320, 0x50c3e0), (0x50c3e0, 0x50c540), (0x536300, 0x53644c), (0x522850, 0x522860),
           (0x523070, 0x52322f), (0x4d0af0, 0x4d0b27), (0x535db0, 0x535dc3), (0x53ad20, 0x53aefc)])
CELL = RAM


def step28(cell, dx, dy):
    """Map_StepCell28's id for the cell id and pair (scale 2), None outside the map."""
    index = (cell - 1) & 0xffffffff
    x, y = (index & 0x7f) + 2 * dx, (index >> 7) + 2 * dy
    if not (0 <= x < 128 and 0 <= y < 128):
        return None
    return ((2 * dy << 7) + cell + 2 * dx) & 0xffff


def nudge(r):
    """The path nudge: worlds at W1 ([0x7cf83c]: the cell, the parent entry), W2 ([0x80239c]: the entries stepped to) and G ([0x8023a0]: the
    handles' entry and cells), laid out where the case reads them."""
    w1, w2, g = 0x31000000, 0x33000000, 0x32000000
    x, y = (r.choice([0, 1, 2, 64, 125, 126, 127, r.randrange(128), r.randrange(128), 128, -1]) for _ in range(2))
    xy = RAM + 0x3000
    regions = {xy: struct.pack('<ii', x, y), 0x7cf83c: struct.pack('<I', w1), 0x80239c: struct.pack('<I', w2), 0x8023a0: struct.pack('<I', g)}
    pairs = bytearray(0x38)
    for at, d in ((0, (0, -1)), (8, (0, 1)), (0x28, (1, 0)), (0x30, (-1, 0))):
        struct.pack_into('<ii', pairs, at, *(d if r.random() < 0.9 else (r.randrange(-2, 3), r.randrange(-2, 3))))
    regions[0x7cdba0] = pairs
    group = r.choice([1, 2, 0x8000])
    types = [0xb, 0xd, 0x10, 0x12, 0x19, 0xb, 0x10, 0x19, 0xc, 0x11, 0, 3, r.getrandbits(32)]
    flags = lambda: r.choice([0x10, 0x10, 0x10, 0x30, 0, 0xffef, r.getrandbits(16)])  # noqa: E731
    cid = ((y << 7) + x + 1) & 0xffff
    cell = bytearray(r.bytes(0x44))
    h0 = r.randrange(0xf000, 0x10000)
    struct.pack_into('<H', cell, 0, h0)
    struct.pack_into('<H', cell, 0x10, group if r.random() < 0.9 else r.getrandbits(16))
    regions[w1 + 0x294 + cid * 0x44] = cell
    entry = bytearray(r.bytes(0x28))
    sid = cid if r.random() < 0.85 else r.choice([0, 0x4001, r.getrandbits(16), (cid + 1) & 0xffff])
    parent = r.random() < 0.3
    struct.pack_into('<HxxxxxxIxxH', entry, 0, sid, r.choice([0xc, 0x11]) if parent else r.choice(types[:8] + types[10:]), flags())
    p = r.randrange(0x9000, 0xa000)
    struct.pack_into('<H', entry, 0x10, p)
    regions[g + 0x10ffd8 + h0 * 0x28] = entry
    if parent:  # the parent entry (W1) and its cell (G), the stepping from the parent's id
        pentry = bytearray(r.bytes(0x28))
        sid = r.choice([cid, cid, cid ^ 1, r.randrange(1, 0x4001)])
        struct.pack_into('<HxxxxxxIxxH', pentry, 0, sid, r.choice(types), flags())
        regions[w1 + (p + 0x6cde) * 0x28] = pentry
        pcell = bytearray(r.bytes(0x44))
        struct.pack_into('<H', pcell, 0x10, group)
        if r.random() < 0.5:  # the group is the parent's, not the cell's
            struct.pack_into('<H', cell, 0x10, group ^ 1)
        regions[g - 0x44 + sid * 0x44] = pcell
    handles = bytearray(r.bytes(0x44 * 9))
    for h in range(9):
        struct.pack_into('<H', handles, 0x44 * h + 0x10, group if r.random() < 0.6 else r.choice([0, group ^ 1]))
    regions[g - 0x44 + 0x5000 * 0x44] = handles
    for at in (0, 8, 0x28, 0x30):
        n = step28(sid, *struct.unpack_from('<ii', pairs, at))
        if n is not None and w2 + (n + 0x6cde) * 0x28 not in regions:
            e = bytearray(r.bytes(0x28))
            struct.pack_into('<HxxxxxxIxxH', e, 0, 0x5000 + r.randrange(9), r.choice(types), flags())
            regions[w2 + (n + 0x6cde) * 0x28] = e
    return Call(0x523070, [xy, xy + 4], regions, ret='v')


def case(r):
    name = r.choice(OPS)
    if name == 'Nudge':
        return nudge(r)
    if name == 'Facing':
        world = RAM + 0x10000
        cells = bytearray(r.bytes(0x44 * 0x200))
        for idx in range(0x200):
            struct.pack_into('<H', cells, 0x44 * idx, idx)
            struct.pack_into('<I', cells, 0x44 * idx + 8, r.choice([3, 3, 9, 0, 1, 3]))
            cells[0x44 * idx + 0xd] = r.choice([0x10, 1, 0x40, 4, 0x10, 1, 0x40, 4, 0x80, 2])
        cell = r.choice([1, 2, 0x7f, 0x80, 0x81, 0x82, 0x100, 0x101, r.randrange(1, 0x100)])
        return Call(CALLS[name], [RAM + 0x2000, cell], {0x7cf83c: struct.pack('<I', world), 0x80239c: struct.pack('<I', world), world + 0x294: cells, RAM + 0x2000: r.bytes(8)}, ret='a')
    if name == 'Init':
        world = RAM + 0x10000
        return Call(CALLS[name], [world], {world: r.bytes(0x1d9180), 0x785610: r.bytes(0xc0)}, conv='t', ret='v')
    if name in ('Spread', 'SpreadAdd', 'SpreadRemove'):
        world = RAM + 0x10000
        records = bytearray(r.bytes(12 * 4))
        for k in range(4):
            records[12 * k + 0xa] = r.choice([0, 1, 2, 3, 3])
            struct.pack_into('<5h', records, 12 * k, *[r.choice([0, 1, -1, 100, -100, 0x7fff, -0x8000, r.randrange(-300, 300)]) for _ in range(5)])
        re_cells = bytearray(r.bytes(10 * 0x302))
        cell = r.choice([1, 2, 0x7f, 0x80, 0x81, 0x100, 0x14a, r.randrange(1, 0x100), 0xfe])
        regions = {world + 0x1d9104: records, world + 0x1b1104: re_cells}
        return Call(CALLS[name], [world] + ([r.choice([0, 1, 2])] if name == 'Spread' else []) + [r.randrange(4), cell], regions, conv='t', ret='v')
    cell = r.choice([1, 2, 0x7f, 0x80, 0x81, 0x4000, 0x3f81, 0x2000, r.randrange(1, 0x4001), 0, 0x4001, r.getrandbits(16)])
    regions = {CELL: struct.pack('<I', cell), 0x80239c: struct.pack('<I', RAM + 0x1000 + 4 * r.randrange(0x100))}
    small = lambda: r.choice([0, 1, -1, 2, -2, 3, 127, -127, 128, r.randrange(-5, 6), r.getrandbits(32) - (1 << 31)]) & 0xffffffff  # noqa: E731
    if name in ('Step', 'Step28'):
        return Call(CALLS[name], [CELL, small(), small(), r.choice([1, 1, 0, 2, 3, -1 & 0xffffffff, small()])], regions, conv='t', ret='a')
    pairs = bytearray(0x40)
    scale = r.choice([1, 1, 2, 0, 3, -1 & 0xffffffff])
    for i in range(0, 0x40, 4):
        struct.pack_into('<I', pairs, i, r.choice([0, 1, 0xffffffff, 2, 0xfffffffe, 0x7f, r.getrandbits(8)]))
    regions[0x7cdba0] = pairs
    return Call(CALLS[name], [CELL, r.choice([1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0, 3, 0x100, 0x101, 0xff, r.getrandbits(32)])] + ([scale] if name == 'Dir28' else []), regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(k for k in CALLS if k != 'Init')).split(',') if k]

if __name__ == '__main__':
    B.main(case)

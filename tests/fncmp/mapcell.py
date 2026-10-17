# The map cell handles' lookups and the memory ranges (src/game/world/mapcell: 0x4d0a70-0x4d0b10)
# and the map's bounds check, RE cells, fixed positions, block values and cell constructor
# (0x4d8300-0x4d8ca0, 0x4d8da0) and the RE cells' constructor and comparison (0x502460, 0x502470)
# the track cell's constructor (0x50c1d0), the map id of a cell (0x510870) and the 0x28-byte
# cells' accessors (0x522700-0x5228b0, 0x535db0-0x536440, 0x539210, 0x53a510-0x53aef0: track
# cells or not, 0x81b4cc set or not, parents 0 and a few in a table of the world's) against the
# originals: handles 0, 1 and wild under any gMapCells, ranges set at any index, coordinates
# about 0 and 128, positions of any value for A and B.
# usage: mapcell.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=5228b0,...: only those cell accessors)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4d0a70, 0x4d0a86), (0x4d0a90, 0x4d0ab1), (0x4d0ac0, 0x4d0ae1), (0x4d0af0, 0x4d0b06),
           (0x4d0b10, 0x4d0b27), (0x4d8300, 0x4d8328), (0x4d8410, 0x4d8434), (0x4d8610, 0x4d8750), (0x4d8c60, 0x4d8cc8), (0x4d8cd0, 0x4d8ceb), (0x4cd720, 0x4cd7e6), (0x5d60c0, 0x5d60d0),
           (0x4d8da0, 0x4d8dc2), (0x502460, 0x502487), (0x50c1d0, 0x50c1e6), (0x510870, 0x51089d), (0x522700, 0x5228b5), (0x535db0, 0x535dc3), (0x536300, 0x536484),
           (0x539210, 0x53921c), (0x53a510, 0x53a53d), (0x53ad20, 0x53aefc)])
CELL = {0x522700: 'v', 0x522730: 'v', 0x522770: 'b', 0x522790: 'a', 0x5227e0: 'v', 0x5227f0: 'v', 0x522810: 'a',
        0x522850: 'b', 0x522860: 'a', 0x522890: 'v', 0x5228a0: 'v', 0x5228b0: 'a', 0x535db0: 'a', 0x5363b0: 'a',
        **{a: 'a' for a in (0x536300, 0x536310, 0x536320, 0x536340, 0x536350, 0x536360, 0x536380, 0x536390,
                            0x5363a0, 0x5363c0, 0x5363e0, 0x53ad20, 0x53ad30, 0x53ad60, 0x53ad70, 0x53ad90,
                            0x53adb0)},
        0x539210: 'v', 0x53a510: 'v', 0x53a520: 'w', 0x53a530: 'v', 0x53ae00: 'b', 0x53ae50: 'b', 0x53ae90: 'b',
        0x53aec0: 'w', 0x53aef0: 'v', 0x5363f0: 'a', 0x536440: 'a', 0x536450: 'a'}
TYPES = [0, 1, 2, 3, 4, 5, 7, 9, 0xa, 0xb, 0xc, 0xd, 0x10, 0x11, 0x12, 0x15, 0x19, 0x1e, 0x115, 0x111]
TABLE = RAM + 0x400  # the world's cells for handles 0-5: gWorld and gMapCells placed to reach it
OPS = [int(a, 16) for a in os.environ.get('OPS', '').split(',') if a]
POS = [0x4d8610, 0x4d8650, 0x4d8690, 0x4d86d0, 0x4d8710]


def coord(r):
    return r.choice([0, 1, 0x7f, 0x80, 0x81, -1, r.randrange(-3, 0x84), r.getrandbits(32) - (1 << 31)]) & 0xffffffff


def case(r):
    mem = r.bytes(0x200)
    if not OPS and r.random() < 0.02:  # the whole-map percentage: 128 x 128 cells
        types = [2, 7, 0x1e, 0, 1, 3, 9, 0xa, 0x12]
        cells = bytearray(r.bytes(0x44 * 0x4000))
        for c in range(0x4000):
            put(cells, 0x44 * c + 8, 'I', r.choice(types) if r.random() < 0.9 else r.getrandbits(32))
        re_cells = bytearray(r.bytes(0x29000))
        world = RAM - 0x2d8  # so that the map (world + 0x2d8) is the cells region
        return Call(0x4cd720, [], {0x802390: put(bytearray(r.bytes(0x20)), 0xc, 'I', world), RAM: cells, RAM + 0x1b1000: re_cells}, ret='a')
    op = 13 if OPS or r.random() < 0.5 else r.randrange(13)  # half on the many cell accessors
    if op == 13:
        fn = r.choice(OPS or list(CELL))
        cell = bytearray(r.bytes(0x28))
        cell[2] = r.choice([2, 2, 0, 1, 3, r.getrandbits(8)])
        mode = bytearray(r.bytes(4))
        mode[0] = r.choice([0, 1, r.getrandbits(8)])
        put(cell, 0x20, 'H', r.choice([0, 1, 0xffff, r.getrandbits(16)]))
        table = bytearray(r.bytes(6 * 0x28))
        for c, k in [(cell, 0)] + [(table, i * 0x28) for i in range(6)]:
            put(c, k + 8, 'I', r.choice(TYPES + [r.randrange(0x20), r.getrandbits(32)]))
            put(c, k + 0xe, 'H', r.choice([0, 0x400, 8, r.getrandbits(16)]))
            put(c, k + 0x10, 'H', r.choice([0, 1, 2, 5, r.randrange(6)]))
            for f in (0xc, 0x22):  # side pairs: corners, straights, junctions
                c[k + f] = r.choice([0x11, 0x44, 0x14, 0x41, 0x50, 0x05, 0x55, 0x15, r.getrandbits(8)]) | r.choice([0, 0xaa, r.getrandbits(8) & 0xaa])
        h = r.randrange(6)
        if r.random() < 0.5:  # a parented cell: the one tested, or the handle's
            put(cell, 8, 'I', r.choice([0xc, 0x11]))
            put(table, h * 0x28 + 8, 'I', r.choice([0xc, 0x11]))
        args = [RAM + 0x40]
        handle = put(bytearray(4), 0, 'I', h | r.getrandbits(16) << 16)
        if fn in (0x5363f0, 0x536440, 0x536450):
            args = [RAM + 0x100]
        if fn in (0x522700, 0x522730, 0x5227e0, 0x5227f0, 0x5228a0, 0x535db0, 0x539210, 0x53a510, 0x53a530, 0x53aef0):
            args.append(r.choice([0, 1, 0x55, 0xff, 0x400, 0x8000, 0xffff, 0x10000, r.getrandbits(8), r.getrandbits(32)]))
        regions = {RAM + 0x40: cell, RAM + 0x100: handle, TABLE: table, 0x81b4cc: mode,
                   0x7cf83c: put(bytearray(4), 0, 'I', TABLE - 0x1102b0), 0x8023a0: put(bytearray(4), 0, 'I', TABLE - 0x10ffd8)}
        return Call(fn, args, regions, conv='t', ret=CELL[fn])
    if op == 12:
        return Call(0x510870, [coord(r), coord(r)], {}, ret='a')
    if op == 11:
        return Call(0x50c1d0, [RAM + r.randrange(16)], {RAM: mem}, conv='t', ret='a')
    if op == 9:
        return Call(0x502460, [RAM + r.randrange(16)], {RAM: mem}, conv='t', ret='a')
    if op == 10:  # the same but for at most one byte, or anything
        cell = bytearray(r.bytes(10))
        other = bytearray(cell)
        if r.random() < 0.6:
            other[r.randrange(10)] = r.getrandbits(8)
        return Call(0x502470, [RAM + 0x40, RAM + 0x80], {RAM + 0x40: cell, RAM + 0x80: other}, conv='t', ret='a')
    if op == 7:  # the blocks at RAM + 0x1000
        blocks = {RAM + 0x1000: r.bytes(0x1110)}
        pos = r.choice([1, 2, 0x10, 0x11, 0x80, 0x81, 0x801, 0x4000, r.randrange(1, 0x4001), r.randrange(1, 0x10000)])
        pos |= r.choice([0, r.getrandbits(16) << 16])
        m = RAM + 0x1000 - 0x1b0000
        if r.random() < 0.3:
            return Call(0x4d8cd0, [m, r.randrange(33), r.randrange(33)], blocks, conv='t', ret='a')
        if pos & 0xffff > 0x4000:
            pos &= 0xffff3fff
        if r.random() < 0.5:
            return Call(0x4d8ca0, [m, pos], blocks, conv='t', ret='a')
        return Call(0x4d8c60, [m, r.getrandbits(32), r.choice([pos, pos & 0xffff0000])], blocks, conv='t')
    if op == 8:
        return Call(0x4d8da0, [RAM + 4 * r.randrange(16)], {RAM: mem}, conv='t', ret='a')
    if op == 4:
        return Call(0x4d8300, [coord(r), coord(r)], {}, ret='a')
    if op == 5:
        return Call(0x4d8410, [r.getrandbits(32), r.choice([0, 1, 0x4000, r.getrandbits(32)])], {}, conv='t', ret='a')
    if op == 6:
        level = r.bytes(0x60)
        for k in range(0, 0x60, 4):
            if r.random() < 0.7:
                put(level, k, 'I', coord(r))
        return Call(r.choice(POS), [r.getrandbits(32), RAM + 0x100, r.choice([0, 1, 2, r.getrandbits(32)])],
                    {RAM: mem, 0x785590: level}, conv='t', ret='a')
    if op == 0:
        put(mem, 0x100, 'H', r.choice([0, 1, 2, 0x4000, 0xffff, r.getrandbits(16)]))
        cells = put(bytearray(4), 0, 'I', r.getrandbits(32))
        return Call(r.choice([0x4d0af0, 0x4d0b10]), [RAM + 0x100], {RAM: mem, 0x8023a0: cells}, conv='t', ret='a')
    if op == 1:
        return Call(0x4d0a70, [RAM + 0x40, r.randrange(-8, 0x18) & 0xffffffff, r.getrandbits(32), r.getrandbits(32),
                               r.getrandbits(32)], {RAM: mem}, conv='t')
    return Call(r.choice([0x4d0a90, 0x4d0ac0]), [r.getrandbits(32), r.choice([0x44, 0x28, r.getrandbits(32)])],
                {0x7cdb30: r.bytes(0x30)})


if __name__ == '__main__':
    B.main(case)

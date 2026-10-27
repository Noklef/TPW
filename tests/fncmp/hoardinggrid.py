# The hoardings' coarse grid (src/game/world/hoardinggrid: 0x532960) against the original: areas
# from empty (and negative) to 12 x 12 and a row past the map's 128 columns, the world's cells of
# type 0x19, parented (0xc / 0x11, parents of any id in the table) or neither, the 0x44-byte cells
# they name with flag 0x40 or not. Flood (0x5328e0): the grid copy at 0x81d7c8 all '#' but for a rectangle of up to 24 x 24 (now and then
# at an edge) of ' ', '@', '#' and others, the start in it, beside it or out of range, the fill '#', '.' or another char (not ' ' or '@').
# Check (0x531a50, the hoardings' area check): see check() below. Blocked (0x532a50): see blocked() below.
# usage: hoardinggrid.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Fill,Flood,Check,Blocked)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x532960, 0x532a50), (0x5328e0, 0x53295b), (0x4d8300, 0x4d8328), (0x4d0b10, 0x4d0b27), (0x535db0, 0x535dc3), (0x53ad20, 0x53aefc),
           (0x531a50, 0x5323ec), (0x5195d0, 0x519620), (0x522770, 0x5227a0), (0x522810, 0x522860), (0x536300, 0x536320), (0x536360, 0x536390),
           (0x6ad810, 0x6ad830), (0x5da3c0, 0x5da3c1), (0x532a50, 0x532b2d), (0x4d0af0, 0x4d0b06)])
CELLS44, TABLE = RAM, RAM + 0x1000  # 16 cells of 0x44 bytes; the world's 0x28-byte cells for ids 0 to 0x680
OPS = [k for k in os.environ.get('OPS', 'Fill,Flood,Check,Blocked').split(',') if k]
WORLD = 0x31000000   # CheckArea's world: its 0x28-byte cells at +0x1102b0, the 0x44-byte cells it names inline at +0x294 (by id)
OUTSIDE = 0xffff   # (a parent off the area, of type 0x19)
MONEY = RAM + 0x2000  # the world's table entry (0x7cfb90 + 20 * the word at world +0x1da726): its +0xc the money


def check(r):
    """CheckArea (0x531a50): mostly a rectangle 3-12 cells a side (else thinner, or a corner off the map), the corners either way round;
    the world's cells in it (per row) of types 0x19 (more often on the sides), 7 / 0x1e, 0xc / 0x11 (parents in the rectangle) or
    others, neighbour bits 0x11 / 0x44 / straight or not, naming 0x44-byte cells of types 7 / 0x1e / 2 or others with flag 0x40 or
    not; the two points at 0x7639f0 off it or on a side; the grid at 0x81eeb0 mostly '#' or mostly ' ' with '#' / '@' scattered."""
    x1, y1 = r.randrange(0, 116), r.randrange(0, 116)
    w, h = (r.choice([3, 4, 5, 6, 8, 12]), r.choice([3, 4, 5, 6, 8, 12])) if r.random() < 0.85 else (r.choice([0, 1, 2, 4]), r.choice([0, 1, 2, 4]))
    x2, y2 = x1 + w, y1 + h
    if r.random() < 0.05:
        x2, y2 = r.choice([(128, y2), (x2, 128), (-1, y2)])
    if r.random() < 0.5:
        x1, x2 = x2, x1
    if r.random() < 0.5:
        y1, y2 = y2, y1
    minx, maxx, miny, maxy = min(x1, x2), max(x1, x2), min(y1, y2), max(y1, y2)
    regions = {}
    # uniform: every 0x44-byte cell flag 0x40 (or every one without it) and none type 7 / 0x1e, few type 0x19 cells, the area all of one kind
    # but for those; a parent now and then the type 0x19 cell OUTSIDE (id 0xffff, off the area). Without flag 0x40 the type 0x19 cells are
    # mostly exactly two of one side's samples, neither a corner (a side with two passes the one-kind test, one with fewer does not)
    uniform = r.random() < 0.3
    word = r.choice([0x40, 0x40, 0])
    cells = bytearray(r.bytes(16 * 0x44))
    for i in range(16):
        put(cells, 0x44 * i + 8, 'I', r.choice([0, 5] if uniform else [7, 0x1e, 2, 0, 5] if i < 8 else [0, 1, 5, 2]))
        put(cells, 0x44 * i + 0xe, 'H', word if uniform else r.choice([0x40, 0x40, 0, 0xffff, r.getrandbits(16)]))
    ids = [(y << 7) + x + 1 for y in range(miny, maxy + 1) for x in range(minx, maxx + 1)] if maxx < 128 and minx >= 0 and maxy < 128 else [1]
    p19, p7 = (r.choice([0.05, 0.1, 0.2] if word else [0.2, 0.35, 0.5]), 0.0) if uniform else (r.choice([0.1, 0.4, 0.8]), r.choice([0.0, 0.1, 0.3]))
    flag = r.choice([0.2, 0.5, 0.9])
    pair = set()
    if uniform and not word and r.random() < 0.8:
        along = r.choice([[(minx, y) for y in range(miny, maxy + 1, 2)], [(maxx, y) for y in range(miny, maxy + 1, 2)],
                          [(x, miny) for x in range(minx, maxx + 1, 2)], [(x, maxy) for x in range(minx, maxx + 1, 2)]])
        inner = [q for q in along if q[0] not in (minx, maxx) or q[1] not in (miny, maxy)]
        pair = set(r.sample(inner, 2)) if len(inner) >= 2 else set()
        p19 = 0.0
    for y in range(miny, maxy + 1):
        if maxx >= 128 or minx < 0 or maxy >= 128:
            break
        row = bytearray(r.bytes(0x28 * (maxx - minx + 1)))
        for x in range(minx, maxx + 1):
            k = 0x28 * (x - minx)
            side = x in (minx, maxx) or y in (miny, maxy)
            handle = r.randrange(1, 9) if r.random() < p7 else r.randrange(9, 17) if r.random() < flag else r.randrange(1, 17)
            put(row, k, 'H', handle)
            row[k + 2] = r.choice([2, 2, 0, 1])
            kind = 0x19 if (x, y) in pair or r.random() < (p19 if side else 0.0 if pair else 0.02 if uniform else 0.15) else r.choice([0xc, 0x11, 0, 3, 0x18, 0x1a])
            put(row, k + 8, 'I', kind)
            bits = r.choice([0x11, 0x44, 0x11, 0x44, 0x55, 0x15, 0, 0x10, r.getrandbits(8)])
            row[k + 0xc], row[k + 0x22] = bits, r.choice([bits, bits, 0x11, 0x44, r.getrandbits(8)])
            put(row, k + 0x10, 'H', OUTSIDE if uniform and r.random() < 0.3 else r.choice(ids))
        regions[WORLD + 0x1102b0 + 0x28 * ((y << 7) + minx + 1)] = row
    if uniform:
        regions[WORLD + 0x1102b0 + 0x28 * OUTSIDE] = put(bytearray(r.bytes(0x28)), 8, 'I', 0x19)
    for x, y in ((x1, y1), (x2, y2)):   # (the corners named as cells by their own ids; off the map only the first counts)
        at = WORLD + 0x1102b0 + 0x28 * ((((y << 7) + x + 1) & 0xffff))
        if not any(a <= at < a + len(v) for a, v in regions.items()):
            regions[at] = r.bytes(0x28)
    first = WORLD + 0x294 + 0x44 * ((((y1 << 7) + x1 + 1) & 0xffff))
    regions[first] = put(bytearray(r.bytes(0x44)), 0xe, 'H', r.choice([0, 0x40]))
    for y in range(miny + 2, maxy):   # (the cells inside the border, charged for)
        n = maxx - minx - 2
        if n > 0:
            row = bytearray(r.bytes(0x44 * n))
            for i in range(n):
                put(row, 0x44 * i + 0xe, 'H', r.choice([0, 0x40, 0x40, 0xffff]))
            regions[WORLD + 0x294 + 0x44 * ((y << 7) + minx + 3)] = row
    points = [r.randrange(0, 128) | 0x100 for _ in range(4)]   # (off the map, mostly)
    for k in range(2):
        if r.random() < 0.08:
            points[2 * k], points[2 * k + 1] = r.choice([minx, maxx, x1]), r.choice(range(miny, maxy + 1)) if maxy < 128 else miny
            if r.random() < 0.5:
                points[2 * k], points[2 * k + 1] = r.choice(range(minx, maxx + 1)), r.choice([miny, maxy])
    source = bytearray(b'#' * (0x43 * 0x42))   # (mostly ' ' the flood would recurse past the emulator's 64 KB stack)
    sprinkle = r.choice([0.0, 0.0, 0.002, 0.01, 0.05, 0.2])
    for i in range(len(source)):
        if r.random() < sprinkle:
            source[i] = r.choice(b' @.A')
    entry = r.randrange(1, 12)
    regions.update({CELLS44: cells, 0x81eeb0: source, 0x81d7c8: r.bytes(0x43 * 0x43), 0x81ea78: r.bytes(0x220),
                    0x81b484: r.bytes(4), 0x81aeec: r.bytes(4), 0x81edec: r.bytes(4), 0x81b73c: r.bytes(4), 0x81af4c: r.bytes(4),
                    0x7639f0: put(bytearray(16), 0, '4i', *points), 0x78560c: put(bytearray(4), 0, 'I', r.choice([0, 1, 10, 500])),
                    0x785a2c: put(bytearray(4), 0, 'I', r.choice([0, 0, 0, 1])), 0x816d5c: r.bytes(4), 0x81b4cc: bytes([r.choice([0, 1])]),
                    0x7cf83c: put(bytearray(4), 0, 'I', WORLD), 0x8023a0: put(bytearray(4), 0, 'I', CELLS44),
                    WORLD + 0x1da726: put(bytearray(4), 0, 'H', entry), 0x7cfb90 + 20 * entry: put(bytearray(4), 0, 'I', MONEY),
                    MONEY: put(bytearray(r.bytes(0x10)), 0xc, 'i', r.choice([0, 5, 100, 1000, 100000, -5]))})
    return Call(0x531a50, [x1 & 0xffffffff, y1 & 0xffffffff, x2 & 0xffffffff, y2 & 0xffffffff, r.choice([0, 1, 1, 0x100])], regions, ret='a')


def blocked(r):
    """0x532a50 (x1, y1, x2, y2): a rectangle up to 6 x 6 cells (now and then reversed: nothing, or partly off the map), the world's
    0x44-byte cells in it (inline at +0x294 by id; [0x8023a0] their first, as the game has it) naming entries 1-16 (at +0x1102b0) of
    type 0x19, 0xc / 0x11 (parents among them) or others, flag 0x40 on the cells mostly, at times on all."""
    x1, y1 = r.choice([r.randrange(0, 126), 0, 124, -2]), r.choice([r.randrange(0, 126), 0, 124, -2])
    x2, y2 = x1 + r.randrange(0, 6), y1 + r.randrange(0, 6)
    if r.random() < 0.1:
        x1, x2 = x2, x1
    if r.random() < 0.1:
        y1, y2 = y2, y1
    every = r.random() < 0.3
    entries = bytearray(r.bytes(0x28 * 17))
    for h in range(17):
        put(entries, 0x28 * h + 8, 'I', r.choice([0x19, 0x19, 0xc, 0x11, 0, 3]))
        put(entries, 0x28 * h + 0x10, 'H', r.randrange(1, 17))
    regions = {0x7cf83c: put(bytearray(4), 0, 'I', WORLD), 0x8023a0: put(bytearray(4), 0, 'I', WORLD + 0x2d8), WORLD + 0x1102b0: entries}
    for y in range(max(y1, 0), min(y2, 127) + 1):
        lo, hi = max(x1, 0), min(x2, 127)
        if lo > hi:
            continue
        row = bytearray(r.bytes(0x44 * (hi - lo + 1)))
        for i in range(hi - lo + 1):
            put(row, 0x44 * i, 'H', r.randrange(1, 17))
            put(row, 0x44 * i + 0xe, 'H', r.getrandbits(16) | 0x40 if every or r.random() < 0.6 else r.getrandbits(16) & ~0x40)
        regions[WORLD + 0x294 + 0x44 * ((y << 7) + lo + 1)] = row
    return Call(0x532a50, [x1 & 0xffffffff, y1 & 0xffffffff, x2 & 0xffffffff, y2 & 0xffffffff], regions, ret='a')


def flood(r):
    grid = bytearray(b'#' * (0x42 * 0x43))
    h, w = r.randrange(1, 25), r.randrange(1, 25)
    top = r.choice([0, 0x42 - h, r.randrange(0x43 - h)])
    left = r.choice([0, 0x42 - w, r.randrange(0x43 - w)])
    space = r.choice([0.3, 0.5, 0.7])
    for y in range(top, top + h):
        for x in range(left, left + w):
            grid[y * 0x43 + x] = r.choice(b' @') if r.random() < space else r.choice(b'##.#A\x00')
    for y in range(0x42):
        grid[y * 0x43 + 0x42] = r.choice(b' \n')   # (the column past the last: never reached)
    row = r.choice([top + r.randrange(h)] * 6 + [-1, 0x42, top - 1, top + h])
    col = r.choice([left + r.randrange(w)] * 6 + [-1, 0x42, left - 1, left + w])
    fill = r.choice([0x23, 0x23, 0x2e, r.choice([0, 0x41, 0xff]) | (r.getrandbits(24) << 8)])
    return Call(0x5328e0, [row & 0xffffffff, col & 0xffffffff, r.getrandbits(32), fill], {0x81d7c8: grid})


def case(r):
    op = r.deal(OPS)
    if op == 'Flood':
        return flood(r)
    if op == 'Check':
        return check(r)
    if op == 'Blocked':
        return blocked(r)
    if r.random() < 0.1:
        rows, cols = r.choice([1, 2]), r.choice([127, 128, 130])
    else:
        rows, cols = r.choice([0, 1, 2, 3, 5, 12, -1, r.randrange(13)]), r.choice([0, 1, 2, 3, 6, 12, -5, r.randrange(13)])
    last = max(rows - 1, 0) * 128 + max(cols, 0) + 1
    table = bytearray(r.bytes(0x28 * (last + 1)))
    kind = r.choice([0.1, 0.3, 0.6])
    for i in range(last + 1):
        k = 0x28 * i
        put(table, k, 'H', r.randrange(1, 17))
        if r.random() < kind:
            put(table, k + 8, 'I', r.choice([0x19, 0xc, 0x11]))
        put(table, k + 0x10, 'H', r.randrange(last + 1))
    cells = bytearray(r.bytes(16 * 0x44))
    for i in range(16):
        put(cells, 0x44 * i + 0xe, 'H', r.choice([0, 0x40, 0xffff, r.getrandbits(16)]))
    regions = {CELLS44: cells, TABLE: table, 0x81eeb0: r.bytes(0x1100),
               0x81b1b0: put(bytearray(4), 0, 'i', rows), 0x81d740: put(bytearray(4), 0, 'i', cols),
               0x7cf83c: put(bytearray(4), 0, 'I', TABLE - 0x1102b0), 0x8023a0: put(bytearray(4), 0, 'I', CELLS44)}
    return Call(0x532960, [], regions)


if __name__ == '__main__':
    B.main(case)

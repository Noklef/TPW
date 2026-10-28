# Runs of type 0x19 map cells (src/game/world/cellrun: Mark 0x531040, Corners 0x530e80, its point list at 0x81b740 random before) against the original:
# a start cell (0x7639f0 / 0x7639f4) anywhere on the map, mostly
# away from the edges, and the cells around it two apart (the only ones a run can reach; each row laid out as one region, the rows and cells between random too) of
# type 0x19 mostly, with neighbour bits often one or two of the four sides, now and then random, the hoarding neighbours (0x81b4cc, a cell's +2 of 2) and
# a cell's own id (+0) now and then off. Both world pointers (0x7cf83c, and 0x80239c that Map_StepCell28 reads) are the same; the direction pairs (0x7cdba0, set at run time) are laid out. A run that would not stop
# (a cycle, or a direction that does not move) is skipped by a model of the walk, so a case is redrawn, as is one leaving the laid out cells (the memory past them is
# not the same on both sides: a run over it may never stop). So a run moved off the map (a next cell off it does not stop it) is seldom kept.
# Most cases (lay_path) lay a track of 0x19 cells with matching neighbour bits over the random ones, so that runs turn and go on. The point list's
# cap (0x401 points) needs a run with over 0x400 corners: not reached.
# The point list's own add (0x52fbf0) and commit (0x52fc30): any list, any count (at and around the cap, negative), any copy below 0x818d98.
# Lay (0x52ba50): draw()'s cells (every cell's own id right) from a start anywhere in them, the direction a side mostly (else 0, a diagonal or
# any), its end a cell the walk (modelled by lay_walk, neighbour bits taken off as it goes) reaches before it would leave the laid out cells
# (with the cells one south / east of it) or hang; setDirection 0, 1, 0x100 or any; agent B's unported 0x5370e0, 0x538fc0 and 0x53afa0
# thiscall recorders 20 of 3, 26 of 1 and 24 of none (driver.cpp's kCellRunEnv); the bounds 0x8186e4 / 0x8186e8 / 0x818c0c / 0x818c08
# dwords of their own; HoardingGrid_Fill's area (0x81d740 x 0x81b1b0) none mostly, else the one cell (0, 0) (its 0x28-byte cell, of type
# 0x19 or not, and the 0x44-byte cell its first word names, flag 0x40 or not; the start's rows then away from row 0).
# Connect (0x52b400): draw()'s cells (ids right, +0x24 words mostly one of three) from a cell near the start (its direction a side mostly),
# to a cell its walk (modelled by connect_model, directions set as it goes) handles, or with the walk at a cell of another type (-1) any cell
# off it; either end now and then a cell of type 0xc or 0x11 apart naming it, from now and then a cell apart of another type whose id names
# the start (else the walk's first cell is always from, and side A with it); words two of them one apart; link 0, 1, 0x100 or any; the side run between the junctions
# modelled too (a case either walk of which would leave the laid out cells or not end is redrawn); [0x8186ac] 0 or a thing whose +0x28 the
# call to agent B's 0x545450 passes. Agent B's 0x5370e0 (also Map_ApplyLine's, which is emulated) recorder 20 of 3, 0x53b280 7 of 3, 0x53bc60
# and 0x545450 cdecl recorders 27 of 3 and 28 of 1 (driver.cpp's kCellRunEnv).
# usage: cellrun.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Mark,Corners,Add,Commit,Lay,Connect)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, _recorder, log_region, put  # noqa: E402

B = Bench([(0x531040, 0x531167), (0x530e80, 0x530fe7),(0x4d8c00, 0x4d8c30), (0x4d8300, 0x4d8328), (0x522700, 0x5228b5), (0x53a510, 0x53a53d), (0x53ad20, 0x53aefc),
           (0x50c320, 0x50c3e0), (0x50c3e0, 0x50c540), (0x536300, 0x53644c), (0x522850, 0x522860), (0x4d99c0, 0x4d99e0), (0x4d97b0, 0x4d97e0), (0x5da3c0, 0x5da3c1),
           (0x52fbf0, 0x52fc21), (0x52fc30, 0x52fc79),
           (0x52ba50, 0x52bdd0), (0x539210, 0x539230), (0x532960, 0x532a50), (0x4d0b10, 0x4d0b40), (0x4d8c30, 0x4d8c50), (0x535db0, 0x535dc3),   # (Lay's)
           (0x52b400, 0x52b99a), (0x53aac0, 0x53ad1c)],   # (Connect's)
          hooks={0x53afa0: _recorder(24, 0), 0x5370e0: _recorder(20, 3), 0x538fc0: _recorder(26, 1), 0x53b280: _recorder(7, 3),
                 0x53bc60: _recorder(27, 3, cdecl=True), 0x545450: _recorder(28, 1, cdecl=True)})
CELLS = RAM + 0x10000
WORLD = (CELLS - 0x6cde * 0x28) & 0xffffffff
REACH = 4  # cells two apart either side of the start
STEPS = {1: (0, -1), 2: (1, -1), 4: (1, 0), 8: (1, 1), 0x10: (0, 1), 0x20: (-1, 1), 0x40: (-1, 0), 0x80: (-1, -1)}
TABLE = {1: 0, 0x10: 8, 0x20: 0x10, 8: 0x18, 0x80: 0x20, 4: 0x28, 0x40: 0x30, 2: 0x38}  # each direction's (dx, dy) at 0x7cdba0 + (set up at run time)
PAIRS = bytearray(0x40)
for _d, _at in TABLE.items():
    struct.pack_into('<ii', PAIRS, _at, *STEPS[_d])
MOVES = {1: (0, -2), 4: (2, 0), 0x10: (0, 2), 0x40: (-2, 0)}
OPS = [k for k in os.environ.get('OPS', 'Mark,Corners,Add,Commit,Lay,Connect').split(',') if k]
BD = (0xb, 0xd, 0x10, 0x12, 0x19)   # (MapCell_IsTypeBDOr10Or12Or19)


def ident(x, y):
    return ((y << 7) + x + 1) & 0xffff


def neighbours(cell, hoarding):
    return cell[0x22] if hoarding and cell[2] == 2 else cell[0xc]


def bits(v, mask):
    return bin(v & mask).count('1')


def stops(cells, x, y, hoarding):
    """The walk modelled: True when it stops within the laid out cells."""
    d, count = 4, 0
    for _ in range(200):
        cell = cells.get(ident(x, y))
        if cell is None:
            return False
        nb = neighbours(cell, hoarding)
        if struct.unpack_from('<I', cell, 8)[0] == 0x19 and bits(nb, 0x55) == 2 and nb & 0x11 and nb & 0x44:
            low = d & 0xff
            d = (low and (low << 4 if low < 0x10 else low >> 4)) & 0xff ^ nb
        done = False
        if d in STEPS:
            own = struct.unpack_from('<H', cell, 0)[0]
            index = (own - 1) & 0xffffffff
            dx, dy = STEPS[d]
            nx, ny = (index & 0x7f) + dx * 2, (index >> 7) + dy * 2
            if 0 <= nx < 0x80 and 0 <= ny < 0x80:
                nxt = cells.get((((dy * 2) << 7) + own + dx * 2) & 0xffff)
                if nxt is None:
                    return False
                if struct.unpack_from('<I', nxt, 8)[0] != 0x19:
                    done = True
        if bits(nb, 0xff) == 1 and count:
            done = True
        if done:
            return True
        mx, my = MOVES.get(d, (0, 0))
        x, y = x + mx, y + my
        count += 1
    return False


def side_bits(r):
    sides = [1, 4, 0x10, 0x40]
    return r.choice([r.choice(sides), r.choice(sides) | r.choice(sides), r.choice(sides) | r.choice(sides), r.getrandbits(8), 0])


def draw(r):
    x0 = r.choice([r.randrange(0, 128), r.randrange(8, 120), r.randrange(8, 120), r.choice([0, 1, 126, 127])])
    y0 = r.choice([r.randrange(0, 128), r.randrange(8, 120), r.randrange(8, 120), r.choice([0, 1, 126, 127])])
    hoarding = r.choice([0, 0, 1])
    p19 = r.choice([0.5, 0.8, 0.95])
    rows, cells = {}, {}
    for j in range(-2 * REACH, 2 * REACH + 1):   # (the rows between too: a run that moves by 1 lands on them)
        y = y0 + j
        lo = ident(x0 - 2 * REACH, y)
        row = bytearray(r.bytes((4 * REACH + 1) * 0x28))
        for i in range(4 * REACH + 1):
            cell = memoryview(row)[i * 0x28:(i + 1) * 0x28]
            own = (lo + i) & 0xffff
            put(row, i * 0x28, 'H', own if r.random() < 0.95 else (own + r.choice([2, -2, 0x100, 1])) & 0xffff)
            put(row, i * 0x28 + 8, 'I', 0x19 if r.random() < p19 else r.choice([0, 1, 0xb, 0x12, 0x1a, 0x119]))
            row[i * 0x28 + 2] = r.choice([2, 2, 0, 1, 3])
            row[i * 0x28 + 0xc] = side_bits(r)
            row[i * 0x28 + 0x22] = side_bits(r)
            if lo + i < 0x10000:
                cells[own] = cell
        rows[CELLS + lo * 0x28] = row
    if r.random() < 0.6:
        lay_path(r, cells, x0, y0)
    return x0, y0, hoarding, rows, cells


REVERSE = {1: 0x10, 0x10: 1, 4: 0x40, 0x40: 4}


def lay_path(r, cells, x0, y0):
    """A track over the laid out cells: from the start (east, or a corner from the west to north or south) a few steps two apart that turn now and
    then without crossing, each cell of type 0x19 with its two sides as neighbour bits (both kinds), the end cell's next one not of type 0x19 (or the
    end with one neighbour); now and then a bit flipped."""
    i, j = 0, 0
    first = r.choice([4, 4, 1, 0x10])
    seen, path = {(0, 0)}, [(0, 0, 0x40 if first != 4 else r.choice([0, 0x40]), first)]
    d = first
    for _ in range(r.randrange(1, 14)):
        mx, my = MOVES[d]
        i, j = i + mx // 2, j + my // 2
        if abs(i) > REACH - 1 or abs(j) > REACH - 1 or (i, j) in seen:
            break
        seen.add((i, j))
        nd = d if r.random() < 0.6 else r.choice([k for k in MOVES if k not in (d, REVERSE[d])])
        path.append((i, j, REVERSE[d], nd))
        d = nd
    last = len(path) - 1
    for k, (i, j, back, fwd) in enumerate(path):
        cell = cells.get(ident(x0 + 2 * i, y0 + 2 * j))
        if cell is None:
            return
        nb = back | fwd if k < last or r.random() < 0.5 else back
        if r.random() < 0.05:
            nb ^= 1 << r.randrange(8)
        cell[0xc] = cell[0x22] = nb
        struct.pack_into('<I', cell, 8, 0x19)
        struct.pack_into('<H', cell, 0, ident(x0 + 2 * i, y0 + 2 * j))
    i, j, _, fwd = path[last]
    mx, my = MOVES[fwd]
    end = cells.get(ident(x0 + 2 * i + mx, y0 + 2 * j + my))
    if end is not None:
        struct.pack_into('<I', end, 8, r.choice([0, 1, 0xb, 0x12, 0x1a, 0x119]))


def nibble(v):
    v &= 0xff
    return 0 if not v else (v << 4) & 0xff if v < 0x10 else v >> 4


def shift_log2(a, b):
    a, cl = a & 0xff, 0
    while a > 1:
        a, cl = a >> 1, cl + 1
    v = (b & 0xff) << cl
    return (v >> 8) & 0xff if v > 0x80 else v & 0xff


def lay_walk(cells, x, y, d, hoarding, limit=40):
    """Lay's walk modelled on copies of the cells: the positions it fully handles in order, until it would leave the laid out cells (the cell
    or the ones one south / east of it that are in the map), hang in a junction's search, or after limit steps."""
    cells = {k: bytearray(v) for k, v in cells.items()}
    path = []
    for _ in range(limit):
        if not (0 <= x < 0x80 and 0 <= y < 0x80):
            break
        cell = cells.get(ident(x, y))
        if cell is None or any(0 <= x + dx < 0x80 and 0 <= y + dy < 0x80 and ident(x + dx, y + dy) not in cells for dx, dy in ((0, 1), (1, 0), (1, 1))):
            break
        side = 0x22 if hoarding and cell[2] == 2 else 0xc
        nb = cell[side]
        if struct.unpack_from('<I', cell, 8)[0] != 0x19 and not (hoarding and nb):
            d = 0
        else:
            sides = bits(nb, 0x55)
            if sides == 2 and ((nb & 1 and nb & 4) or (nb & 1 and nb & 0x40) or (nb & 0x10 and nb & 4) or (nb & 0x10 and nb & 0x40)):
                back = nibble(d)
                if not back & nb:
                    back = d
                d = (nb ^ back) & 0xff
            if sides > 2:
                m, bit = 0, shift_log2(d, 0x40)
                for _ in range(9):
                    if bit & nb:
                        m = bit
                    bit = shift_log2(bit, 4)
                    if m:
                        break
                if not m:
                    break
                if d != m:
                    cell[side] &= ~d & 0xff
                    clear = nibble(m)
                else:
                    clear = ~(nibble(d) | d) & 0xff
                cell[side] &= ~clear & 0xff
                d = m
        path.append((x, y))
        mx, my = MOVES.get(d, (0, 0))
        x, y = x + mx, y + my
    return path


def lay_case(r):
    while True:
        x0, y0, hoarding, rows, cells = draw(r)
        fill = r.random() < 0.4
        if fill and y0 < 9:   # (HoardingGrid_Fill's cell (0, 0), id 1, apart from the laid out ones)
            continue
        for own, cell in cells.items():
            struct.pack_into('<H', cell, 0, own)
        d = r.choice([4, 4, 1, 0x10, 0x40, 0, 2, 0x80, r.getrandbits(8)]) | r.choice([0, 0, r.getrandbits(24) << 8])
        sx, sy = x0 + r.choice([0, 0, 0, -2, 2, -1, 1]), y0 + r.choice([0, 0, 0, -2, 2, 1])
        path = lay_walk(cells, sx, sy, d & 0xff, hoarding)
        if path:
            break
    ex, ey = path[r.choice([len(path) - 1, len(path) - 1, r.randrange(len(path))])]
    regions = dict(rows)
    log = log_region()
    regions.update({LOG: log, 0x7cdba0: PAIRS, 0x7cf83c: struct.pack('<I', WORLD), 0x80239c: struct.pack('<I', WORLD),
                    0x81b4cc: struct.pack('<I', r.choice([1, 0xff, 0x101]) if hoarding else r.choice([0, 0x100])),
                    0x8186e4: r.bytes(8), 0x818c08: r.bytes(8), 0x81d740: struct.pack('<i', 1 if fill else r.choice([0, 0, -1])),
                    0x81b1b0: struct.pack('<i', 1 if fill else r.choice([0, 0, 1]))})
    if fill:
        h = r.choice([1, 2])
        first = put(bytearray(r.bytes(0x28)), 8, 'I', r.choice([0x19, 0, 1, 0xb]))
        put(first, 0, 'H', h)
        regions[CELLS + 0x28] = first
        regions[(WORLD + 0x294 + 0x44 * h) & 0xffffffff] = r.bytes(0x44)
        regions[0x8023a0] = struct.pack('<I', (WORLD + 0x294 + 0x44) & 0xffffffff)   # (gMapCells: id 1's at WORLD +0x294 +0x44)
        regions[0x81eeb0] = r.bytes(0x10)
    flag = r.choice([0, 1, 1, 0x100, r.getrandbits(32)])
    return Call(0x52ba50, [sx, sy, ex, ey, d, flag], regions, conv='c', ret='a')


def corner(nb):
    return bits(nb, 0x55) == 2 and bool((nb & 1 and nb & 4) or (nb & 1 and nb & 0x40) or (nb & 0x10 and nb & 4) or (nb & 0x10 and nb & 0x40))


def straight(nb):
    return bits(nb, 0x55) == 2 and bool((nb & 1 and nb & 0x10) or (nb & 4 and nb & 0x40))


def junction(nb):
    return bits(nb, 0x55) > 2


def kind(cell):
    return struct.unpack_from('<I', cell, 8)[0]


def word(cell):
    return struct.unpack_from('<H', cell, 0x24)[0]


def s16(v):
    return v - 0x10000 if v & 0x8000 else v


def xy_of(key):
    index = (key - 1) & 0xffffffff
    return index & 0x7f, index >> 7


def connect_model(cells, fkey, tkey, hoarding, link, alone=None, limit=40):
    """Connect's walks modelled on copies of the cells. tkey None: the keys of the cells the walk handles as far as it goes within limit steps
    and the laid out cells, and whether it then meets a cell of another type (-1). Else True when the walk and the side run both end. alone:
    from is that cell apart (its id fkey) instead of the laid out one."""
    cells = {k: bytearray(v) for k, v in cells.items()}
    frm = cells[fkey] if alone is None else alone
    wf = word(frm)
    wt = word(cells[tkey]) if tkey is not None else 0
    if link and tkey is not None and s16(wt) - s16(wf) < 0:
        return True
    x, y = xy_of(fkey)
    d = frm[0xd]
    path, a, b = [], None, None
    for _ in range(limit):
        key = ident(x, y)
        cell = cells.get(key)
        if cell is None:
            break
        if kind(cell) not in BD:
            return (path, True) if tkey is None else True
        nb = neighbours(cell, hoarding)
        if link:
            if a is None:
                if word(cell) == wf:
                    a = key
                if a is None and junction(nb):
                    a = key
            if word(cell) == wt:
                b = key
            if b is None and junction(nb):
                b = key
        if corner(nb) and path:
            back = nibble(d)
            if not back & nb:
                back = d
            d = (nb ^ back) & 0xff
        if link:
            cell[0xd] = (cell[0xd] | d) if junction(nb) else d
        path.append(key)
        if key == tkey:
            break
        mx, my = MOVES.get(d, (0, 0))
        x, y = x + mx, y + my
    else:
        return (path, False) if tkey is None else False
    if tkey is None:
        return path, False
    if path[-1] != tkey:
        return False
    if link and a is not None and b is not None and a != b:
        x, y = xy_of(a)
        d = frm[0xd] if junction(neighbours(cells[a], hoarding)) else cells[a][0xd]
        for _ in range(limit):
            key = ident(x, y)
            cell = cells.get(key)
            if cell is None:
                return False
            stop = key == b
            if kind(cell) not in BD:
                stop = True
            elif bits(cell[0xd], 0x55) == 1:
                d = cell[0xd]
            else:
                cell[0xd] &= ~d & 0xff
            mx, my = MOVES.get(d, (0, 0))
            x, y = x + mx, y + my
            if stop:
                return True
        return False
    return True


def connect_case(r):
    while True:
        x0, y0, hoarding, rows, cells = draw(r)
        w = r.getrandbits(16)
        words = [w, (w + 1) & 0xffff, r.getrandbits(16)]   # (two a word apart: 0x53bc60's first call at most 1 apart)
        for own, cell in cells.items():
            struct.pack_into('<H', cell, 0, own)
            if r.random() < 0.7:
                struct.pack_into('<H', cell, 0x24, r.choice(words))
        fkey = ident(x0 + r.choice([0, 0, 0, -2, 2, -1, 1]), y0 + r.choice([0, 0, 0, -2, 2, 1]))
        if fkey not in cells or not fkey:
            continue
        alone = None
        if r.random() < 0.15:   # (from a cell apart whose id names the start: the walk's first cell not from)
            alone = put(put(bytearray(r.bytes(0x28)), 8, 'I', r.choice([0x19, 0x19, 0xb, 0])), 0, 'H', fkey)
            put(alone, 0x24, 'H', r.choice(words))
            alone[2] = cells[fkey][2]
            alone[0xc] = alone[0x22] = r.choice([cells[fkey][0xc], side_bits(r)])
        start = cells[fkey] if alone is None else alone
        start[0xd] = r.choice([4, 4, 1, 0x10, 0x40, r.getrandbits(8)])
        link = r.choice([0, 1, 1, 1, 0x100, r.getrandbits(32)])
        path, failed = connect_model(cells, fkey, None, hoarding, link, alone)
        if not path and not failed:
            continue
        if failed and (not path or r.random() < 0.3):
            off = [k for k in cells if k not in path and k]
            if not off:
                continue
            tkey = r.choice(off)
        else:
            tkey = r.choice([path[-1], path[-1], r.choice(path)])
        if connect_model(cells, fkey, tkey, hoarding, link, alone):
            break
    regions = dict(rows)
    args = []
    if alone is not None:
        regions[RAM + 0x180] = alone
    for at, key in ((RAM + 0x100, fkey), (RAM + 0x140, tkey)):
        if at == RAM + 0x100 and alone is not None:
            args.append(RAM + 0x180)
        elif r.random() < 0.15:   # (an end of type 0xc or 0x11 naming the cell)
            stand = put(put(bytearray(r.bytes(0x28)), 8, 'I', r.choice([0xc, 0x11])), 0x10, 'H', key)
            regions[at] = stand
            args.append(at)
        else:
            args.append((CELLS + key * 0x28) & 0xffffffff)
    track = r.choice([0, RAM + 0x200])
    if track:
        regions[track] = r.bytes(0x30)
    regions.update({LOG: log_region(), 0x7cdba0: PAIRS, 0x7cf83c: struct.pack('<I', WORLD), 0x80239c: struct.pack('<I', WORLD),
                    0x81b4cc: struct.pack('<I', r.choice([1, 0xff, 0x101]) if hoarding else r.choice([0, 0x100])),
                    0x8186ac: struct.pack('<I', track)})
    return Call(0x52b400, args + [link], regions, conv='c', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Lay':
        return lay_case(r)
    if op == 'Connect':
        return connect_case(r)
    if op == 'Add':
        count = r.choice([0, 1, 5, 0x3ff, 0x400, 0x401, 0x402, 0xffffffff, r.randrange(0x400)])
        regions = {0x81b740: r.bytes(0x2010), 0x820a80: put(bytearray(r.bytes(0x20)), 0xc, 'I', count)}
        return Call(0x52fbf0, [r.getrandbits(32), r.getrandbits(32)], regions, conv='c', ret='a')
    if op == 'Commit':
        count = r.choice([0, 1, 5, 0x400, 0x401, 0xffffffff, 0xfffffffb])
        regions = {0x81b740: r.bytes(0x2010), 0x818d90: r.bytes(0x2010), 0x820a80: put(bytearray(r.bytes(0x20)), 0xc, 'I', count)}
        return Call(0x52fc30, [], regions, conv='c', ret='a')
    while True:
        x0, y0, hoarding, rows, cells = draw(r)
        if stops(cells, x0, y0, hoarding):
            break
    regions = dict(rows)
    if op == 'Corners':
        regions[0x81b740] = r.bytes(0x800)
        regions[0x820a8c] = r.bytes(4)
        regions[0x81ede4] = r.bytes(8)
    regions[0x7639f0] = struct.pack('<ii', x0, y0)
    regions[0x818c50] = r.bytes(4)
    regions[0x81b4cc] = struct.pack('<I', r.choice([1, 0xff, 0x101]) if hoarding else r.choice([0, 0x100]))
    regions[0x7cf83c] = struct.pack('<I', WORLD)
    regions[0x80239c] = struct.pack('<I', WORLD)
    regions[0x7cdba0] = PAIRS
    if op == 'Corners':
        return Call(0x530e80, [], regions, conv='c', ret='v')
    return Call(0x531040, [r.choice([0, 1, 0x100, 0xffffff01, r.getrandbits(32)])], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)

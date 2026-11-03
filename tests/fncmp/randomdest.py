# A person's next random destination (0x4f9490 Person_SetRandomDest, src/game/world/person) against the original: routesearch.py's box of cells (the
# route's world, ringed so the search stays in it; the destination the nav's route is asked for may be in the box or past it), the person in the box
# with its nav in its cell. [0x8023a0] is the box's ENTRIES there, not the route's cells, so the walk's own cells lie at that base (a second map about
# the person: all types, every neighbour and direction bit), the neighbours' type 0xa read from the route's cells ([0x80239c]). Kinds 4-8 (staff) or
# not, the patrol region about the person, excluding it, narrow or wild; the timer +0x198 0, past the counter (cleared), or in force over
# objectblocks.py's block values (then the map's type 3 cells face up or right only: the block test's walk would otherwise not end). Sides-less cells
# take the rings' and the five tries' path. Thoughts_Show as thoughtshow.py's.
# usage: randomdest.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import cellstep  # noqa: E402
import navpath  # noqa: E402
import objectblocks  # noqa: E402
import routesearch  # noqa: E402
import wctimage  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

B = Bench(wctimage.merged(navpath.RANGES + [
    (0x4d8300, 0x4d8328), (0x4f9490, 0x4f9e3e), (0x4fa530, 0x4fa600), (0x506ed0, 0x506f2e), (0x506f30, 0x50703b), (0x50be80, 0x50c075),
    (0x516330, 0x516370)]),
    hooks={0x550ca0: _recorder(0, 0), 0x67a7d0: _recorder(1, 1, cdecl=True), 0x475550: _recorder(2, 1, cdecl=True),
           0x475a10: _recorder(3, 7, cdecl=True)})
WORLD, ENTRIES = cellstep.WORLD, cellstep.ENTRIES
MARGIN = 7   # (the walk's cells about the box, as far as the walk and the tries of 5 go)


def poke(regions, r, at, fmt, *values):
    """Writes into the world's pages, a new page drawn random."""
    page = at & ~0xfff
    if page not in regions:
        regions[page] = bytearray(r.bytes(0x1000))
    elif not isinstance(regions[page], bytearray):
        regions[page] = bytearray(regions[page])
    put(regions[page], at - page, fmt, *values)


def walk_cells(regions, r, x0, y0, x1, y1, safe):
    """The second map at [0x8023a0]: a row of cells each, ids (y << 7) + x + 1."""
    for y in range(max(y0, 0), min(y1, 0x7f) + 1):
        lo, hi = max(x0, 0), min(x1, 0x7f)
        row = bytearray(r.bytes(0x44 * (hi - lo + 1)))
        for k in range(hi - lo + 1):
            at = 0x44 * k
            kind = r.choice([1, 1, 1, 1, 3, 3, 9, 0xa, 0, 4, 2])
            put(row, at + 8, 'I', kind)
            row[at + 2] = r.choice([0, 1, 2])
            bits = r.choice([0x55, 0x55, 0x51, 0x45, 0x15, 0x54, 0x11, 0x44, 0x10, 4, 1, 0x40, 0, 0xff, r.getrandbits(8)])
            row[at + 0xc] = bits
            row[at + 0x22] = r.choice([bits, 0, r.getrandbits(8)])
            if safe and kind == 3:
                row[at + 0xd] = r.choice([0x10, 0x40, 0x10, 0x40, 0, 0x80, 2, 0x50])
            elif safe:
                row[at + 0xd] = r.choice([0x10, 0x40, 1, 4, 0])
            else:
                row[at + 0xd] = r.choice([1, 0x10, 4, 0x40, 1, 0x10, 4, 0x40, 0, 0x80, r.getrandbits(8)])
        regions[ENTRIES + (y << 7 | lo) * 0x44] = row


def region(r, x, y, box):
    """The patrol region's words: about the person, past it, narrow; now and then any way."""
    bx, by, size = box
    kind = r.random()
    if kind < 0.45:
        lo, hi = (max(x - r.randrange(4), 0), max(y - r.randrange(4), 0)), (min(x + r.randrange(4), 0x7f), min(y + r.randrange(4), 0x7f))
    elif kind < 0.7:   # (the person just inside: an edge at it)
        lo, hi = (x, max(y - 2, 0)), (min(x + 2, 0x7f), y)
    elif kind < 0.97:  # (the person outside)
        cx, cy = bx + r.randrange(size), by + r.randrange(size)
        lo, hi = (max(cx - 1, 0), max(cy - 1, 0)), (min(cx + 1, 0x7f), min(cy + 1, 0x7f))
    else:
        return r.getrandbits(16) | 1, r.getrandbits(16) | 1
    return (lo[1] << 7 | lo[0]) + 1, (hi[1] << 7 | hi[0]) + 1


def case(r):
    mode = r.choice([0, 1, 2, 2, 2, 3])
    size = r.choice([6, 8, 10])
    regions, box, ring, bx, by, size = routesearch.boxed(r, mode, size)
    x, y = r.choice(sorted(box))
    person = bytearray(r.bytes(0x220))
    person[2] = r.choice([4, 5, 6, 7, 8, 0, 1, 2, 3, 9])
    person[5], person[7] = x, y
    put(person, 0xd4 + 8, 'II', navpath.fixed(x, r), navpath.fixed(y, r))
    put(person, 0xd4 + 0xb4, 'I', mode)
    put(person, 0x20a, 'HH', *region(r, x, y, (bx, by, size)))
    counter = r.choice([0, 100, 5000, 0xfffffff0, r.getrandbits(32)])
    kind = r.random()
    if kind < 0.6:
        timer = 0
    elif kind < 0.75:
        timer = (counter + r.choice([2, 3, 100])) & 0xffffffff or 2
    else:
        timer = (counter + r.choice([0, 1])) & 0xffffffff or 1
    safe = timer != 0 and not (counter + 1) & 0xffffffff < timer   # (in force: the block test runs)
    put(person, 0x198, 'I', timer)
    person[0xb4:0xb8] = struct.pack('<I', r.choice([0, 0, r.randrange(1, 64), r.getrandbits(32)]))
    last = r.choice([0, r.randrange(100), r.getrandbits(32), 0xffffffff - r.randrange(60)])
    put(person, 0xbc, 'I', last)
    regions[RAM] = person
    navpath.common(regions, r, counter)
    walk_cells(regions, r, bx - MARGIN, by - MARGIN, bx + size + MARGIN, by + size + MARGIN, safe)
    if r.random() < 0.15:   # (no side neighbours here: the rings)
        row = next(v for k, v in regions.items() if k == ENTRIES + (y << 7 | max(bx - MARGIN, 0)) * 0x44)
        row[(x - max(bx - MARGIN, 0)) * 0x44 + 0xc] = r.choice([0, 0xaa])
        row[(x - max(bx - MARGIN, 0)) * 0x44 + 0x22] = r.choice([0, 0xaa])
    for yy in range(max(by - MARGIN, 0), min(by + size + MARGIN, 0x7f) + 1):   # (the neighbours' type 0xa, in the route's cells)
        for xx in range(max(bx - MARGIN, 0), min(bx + size + MARGIN, 0x7f) + 1):
            routesearch.cell_at(regions, r, (yy << 7 | xx) + 1)
    pairs = bytearray(r.bytes(0x38))
    for at, d in objectblocks.STEPS.items():
        put(pairs, at - 0x7cdba0, 'ii', *d)
    regions[0x7cdba0] = pairs
    regions[0x80239c] = struct.pack('<I', WORLD)
    poke(regions, r, WORLD + 0x1da708, 'II', r.getrandbits(32), (last + r.choice([0, 1, r.randrange(80), r.getrandbits(32)])) & 0xffffffff)
    if safe:   # (the block test's: objectblocks.py's offsets and grid)
        offsets = bytearray(0x48)
        for k in range(9):
            put(offsets, 8 * k, 'ii', r.randrange(-2, 3), r.randrange(-2, 3))
        regions[0x7ced88] = offsets
        near = [timer, timer, (timer - 1) & 0xffffffff, (timer + 1) & 0xffffffff, 0, 1, 0xffffffff]
        regions[WORLD + 0x2d8 + 0x1b0000] = b''.join(struct.pack('<I', r.choice(near + [r.getrandbits(32)])) for _ in range(33 * 32))
    regions.update({
        LOG: log_region(),
        0x790ab0: bytes([r.choice([0, 0, 0, 0, 1, 2, 4, 0x10, 0x20, 0xe9, r.getrandbits(8)])]),
        0xfb34aa: bytes([r.choice([1, 1, 1, 0, 2, 3, r.getrandbits(8)])]),
        0xfb3b7c: struct.pack('<I', r.choice([0, 1, 1, 2, r.getrandbits(32)])),
    })
    return Call(0x4f9490, [RAM], regions, conv='t', ret='a', steps=20_000_000)   # (up to 37 route searches, each up to 0xf8 cells or so)


if __name__ == '__main__':
    B.main(case)

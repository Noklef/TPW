# The ends of a line on the map's 0x28-byte cells (src/game/world/lineends: TrackCell_ResetEnds 0x532dd0, LineEnd_Unlink 0x53bc60) against
# the originals. cellapply.py's scene for MapCell_ApplyModel (cellmodel.py's tables, kept cells, variant counter and world, animator.py's
# world, the placers' and Ride_Destroy's recorders), the world ([0x7cf83c] = [0x80239c], cellmodel.py's) holding the 9 x 9 cells about the
# cell at + 0x1102b0 + id * 0x28 (kinds 0, 5, 0xd, 0x12 and the picking ones; neighbour bits as cellmodel.py's; the four sides as
# directions mostly; the end flags (+0xe's low four bits) of the sorts the code tells apart; a ride now and then) and their 0x44-byte cells
# ([0x8023a0] + (id - 1) * 0x44, +8 0x15 or not). The line's start and end (0x7639e0 / 0x7639e4, 0x7639e8 / 0x7639ec) anywhere, -1, or
# for Unlink: the end on the cell and the start two cells along a row or column from it, mostly.
# The direction pairs (0x7cdba0) the real ones mostly.
# Ends: the cell (in the world), the pairs mostly the opposite sides (west / east, north / south), now and then any of -1, 0 and 1. A cell
# whose direction is none of the four sides has ends without end flags: its flags would be a stack address (sic), not the same on both sides.
# Unlink: x and y the cell's (x now and then -1 or -2: its id a cell of the row before, x's high bytes in the direction's slot), dir 0, a
# side, a diagonal, any or one whose low byte is 0 (only that byte counts); the cell's neighbour bits now and then its direction and one other.
# usage: lineends.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ends,Unlink)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import cellapply  # noqa: E402
import cellmodel  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

WORLD = cellmodel.WORLD
TABLE = WORLD + 0x1102b0
MAPC = 0x36000000
CALLS = {'Ends': 0x532dd0, 'Unlink': 0x53bc60}
RANGES = cellapply.merged(cellapply.RANGES + [
    (0x532dd0, 0x532f5e), (0x53bc60, 0x53bf2c), (0x50c320, 0x50c5e0), (0x4d97b0, 0x4d97e0), (0x4d99c0, 0x4d99e0), (0x4d8300, 0x4d8330),
    (0x53a510, 0x53a540), (0x53ad20, 0x53ad30), (0x535db0, 0x535dd0), (0x5da3c0, 0x5da3d0)])
B = Bench(RANGES, sines=256, hooks=cellapply.HOOKS)
SIDES = [1, 4, 0x10, 0x40]
KINDS = [0, 5, -3, 0xd, 0xd, 0x12, 0x12, 3, 0xb, 0x10, 0x19, 1, 9, 0xc, 0x11]
FLAGS = [0, 1, 2, 4, 8, 6, 0xa, 0xc, 3, 9, 0xf, 0x12, 0x18, 0x14]
PAIRS = [(0, -1), (0, 1), (-1, 1), (1, 1), (-1, -1), (1, 0), (-1, 0), (1, -1)]   # (0x7cdba0..: N, S, SW, SE, NW, E, W, NE; .bss)


def pairs(r):
    """The direction pairs (0x7cdba0), the real ones mostly."""
    return b''.join(struct.pack('<ii', *(d if r.random() < 0.97 else (r.randrange(-1, 2), r.randrange(-1, 2)))) for d in PAIRS)


def ident(x, y):
    return ((y << 7) + x + 1) & 0xffff


def cell(r, h):
    c = bytearray(r.bytes(0x28))
    put(c, 0, 'H', h)
    c[2] = r.choice([2, 2, 0, 1])
    put(c, 4, 'I', r.choice([0, 0, 0, 0, 5]))
    put(c, 8, 'i', r.choice(KINDS + [r.getrandbits(32) - (1 << 31)]))
    c[0xc] = cellmodel.bits(r)
    c[0x22] = c[0xc] if r.random() < 0.5 else cellmodel.bits(r)
    c[0xd] = r.choice(SIDES * 2 + [2, 8, 0x20, 0x80, 0, r.getrandbits(8)])
    put(c, 0xe, 'H', r.choice(FLAGS * 2 + [r.getrandbits(16)]))
    put(c, 0x20, 'h', r.choice([0, 0, 1, -1, 2]))
    return c


def block(r, regions, x, y, reach=4):
    """The world's cells within reach of (x, y) (inside the map) and their 0x44-byte cells."""
    for yy in range(max(0, y - reach), min(127, y + reach) + 1):
        x0, x1 = max(0, x - reach), min(127, x + reach)
        regions[TABLE + ident(x0, yy) * 0x28] = b''.join(cell(r, ident(xx, yy)) for xx in range(x0, x1 + 1))
        mapc = bytearray(r.bytes(0x44 * (x1 - x0 + 1)))
        for i in range(x1 - x0 + 1):
            put(mapc, i * 0x44 + 8, 'I', r.choice([0x15, 0, 1, 0x14, r.getrandbits(32)]))
        regions[MAPC + (ident(x0, yy) - 1) * 0x44] = mapc


def spot(r):
    return r.choice([r.randrange(128), r.randrange(4, 124), r.randrange(4, 124), 0, 1, 2, 125, 126, 127])


def case(r):
    op = r.deal(OPS)
    regions = dict(cellapply.apply_case(r).regions)
    regions.update({0x80239c: struct.pack('<I', WORLD), 0x7cf83c: struct.pack('<I', WORLD), 0x8023a0: struct.pack('<I', MAPC),
                    0x7cdba0: pairs(r)})
    ends = [r.choice([spot(r), -1]) for _ in range(4)]
    x, y = spot(r), spot(r)
    if op == 'Ends':
        block(r, regions, x, y)
        a, b = r.choice([((-1, 0), (1, 0)), ((0, -1), (0, 1)), ((1, 0), (-1, 0)), ((0, 1), (0, -1))])
        if r.random() < 0.15:
            a, b = (r.randrange(-1, 2), r.randrange(-1, 2)), (r.randrange(-1, 2), r.randrange(-1, 2))
        regions[0x7639e0] = struct.pack('<4i', *ends)
        here = TABLE + ident(x, y) * 0x28
        if regions[TABLE + ident(max(0, x - 4), y) * 0x28][here - TABLE - ident(max(0, x - 4), y) * 0x28 + 0xd] not in SIDES:
            # (no side direction: the flags tested and set are a stack address's, which the two sides do not share; ends without end flags)
            for px, py in a, b:
                ex, ey = x + 2 * px, y + 2 * py
                if 0 <= ex < 128 and 0 <= ey < 128:
                    row = TABLE + ident(max(0, x - 4), ey) * 0x28
                    data = bytearray(regions[row])
                    data[(ex - max(0, x - 4)) * 0x28 + 0xe] &= 0xf0
                    regions[row] = data
        return Call(CALLS[op], [here, a[0], a[1], b[0], b[1]], regions, ret='v')
    ax, ay = x, y
    if r.random() < 0.05:   # (x's high bytes: the id one of the row before's)
        ax, ay = r.choice([-1, -2]), r.randrange(1, 128)
        x, y = (ident(ax, ay) - 1) & 0x7f, (ident(ax, ay) - 1) >> 7
    block(r, regions, x, y)
    row = TABLE + ident(max(0, x - 4), y) * 0x28
    c = bytearray(regions[row])
    at = (x - max(0, x - 4)) * 0x28
    if r.random() < 0.4:   # (the direction and one other: the end's cut of a single direction)
        d = c[at + 0xd] = r.choice(SIDES)
        c[at + 0xc] = c[at + 0x22] = d | r.choice(SIDES + [2, 8, 0x20, 0x80])
    regions[row] = c
    if r.random() < 0.35:
        ends[2:] = [ax, ay]
    if r.random() < 0.6:
        dx, dy = r.choice([(2, 0), (-2, 0), (0, 2), (0, -2)])
        ends[:2] = [ax + dx, ay + dy]
    regions[0x7639e0] = struct.pack('<4i', *ends)
    d = r.choice([0, 0, 0, r.choice(SIDES), r.choice([2, 8, 0x20, 0x80]), r.getrandbits(32), r.choice([0x100, 0x4000, 0xff00])])
    return Call(CALLS[op], [ax, ay, d], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

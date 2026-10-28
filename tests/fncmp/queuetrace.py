# A ride's queue line traced from its entrance (src/game/world/queuetrace: RideQueue_Trace 0x530120) against the original. trackline.py's
# map in the game's layout (its 0x44-byte map cells [0x7cf83c] + 0x294 + id * 0x44, laid out over the map's corner, MapCell_ApplyPathModel's
# scene) with a queue line on it: from the entrance (the thing's +0x36, a cell of the laid-out map, now and then off it: handle 0) a step a
# cell of queue cells (types 3 / 9: bits the way in and out, straight or a corner) ending at a path cell (type 1, more bits now and then),
# a dead end (one bit) or another kind; the entrance's bits the way out mostly (or straight through, 0 or any), its type a queue's, a
# path's or another, its +0x10 word the first queue cell's now and then (so that the walk starts at the entrance). The hoarding flag
# (0x81b4cc) and the cells' +2 either way (both bit bytes alike). A walk that would not stop, or would leave the laid-out cells, is caught
# by a model of it and the line redrawn (else the entrance's bits 0). The thing's angle (+0x10) one of the four mostly, its address's low
# byte (the facing for other angles) one of several. The point list (0x81b740) and its count start random (the count is cleared).
# usage: queuetrace.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import cellapply  # noqa: E402
import trackline  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

ident, CELLS = trackline.ident, trackline.CELLS
B = Bench(cellapply.merged(trackline.RANGES + [(0x530120, 0x5306d8), (0x52fbf0, 0x52fc30), (0x4d96f0, 0x4d97e0), (0x4d99c0, 0x4d99e0),
                                                (0x4d8300, 0x4d8330), (0x4d8c00, 0x4d8cb0), (0x522700, 0x5228b5), (0x536300, 0x53644c),
                                                (0x53ad20, 0x53aefc)]),
          sines=256, hooks=trackline.HOOKS)
THING = 0x3e000000
STEP = {1: (0, -1), 4: (1, 0), 0x10: (0, 1), 0x40: (-1, 0)}
REVERSE = {1: 0x10, 4: 0x40, 0x10: 1, 0x40: 4}
QUEUE = [3, 3, 9]
PAIR = {1: 0, 0x10: 8, 4: 0x28, 0x40: 0x30}   # (each direction's pair: 0x7cdba0 + this)


def nibble(v):
    return v if v == 0 else (v << 4 if v < 0x10 else v >> 4) & 0xff


def queue(r, x, y, w, h):
    """A queue line from (x, y) (not on it) in columns 0..w and rows 0..h: its first direction and {(x, y): (type, bits)}."""
    first = r.choice(list(STEP))
    facing, cells = first, {}
    x, y = x + STEP[first][0], y + STEP[first][1]
    start = (x - STEP[first][0], y - STEP[first][1])
    for _ in range(r.randrange(0, 9)):
        options = [d for d in STEP if d != REVERSE[facing] and 0 <= x + STEP[d][0] <= w and 0 <= y + STEP[d][1] <= h and
                   (x + STEP[d][0], y + STEP[d][1]) not in cells and (x + STEP[d][0], y + STEP[d][1]) != start]
        if not options or r.random() < 0.1:
            break
        out = r.choice(options + [facing] * 2) if facing in options else r.choice(options)
        cells[(x, y)] = (r.choice(QUEUE), REVERSE[facing] | out)
        facing = out
        x, y = x + STEP[out][0], y + STEP[out][1]
    if (x, y) in cells or (x, y) == start or not (0 <= x <= w and 0 <= y <= h):
        return first, cells
    end = r.random()
    if end < 0.5:
        cells[(x, y)] = (1, REVERSE[facing] | r.choice([0, 0, 0, 4, 0x40, 0x11, r.getrandbits(8)]))
    elif end < 0.75:
        cells[(x, y)] = (r.choice(QUEUE), REVERSE[facing])
    else:
        cells[(x, y)] = (r.choice([0, 5, 0x15, 2, -1]), r.choice([REVERSE[facing], r.getrandbits(8)]))
    return first, cells


def stops(rows, x, y, d, w, h, gate):
    """Whether the walk from (x, y) going d stops inside columns 0..w and rows 0..h (the start's step not taken: the caller checks)."""
    def get(x, y):
        at = x * 0x44
        row = rows[y]
        bits = row[at + 0x22] if gate and row[at + 2] == 2 else row[at + 0xc]
        return struct.unpack_from('<i', row, at + 8)[0], bits
    seen = set()
    while True:
        if not (0 <= x <= w and 0 <= y <= h) or (x, y, d) in seen:
            return False
        seen.add((x, y, d))
        kind, bits = get(x, y)
        stop = False
        if kind in (3, 9):
            if bin(bits & 0x55).count('1') == 2 and bits & 0x11 and bits & 0x44:
                d = bits ^ nibble(d)
        else:
            stop = True
        if kind != 9:
            if bin(bits).count('1') == 1:
                stop = True
            if kind == 1:
                bx, by = x - STEP.get(d, (0, 0))[0], y - STEP.get(d, (0, 0))[1]
                if not (0 <= bx <= w and 0 <= by <= h):
                    return False
                stop = True
        if stop:
            return True
        x, y = x + STEP.get(d, (0, 0))[0], y + STEP.get(d, (0, 0))[1]


def case(r):
    regions = dict(trackline.case(r).regions)
    w = max(struct.unpack('<i', regions[0x81d740])[0], 4) + 3   # (the last column and row the walk may use: those after have no cells)
    h = max(struct.unpack('<i', regions[0x81b1b0])[0], 4) + 3
    gate = r.choice([0, 0, 1])
    base = {y: bytes(regions[CELLS + ident(0, y) * 0x44]) for y in range(h + 2)}
    x0, y0 = r.randrange(w + 1), r.randrange(h + 1)
    for attempt in range(40):
        rows = {y: bytearray(row) for y, row in base.items()}

        def cell(x, y, kind, bits, owner=None):
            at = x * 0x44
            put(rows[y], at + 8, 'i', kind)
            rows[y][at + 0xc] = rows[y][at + 0x22] = bits
            if owner is not None:
                put(rows[y], at + 0x10, 'H', owner)

        first, line = queue(r, x0, y0, w, h)
        for (x, y), (kind, bits) in line.items():
            cell(x, y, kind, bits)
        d = first
        bits = r.choice([d] * 6 + [d | REVERSE[d]] * 2 + [0, r.getrandbits(8)]) if attempt < 39 else 0
        start_kind = r.choice([3, 3, 9, 1, 0x15, 0])
        nx, ny = x0 + STEP[d][0], y0 + STEP[d][1]
        same = struct.unpack_from('<H', rows[ny], nx * 0x44 + 0x10)[0] if 0 <= nx <= w and 0 <= ny <= h and r.random() < 0.5 else None
        cell(x0, y0, start_kind, bits, same)
        # the step from the entrance taken or not, as the model (Map_StepCell by the direction pair: trackline.py's, the real ones mostly)
        if bits in STEP:
            nx, ny = (x0 + v for v in struct.unpack_from('<ii', regions[0x7cdba0], PAIR[bits]))
        if bits not in STEP or not (0 <= nx <= w + 1 and 0 <= ny <= h + 1):
            moved = False
        else:
            kind = struct.unpack_from('<i', rows[ny], nx * 0x44 + 8)[0]
            moved = kind in (3, 9) and rows[ny][nx * 0x44 + 0x10:nx * 0x44 + 0x12] != rows[y0][x0 * 0x44 + 0x10:x0 * 0x44 + 0x12]
        if moved or bits == 0 or stops(rows, x0, y0, bits, w, h, gate):
            break
    for y, row in rows.items():
        regions[CELLS + ident(0, y) * 0x44] = row
    handle = ident(x0, y0) if r.random() < 0.97 else 0
    if handle == 0:   # (column 127 of row 0x1ffffff: the cell before the map's first, off the map)
        regions[CELLS] = bytearray(r.bytes(0x44))
        regions[CELLS][0xc] = regions[CELLS][0x22] = r.choice([0, 4])   # (its east step: the map's first cell)
    low = r.choice([0, 1, 4, 0x10, 0x40, 0x37, 0x80, 0xc4])
    thing = bytearray(r.bytes(0x40))
    put(thing, 0x36, 'H', handle)
    put(thing, 0x10, 'i', r.choice([0, 90, 180, 270] * 3 + [45, -90, 360, r.getrandbits(32) - (1 << 31)]))
    regions.update({THING + low: thing, 0x81b4cc: bytes([gate]), 0x81b740: r.bytes(0x100), 0x820a8c: r.bytes(4)})
    return Call(0x530120, [THING + low], regions, ret='b')


if __name__ == '__main__':
    B.main(case)

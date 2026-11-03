# A staff member sent to a random cell of its patrol region (0x506f30 Staff_GoToRandomPatrolCell, src/game/world/staff) against the original:
# navpath.py's person (the destination it is sent to by cell works when it is the nav's cell), the region small, whole, inverted or wild (no
# span of 0: that divides by zero in both). The rolls are worked out here from the seed (+0x1da708), so the map holds just the cells rolled:
# none of them type 1 but, mostly, one (at times one rolled late or twice), the person's destination; its Sprite_SetDestinationCell may fail
# (the block value limit), the next tries going on, and after the 30 tries thought 0x16 (Thoughts_Show's set-up as thoughtshow.py's).
# usage: staffwander.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import navpath  # noqa: E402
import objectblocks  # noqa: E402
import wctimage  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402
from movingperson import cell_xy  # noqa: E402

B = Bench(wctimage.merged(navpath.RANGES + [
    (0x4d8300, 0x4d8328), (0x4fa530, 0x4fa600), (0x506f30, 0x50703b), (0x50be80, 0x50c075), (0x516330, 0x516370), (0x536300, 0x536320)]),
    hooks={0x550ca0: _recorder(0, 0), 0x67a7d0: _recorder(1, 1, cdecl=True), 0x475550: _recorder(2, 1, cdecl=True),
           0x475a10: _recorder(3, 7, cdecl=True)})
MAP = objectblocks.MAP
POINTER = RAM + 0x60000
CELLS = MAP + 0x294   # (a cell id's: + id * 0x44; [0x8023a0] is this + 0x44)
SEED = MAP + 0x1da708


def rolls(seed, n):
    out = []
    for _ in range(n):
        seed = (seed * 0x19660d + 0x3c6ef35f) & 0xffffffff
        seed = (seed >> 13 | seed << 19) & 0xffffffff
        out.append((-seed & 0xffffffff) if seed >> 31 else seed)
    return out


def word(r):
    return r.choice([0, 1, 2, 0x7f, 0x80, 0x81, 0x4000, 0x4001, 0xffff, r.randrange(1, 0x4001), r.getrandbits(16)])


def region(r):
    """The corner words and the spans: small, whole, or any way (no span of 0)."""
    while True:
        kind = r.random()
        if kind < 0.5:   # (up to 6 by 6, in the map or over its edge)
            c, w = r.randrange(128), r.randrange(6)
            y, h = r.randrange(128), r.randrange(6)
            a = (y << 7 | c) + 1
            b = (min(y + h, 0x1ff) << 7 | min(c + w, 0x7f)) + 1
        elif kind < 0.65:
            a, b = 1, r.choice([0x4000, 0x4000, 0x3fff])
        elif kind < 0.75:
            a, b = 0, 0x4000 if r.random() < 0.5 else 1
        else:
            a, b = word(r), word(r)
        columns = (((b - 1) & 0x7f) - ((a - 1) & 0x7f) + 1) & 0xffffffff
        rows = (((b - 1) & 0xffffffff) >> 7) - (((a - 1) & 0xffffffff) >> 7) + 1 & 0xffffffff
        if columns and rows:
            return a, b, columns, rows


def set_cell(regions, r, cid, kind):
    """A rolled cell's type (+8), in objectblocks.py's rows when they hold it."""
    at = CELLS + cid * 0x44
    for base, block in regions.items():
        if isinstance(block, bytearray) and CELLS <= base < CELLS + 0x200000 and base <= at < base + len(block):
            put(block, at - base + 8, 'I', kind)
            return
    cell = bytearray(r.bytes(0x44))
    put(cell, 0, 'H', cid)
    put(cell, 8, 'I', kind)
    regions[at] = cell


def case(r):
    a, b, columns, rows = region(r)
    seed = r.getrandbits(32)
    tries = []
    values = rolls(seed, 60)
    for i in range(30):
        column = (values[2 * i] % columns + ((a - 1) & 0x7f)) & 0xffffffff
        row = (values[2 * i + 1] % rows + (((a - 1) & 0xffffffff) >> 7)) & 0xffffffff
        if column < 0x80 and row < 0x80:
            tries.append((row << 7) + column + 1)
    target = None
    if tries and r.random() < 0.85:
        target = r.choice([tries[-1], tries[0], r.choice(tries), r.choice(tries)])
    c = navpath.person_case(r, cell_xy(target) if target else None)
    regions = dict(c.regions)
    person = bytearray(regions[RAM]) + bytearray(r.bytes(0x20))
    put(person, 0x20a, 'HH', a, b)
    person[0xb4:0xb8] = struct.pack('<I', r.choice([0, 0, r.randrange(1, 64), r.getrandbits(32)]))
    last = r.choice([0, r.randrange(100), r.getrandbits(32), 0xffffffff - r.randrange(60)])
    put(person, 0xbc, 'I', last)
    regions[RAM] = person
    for cid in sorted(set(tries)):
        set_cell(regions, r, cid, 1 if cid == target else r.choice([0, 2, 3, 4, 9, 0x101, r.getrandbits(32) | 2]))
    clock = (last + r.choice([0, 1, r.randrange(80), r.getrandbits(32)])) & 0xffffffff
    regions[SEED] = struct.pack('<II', seed, clock)
    regions[POINTER] = struct.pack('<I', MAP)
    regions.update({
        LOG: log_region(), 0x7cf6ec: struct.pack('<I', POINTER),
        0x790ab0: bytes([r.choice([0, 0, 0, 0, 1, 2, 4, 0x10, 0x20, 0xe9, r.getrandbits(8)])]),
        0xfb34aa: bytes([r.choice([1, 1, 1, 0, 2, 3, r.getrandbits(8)])]),
        0xfb3b7c: struct.pack('<I', r.choice([0, 1, 1, 2, r.getrandbits(32)])),
    })
    return Call(0x506f30, [RAM], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

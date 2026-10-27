# A line cell removed (src/game/world/cellkind: TrackCell_Remove 0x53b280, recursive) against the original. lineends.py's world (the 9 x 9
# 0x28-byte cells about the cell in [0x7cf83c] = [0x80239c], cellmodel.py's world, and their 0x44-byte cells at [0x8023a0]) and
# cellapply.py's MapCell_ApplyModel scene; object.py's Scrap object (its clock in the same world) and cellkind.py's things table (handles 0,
# 3, 4 the object, 1 money.py's record as the world's money, 2 the park thing, 5-7 decoys), the 0x44-byte cells' first things (+0x24) among
# them. Agent C's unported 0x527ee0 (Path_CheckDeadEnds) a cdecl recorder (27 of 3, driver.cpp's kCellKindEnv).
# The cell (in the world) of the line kinds 0xb, 0xd, 0x10, 0x12, 0x19 mostly, 0xc / 0x11 (its owner's cell, of a line kind, removed
# instead) or others; its count +0x20 about 0; its owner and its neighbours' among a few ids nearby (the cells south, east and south-east
# owned by it now and then), their kinds the line ones, 0xc / 0x11 and others. a and b 0, -1, +-2, 1 or any (-1 and -1 together often),
# models 0, 1 or any. The line's start (0x7639e0 / 0x7639e4) on the cell often, its end (0x7639e8 / 0x7639ec) now and then; the flags
# 0x81b46c, 0x81d7a8, 0x785a2c, 0x8186d4 set or not, the prices 0x81aebc / 0x81b43c, the counter 0x7cdb98 and the cells' block values.
# usage: trackremove.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, _recorder, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import cellapply  # noqa: E402
import cellkind  # noqa: E402
import lineends as le  # noqa: E402
import object as objects  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

WORLD, TABLE, MAPC, ident = le.WORLD, le.TABLE, le.MAPC, le.ident
objects.WORLD = WORLD   # (the clock at + 0x2a0 / + 0x1da70c of the line's world)
B = Bench(cellapply.merged(le.RANGES + cellkind.RANGES + [(0x53b280, 0x53baa2)]), sines=256,
          hooks={**cellapply.HOOKS, **cellkind.HOOKS, 0x527ee0: _recorder(27, 3, cdecl=True)})
LINES = [0xb, 0xd, 0x10, 0x12, 0x19]
STEPS = [0, 0, -1, -1, 1, 2, 2, -2, -2, 3]
PRICES = [0, 1, 50, 99, 100, 101, 0x80000000]


def put_cell(regions, x, y, x0, offset, fmt, value):
    """A field of the block's cell (x, y) (its row's region starting at column x0)."""
    row = TABLE + ident(x0, y) * 0x28
    data = bytearray(regions[row])
    put(data, (x - x0) * 0x28 + offset, fmt, value)
    regions[row] = data


def case(r):
    regions = dict(objects.case(r).regions)
    regions.update(cellapply.case(r, regions[LOG]).regions)
    regions.update({0x80239c: struct.pack('<I', WORLD), 0x7cf83c: struct.pack('<I', WORLD), 0x8023a0: struct.pack('<I', MAPC)})
    cellkind.things(r, regions, WORLD)
    x, y = le.spot(r), le.spot(r)
    h = ident(x, y)
    le.block(r, regions, x, y)
    x0 = max(0, x - 4)
    spots = [(min(127, max(0, x + dx)), min(127, max(0, y + dy))) for dx, dy in ((1, 0), (-1, 1), (2, -2), (0, 2), (-2, 0))]
    owners = [ident(*s) for s in spots] + [h, h, h]
    for yy in range(max(0, y - 4), min(127, y + 4) + 1):
        row, mapc = bytearray(regions[TABLE + ident(x0, yy) * 0x28]), bytearray(regions[MAPC + (ident(x0, yy) - 1) * 0x44])
        for i, xx in enumerate(range(x0, min(127, x + 4) + 1)):
            put(row, i * 0x28 + 8, 'i', r.choice(le.KINDS + LINES + [0xc, 0x11]))
            put(row, i * 0x28 + 0x10, 'H', h if (xx - x, yy - y) in ((1, 0), (0, 1), (1, 1)) and r.random() < 0.5 else r.choice(owners))
            put(mapc, i * 0x44 + 0x24, 'H', r.choice([0, 3, 4, 5, 6, 7]))
            regions[WORLD + 0x1b02d8 + cellkind.block(ident(xx, yy)) * 4] = r.bytes(4)
        regions[TABLE + ident(x0, yy) * 0x28], regions[MAPC + (ident(x0, yy) - 1) * 0x44] = row, mapc
    kind = r.choice(LINES * 3 + [0xc, 0x11, 0, 1, 3, r.getrandbits(32)])
    put_cell(regions, x, y, x0, 8, 'I', kind)
    put_cell(regions, x, y, x0, 0xd, 'B', r.choice(le.SIDES * 2 + [0x11, 0x44, 0x14, 0x41, 0x55, 0x50, 0x05, r.getrandbits(8)]))
    put_cell(regions, x, y, x0, 0xe, 'H', r.choice([0, 0x10, 0x10, r.getrandbits(16)]))
    put_cell(regions, x, y, x0, 0x20, 'h', r.choice([0, 1, 1, 2, -1, r.getrandbits(16) - 0x8000]))
    if kind in (0xc, 0x11):   # (its owner's cell, removed instead, of a line kind: not 0xc / 0x11 again)
        ox, oy = r.choice(spots)
        put_cell(regions, x, y, x0, 0x10, 'H', ident(ox, oy))
        put_cell(regions, ox, oy, x0, 8, 'I', r.choice(LINES))
    start = r.choice([(x, y), (x, y), (x, y), r.choice(spots), (r.randrange(128), r.randrange(128)), (-1, -1)])
    end = r.choice([(x, y), r.choice(spots), (r.randrange(128), r.randrange(128)), (-1, -1)])
    regions.update({0x7639e0: struct.pack('<4i', *start, *end), 0x81b46c: struct.pack('<I', r.choice([0, 0, 1])),
                    0x81d7a8: struct.pack('<I', r.choice([0, 0, 0, 1])), 0x785a2c: struct.pack('<I', r.choice([0, 0, 1])),
                    0x8186d4: bytes([r.choice([0, 0, 1])]), 0x81aebc: struct.pack('<I', r.choice(PRICES + [r.getrandbits(32)])),
                    0x81b43c: struct.pack('<I', r.choice(PRICES + [r.getrandbits(32)])), 0x7cdb98: r.bytes(4), 0x820ad0: struct.pack('<I', 0)})
    a, b = r.choice([(-1, -1), (-1, -1), (0, 0)] + [(r.choice(STEPS), r.choice(STEPS)) for _ in range(4)] + [(r.getrandbits(32), r.getrandbits(32))])
    return Call(0x53b280, [TABLE + h * 0x28, a, b, r.choice([0, 1, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

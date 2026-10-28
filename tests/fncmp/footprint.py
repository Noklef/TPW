# The footprint of the object being placed (src/game/world/placeobject: PlaceObject_Footprint 0x528a70) against the original: the
# definition of the id at [0x8186e0] (gObjectDefs 0x7890a0, found through its cache or its list) with a shape of 1-5 columns and rows (now
# and then none), its cells' types mostly 0, 1, 4, 9, 0xa, 0xb, 0x10 and 0x17 (a 9 and an 0xa put in half the time) and their direction
# bytes 1, 4, 0x10, 0x40 or others, its offsets +0xa4 - +0xc0 small; (x, y) on the map mostly (near its edges now and then, off it
# rarely), the angle 0, 0x5a, 0xb4, 0x10e or another, place and check either way, the centring flag [0x81af7c], the flag [0x8187f8] and
# the dwords it guards (0x81b0cc, 0x81ecac: 0-3, -1), sound on or off. The map [0x7cf83c]: the cells within 14 of (x, y), the corner
# cells (0, 0) - (1, 1) and the cell of (-1, -1) (where an unset 0x818c20 / 24 points), each one's first word its own id, and their
# 0x28-byte entries ([0x80239c], Map_StepCell's map, the same); the directions' steps (0x7cdba0) as deadends.py lays them out. The calls the port makes through PlaceFootprintEnv are
# recorders (driver.cpp): 0x532fc0 thiscall 9 of 3 (answering from a table, mostly not 0), MapCell_Clear 28 of 2, 0x536780 10 of 0,
# 0x4de1f0 11 of 0, MapCell_LinkPath 12 of 4, PlaceObject_Create cdecl 13 of 1 (answering an object whose +5 / +7 are a cell),
# FocusCell_Update cdecl 14 of 1, Sound_PlayAt cdecl 15 of 6; TrackSquare_Check runs (its 0x532fc0 the same recorder).
# (The direction TrackSquare_Check gets is unset in the original with no rows, or with no columns and no placing: those cases have no
# track kind (0x81ecac 0).)
# usage: footprint.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put, set_return  # noqa: E402
from deadends import DIRECTIONS  # noqa: E402

B = Bench([(0x528a70, 0x529d04), (0x5286a0, 0x528a70), (0x412e90, 0x412f00), (0x4d8300, 0x4d8340), (0x4d8c00, 0x4d8c60),
           (0x4d96f0, 0x4d9a70), (0x522700, 0x522870), (0x536380, 0x5363a0), (0x539210, 0x539230), (0x53a510, 0x53a550),
           (0x52fab0, 0x52fb10), (0x67a830, 0x67a860), (0x5da3c0, 0x5da3c1)],
          hooks={0x532fc0: _recorder(9, 3), 0x5367a0: _recorder(28, 2), 0x536780: _recorder(10, 0), 0x4de1f0: _recorder(11, 0),
                 0x5348d0: _recorder(12, 4), 0x529e10: _recorder(13, 1, cdecl=True), 0x5276d0: _recorder(14, 1, cdecl=True),
                 0x51bfc0: _recorder(15, 6, cdecl=True)})
MAP = 0x31000000
DEFS, DEF, SHAPE, OBJ, TABLE = 0x7890a0, RAM, RAM + 0x200, RAM + 0x1000, RAM + 0x1100
TYPES = [0, 0, 0, 1, 1, 4, 4, 9, 0xa, 0xb, 0x10, 0x17, 2, 3, 0x18, 0xffffffff]
DIRS = [1, 4, 0x10, 0x40, 1, 4, 0x10, 0x40, 0, 2, 0x80, 0x20]
CELL_TYPES = [1, 1, 1, 4, 9, 0xa, 0, 3, 0xb]
SPAN = 14


def cell_id(x, y):
    return ((y << 7) + x + 1) & 0xffff


def cells(r, xs, y):
    """The 0x44-byte cells of row y over columns xs, each one's first word its own id."""
    out = bytearray(r.bytes(0x44 * len(xs)))
    for i, x in enumerate(xs):
        c = 0x44 * i
        put(out, c, 'H', cell_id(x, y))
        put(out, c + 8, 'I', r.choice(CELL_TYPES))
        out[c + 0xd] = r.choice(DIRS)
    return out


def case(r):
    log = log_region()
    regions = {LOG: log, 0x7cf83c: struct.pack('<I', MAP), 0x80239c: struct.pack('<I', MAP), 0x7cdba0: DIRECTIONS}
    x, y = r.randrange(128), r.randrange(128)
    if r.random() < 0.2:
        x, y = r.choice([0, 1, 2, 125, 126, 127, x]), r.choice([0, 1, 2, 125, 126, 127, y])
    if r.random() < 0.03:
        x, y = r.choice([-3, -1, 128, 130, x]), r.choice([-3, -1, 128, 130, y])
    # (the map: the cells within SPAN of (x, y), the corners (0, 0) - (1, 1), the cell of (-1, -1); their entries)
    lo, hi = max(0, y - SPAN), min(127, y + SPAN)
    clo, chi = max(0, x - SPAN), min(127, x + SPAN)
    xs = list(range(clo, chi + 1))
    for cy in range(lo, hi + 1):
        regions[MAP + 0x294 + cell_id(clo, cy) * 0x44] = cells(r, xs, cy)
        regions[MAP + 0x1102b0 + cell_id(clo, cy) * 0x28] = r.bytes(0x28 * len(xs))
    for cx, cy in ((0, 0), (1, 0), (0, 1), (1, 1), (-1, -1)):
        if not (clo <= cx <= chi and lo <= cy <= hi):
            regions[MAP + 0x294 + cell_id(cx, cy) * 0x44] = cells(r, [cx], cy)
            regions[MAP + 0x1102b0 + cell_id(cx, cy) * 0x28] = r.bytes(0x28)
    # (the definition, its shape)
    oid = r.getrandbits(16)
    dtab = bytearray(r.bytes(0x740))
    put(dtab, 0, 'I', DEF)
    put(dtab, 0x528, 'i', 1)
    put(dtab, 0x730, 'HxxI', oid if r.random() < 0.5 else oid ^ r.randrange(1, 0x10000), DEF)
    regions[DEFS] = dtab
    regions[0x8186e0] = struct.pack('<H', oid)
    d = bytearray(r.bytes(0x100))
    put(d, 0, 'H', oid)
    put(d, 0x18, 'I', SHAPE)
    for off in range(0xa4, 0xc4, 4):
        put(d, off, 'i', r.randrange(-3, 4))
    regions[DEF] = d
    cols, rows = r.choice([1, 1, 2, 2, 3, 3, 4, 5, 0, -1]), r.choice([1, 1, 2, 2, 3, 3, 4, 5, 0, -1])
    shape = bytearray(r.bytes(8 + 0xa0 * 5))
    put(shape, 0, 'ii', cols, rows)
    spots = [(c, w) for w in range(max(rows, 0)) for c in range(max(cols, 0))]
    many = r.random() < 0.25   # (mostly type 4: the sound's counts 4-9)
    for c, w in spots:
        put(shape, 8 + 0xa0 * w + 8 * c, 'I', 4 if many and r.random() < 0.85 else r.choice(TYPES))
        shape[8 + 0xa0 * w + 8 * c + 4] = r.choice(DIRS)
    if len(spots) >= 2 and r.random() < 0.5:
        for (c, w), t in zip(r.sample(spots, 2), (9, 0xa)):
            put(shape, 8 + 0xa0 * w + 8 * c, 'I', t)
    regions[SHAPE] = shape
    place = r.choice([0, 1, 1, r.getrandbits(32)])
    check = r.choice([0, 0, 1])
    angle = r.choice([0, 0x5a, 0xb4, 0x10e, 0, 0x5a, 0xb4, 0x10e, 0x2d, 0xffffffff])
    # (the globals read: the centring flag, the flag and the dwords it guards, sound, the thing; those written)
    regions[0x81af7c] = bytes([r.choice([0, 0, 1])])
    regions[0x8187f8] = struct.pack('<I', r.choice([0, 0, 0, 1]))
    kind = r.choice([0, 1, 2, 3, 0xffffffff, 1, 2])
    if rows <= 0 or (cols <= 0 and not place):
        kind = 0
    regions[0x81b0cc] = struct.pack('<I', r.choice([0, 1, 2, 3, 0xffffffff]))
    regions[0x81ecac] = struct.pack('<I', kind)
    regions[0x81b4cc] = struct.pack('<I', r.choice([0, 1]))
    regions[0x785a2c] = struct.pack('<I', r.choice([0, 0, 1]))
    regions[0x803a2c] = r.bytes(4)
    regions[0x8186ac] = r.bytes(4)
    regions[0x818c18] = r.bytes(0x3c)
    regions[0x7639e0] = r.bytes(8)
    regions[0x816d50] = r.bytes(4)
    # (the made object's cell; 0x532fc0's answers)
    obj = bytearray(r.bytes(0x10))
    obj[5], obj[7] = r.randrange(128), r.randrange(128)
    regions[OBJ] = obj
    set_return(log, 13, OBJ)
    regions[TABLE] = struct.pack('<48I', *[r.choice([1, 1, 1, 1, 0, 0x100, 0x80000000]) for _ in range(48)])
    answers(log, 9, TABLE)
    return Call(0x528a70, [x & 0xffffffff, y & 0xffffffff, r.choice([4, r.getrandbits(32)]), angle, place, check], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

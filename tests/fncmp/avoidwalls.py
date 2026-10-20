# avoid_walls' steering (0x50e4a0 SteerAvoidWalls_Steer, src/game/world/steering) against the original, with the callees run: a nav whose cell
# (pos >> 16) is about the map's edges or anywhere on it (at times off it), its fraction near the cell's sides; a velocity (Nav_PredictPos(1.0)) of
# any direction, small or large (0 at times: the length's 0); the radius about a cell's half (0 at times: the corner's divide faults in both);
# the mode +0xb4 0-3; the nine cells about it set up for every step out of them as cellstep.py's world() does (blocked or open).
# usage: avoidwalls.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import cellstep  # noqa: E402
import wctimage  # noqa: E402
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench(wctimage.merged([
    (0x4d8750, 0x4d8b29), (0x4d0af0, 0x4d0b06), (0x522700, 0x5228b5), (0x536300, 0x536484), (0x53ad20, 0x53aefc), (0x4dd4e0, 0x4dd510),
    (0x412e90, 0x412f00), (0x42a440, 0x42a4f0), (0x436840, 0x436870), (0x436880, 0x4368d0), (0x44b220, 0x44b280), (0x549d30, 0x549dc0),
    (0x55a070, 0x55a0c0), (0x55a360, 0x55a3a0), (0x55a570, 0x55a5b0),   # (cellstep.py's: MapCell_StepBlocked and Ride_IsBusy)
    (0x4d49a0, 0x4d49e0), (0x4d8300, 0x4d8330), (0x50cba0, 0x50cdc0), (0x50df70, 0x50e190), (0x50e4a0, 0x50ece0), (0x50ece0, 0x50ed20),
    (0x50f870, 0x50f8f0), (0x5104b0, 0x510510), (0x510870, 0x5108a0), (0x5da3c0, 0x5da3c1)]))
NAV = RAM + 0x50000   # (clear of the world's regions)
OUT = NAV + 0x200


def coord(r, cell):
    """A 16.16 coordinate in the cell: its fraction anywhere, or near a side."""
    frac = r.choice([r.randrange(0x10000), r.randrange(0x2000), 0x10000 - r.randrange(1, 0x2000), 0x8000])
    return ((cell << 16) + frac) & 0xffffffff


def cell(x, y):
    """The address of the cell (x, y) (bytes) in world()'s map."""
    return cellstep.WORLD + 0x294 + (((y & 0xff) << 7) + (x & 0xff) + 1) * 0x44


def poke(regions, address, fmt, *values):
    """Writes into world()'s pages (the cell's page is laid out)."""
    data = struct.pack('<' + fmt, *values)
    for i, b in enumerate(data):
        regions[(address + i) & ~0xfff][(address + i) & 0xfff] = b


def case(r):
    edge = [0, 1, 0x7e, 0x7f]
    cx = r.choice(edge + [r.randrange(2, 0x7e)] * 6 + [r.randrange(0x80, 0x10000)])
    cy = r.choice(edge + [r.randrange(2, 0x7e)] * 6 + [r.randrange(0x80, 0x10000)])
    mode = r.choice([0, 1, 1, 2, 2, 3])
    steps = []
    for x in (cx - 1, cx, cx + 1):
        for y in (cy - 1, cy, cy + 1):
            for d in range(4):
                target, source = cellstep.target_of(x & 0xff, y & 0xff, d)
                steps.append((source, target, d))
    regions = cellstep.world(r, mode, steps)
    nav = bytearray(r.bytes(0x208))   # (the out pair at +0x200)
    radius = r.choice([0x8000, 0x4000, 0xc000, 0x10000, r.randrange(1, 0x18000), 0])
    span = r.choice([0x4000, 0x10000, 0x20000, 0x80000])
    vel = [r.choice([r.randrange(-span, span + 1), 0, span, -span]) for _ in range(2)]
    if r.random() < 0.04:
        vel = [0, 0]
    px, py = coord(r, cx), coord(r, cy)
    if r.random() < 0.4 and cx < 0x7f and cy < 0x7f and cx and cy:
        # a corner: the nine cells open (type 1 into type 1), one diagonal of type 3 whose steps into the two cells beside it are blocked (their
        # way-back bits clear, at times set: open), the nav heading for it from inside its quarter of the cell
        sx, sy = r.choice([-1, 1]), r.choice([-1, 1])
        for x in (cx - 1, cx, cx + 1):
            for y in (cy - 1, cy, cy + 1):
                poke(regions, cell(x, y) + 8, 'I', 1)
                poke(regions, cell(x, y) + 0xc, 'B', 0xff)
        poke(regions, cell(cx + sx, cy + sy) + 8, 'I', 3)
        for x, y in ((cx + sx, cy), (cx, cy + sy)):
            poke(regions, cell(x, y) + 0xc, 'B', r.choice([0, 0, 0, 0xff]))
        radius = r.choice([0x2000, 0x4000, 0x6000, r.randrange(1, 0x8000)])
        quarter = lambda c, s: ((c << 16) + (0x8000 + r.randrange(0, 0x6000) if s > 0 else 0x8000 - r.randrange(0, 0x6000)))
        px, py = quarter(cx, sx), quarter(cy, sy)
        speed = r.choice([0x2000, 0x4000, 0x8000, 0x10000, 0x20000])
        vel = [sx * r.randrange(speed // 4, speed + 1), sy * r.randrange(speed // 4, speed + 1)]
    struct.pack_into('<IIIii', nav, 4, radius, px, py, *vel)
    struct.pack_into('<I', nav, 0xb4, mode & 0xffffffff)
    regions[NAV] = nav
    assert not any(a < NAV + 0x208 and a + len(v) > NAV for a, v in regions.items() if a != NAV)
    return Call(0x50e4a0, [RAM, r.choice([OUT, OUT, NAV + 8, NAV + 0x10]), NAV], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

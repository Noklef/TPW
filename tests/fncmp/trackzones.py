# The track's section records placed along it by zone (0x4402d0, src/game/coaster/trackmesh) against the original: the current coaster
# [0x792218] as coaster.py's FrameAt one (twelve nodes over two rings of spline nodes, their +0 any sign), its length (+0xa8) mostly short
# (a few quarter steps; the frame is costly in the emulator), its build (+4) with 1-4 zone boxes (+0x78, count +0x7c: the t range about 0,
# the yaw / pitch / roll ranges parts of +-pi so that the zone changes along the track, +0x20 one of four zones or 0, +0x24 / +0x28 a
# pair of them for the transitions) and the box flag +0x74 bit 0 mostly; each zone's +0x14 a blend length (positive, 0 or negative; no
# NaN: the spline value at the position plus a NaN length walks the ring of spline nodes forever, in the original too, as does a
# position past the ring's last distance, so the length stays below 7.25 + 1.5 < 9). The track [0x790fe4] (its +0x38-+0x58 and orientation +0x3c written), the fixed records 0x7925e8 / 0x7923f0 and the
# records from 0x792600 (room for 0x100).
# usage: trackzones.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import coaster  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x438a70, 0x438ac0), (0x43c080, 0x43c170), (0x43e1b0, 0x43e490), (0x43e590, 0x43e990), (0x43e9c0, 0x43eaa0), (0x4402d0, 0x440ae0),
           (0x46f210, 0x46f250)], sines=256)
X = RAM + 0x80000
BUILD, BOXES, ZONES, TRACK = X, X + 0x200, X + 0x400, X + 0x600


def angle_range(r):
    k = r.random()
    if k < 0.35:
        return -4.0, 4.0
    lo = r.uniform(-3.5, 2.5)
    return lo, lo + r.choice([r.uniform(0.2, 3.0), r.uniform(1.0, 6.0)])


def case(r):
    regions = {}
    nodes = bytearray(r.bytes(12 * 0x38))
    for k in range(12):
        put(nodes, k * 0x38 + 0x18, 'I', r.f(4))
        s = min(max(0, k - r.randrange(4)), coaster.RING - 2)
        put(nodes, k * 0x38 + 0x2c, 'II', coaster.SPL + s * 0x60, coaster.SPL + (coaster.RING + s) * 0x60)
    spl, pieces = coaster.spline_nodes(r, 2 * coaster.RING, ring=True)
    for k in range(2 * coaster.RING):   # (the spline nodes' +0: small, or any sign for the unsigned read)
        put(spl, k * 0x60, 'I', r.choice([k, r.randrange(20), r.getrandbits(32)]))
    regions[coaster.NODES], regions[coaster.SPL], regions[coaster.PIECES] = nodes, spl, pieces
    c = bytearray(r.bytes(0x100))
    put(c, 4, 'I', BUILD)
    c[0x3c] = r.choice([0, 8, r.getrandbits(8)])
    put(c, 0x9c, 'I', coaster.NODES)
    put(c, 0xa8, 'f', r.choice([r.uniform(0, 1.5), r.uniform(0.5, 3.0), r.uniform(0.5, 3.0), r.uniform(3.0, 7.0), 0.0, -1.0]))
    regions[coaster.COAST] = c
    regions[0x792218] = struct.pack('<I', coaster.COAST)

    zones = bytearray(r.bytes(0x100))
    for j in range(4):
        put(zones, 0x20 * j + 0x14, 'f', r.choice([r.uniform(0.1, 1.5), r.uniform(0.1, 1.5), 0.0, r.uniform(-1.5, -0.1), r.uniform(-1.5, -0.1)]))
    regions[ZONES] = zones
    pick = [ZONES + 0x20 * j for j in range(4)] + [0]
    count = r.choice([1, 2, 3, 4, 4])
    boxes = bytearray(r.bytes(0x2c * 5))
    for k in range(count):
        lo = r.choice([-1.0, -1.0, r.uniform(-1, 0.5)])
        put(boxes, k * 0x2c, 'ff', lo, lo + r.choice([2.0, r.uniform(0, 2)]))
        for j in range(1, 4):
            put(boxes, k * 0x2c + 8 * j, 'ff', *angle_range(r))
        put(boxes, k * 0x2c + 0x20, 'III', r.choice(pick[:4] + [0] if k else pick[:4]), r.choice(pick), r.choice(pick))
    regions[BOXES] = boxes
    build = bytearray(r.bytes(0x100))
    put(build, 0x74, 'I', r.choice([1, 1, 1, 0, r.getrandbits(32)]))
    put(build, 0x78, 'II', BOXES, count)
    regions[BUILD] = build

    regions[TRACK] = r.bytes(0x80)
    regions[0x790fe4] = struct.pack('<I', TRACK)
    regions[0x7923f0] = r.bytes(0x10)
    regions[0x7925e8] = r.bytes(0x18) + r.bytes(0x1000)   # (0x7925e8 to 0x792600, then the records)
    return Call(0x4402d0, [], regions)


if __name__ == '__main__':
    B.main(case)

# follow_path's steering (0x50ed10 SteerFollowPath_Steer, src/game/world/steering) against the original, with the callees run (Nav_PathTo's
# route search on navpath.py's box of cells): the nav at a cell of the box (no wild top byte: the block value is read at pos >> 20), the block
# value there (the table at [0x7cf83c] + 0x1b02d8) about the nav's +0x48; the path done or not (+0x60), 0-5 points (+0x54), the current
# one (+0x5c) any of them (the last mostly), the count +0x58 about it, the waypoints (+0x64) and the target (+0x4c) within a few radii of
# pos (passed or reached or not), the lengths (+0x8c, +0xa4, +0xa8) and the total +0xa0 (0 at times) about them, the history +0xb0 (0-15
# of its low bits set), the radius, velocity and max_speed about 1.0.
# usage: followpath.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import navpath  # noqa: E402
import wctimage  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench(wctimage.merged(navpath.RANGES + [
    (0x4d49a0, 0x4d49e0), (0x4d8cd0, 0x4d8cf0), (0x50cd80, 0x50cdc0), (0x50df70, 0x50dfd0), (0x50ed10, 0x50f1a0), (0x50f1a0, 0x50f1f0),
    (0x50fd40, 0x50fdf0), (0x67a830, 0x67a858), (0x5da3c0, 0x5da3c1)]))
OUT = RAM + 0x48400
BLOCKS = RAM + 0x700000   # the world [0x7cf83c] when the box sets none


def near(r, centre, reach):
    return (centre + r.randrange(-reach, reach + 1)) & 0xffffffff


def case(r):
    c = navpath.nav_case(r, 0x50f8e0)
    regions = dict(c.regions)
    nav = bytearray(regions[navpath.NAV])
    x, y = (v & 0xffffff for v in struct.unpack_from('<II', nav, 8))
    put(nav, 8, 'II', x, y)
    target = struct.unpack_from('<II', regions[navpath.TARGET])
    radius = r.choice([0x8000, 0x4000, 0x10000, r.randrange(0x1000, 0x20000)])
    reach = r.choice([radius, radius * 2, radius * 4, 0x40000])
    count = r.choice([0, 1, 2, 3, 4, 5, 5, 5])
    point = r.choice([count - 1] * 3 + list(range(max(count, 1)))) if count else r.randrange(5)
    put(nav, 4, 'I', radius)
    put(nav, 0x10, 'ii', r.randrange(-0x20000, 0x20001), r.randrange(-0x20000, 0x20001))
    put(nav, 0x1c, 'I', r.choice([0x10000, 0x8000, 0x3333, r.randrange(1, 0x40000)]))
    put(nav, 0x4c, 'II', *(target if r.random() < 0.3 else (near(r, x, reach), near(r, y, reach))))
    put(nav, 0x54, 'III', count, r.choice([count, count, count + 1, point + 1, r.randrange(6)]), point)
    nav[0x60] = r.choice([0, 0, 0, 1, r.getrandbits(8)])
    for i in range(5):
        put(nav, 0x64 + 8 * i, 'II', near(r, x, reach), near(r, y, reach))
    lengths = [r.choice([radius, 0x10000, r.randrange(0, 0x80000)]) for _ in range(5)]
    put(nav, 0x8c, 'IIIII', *lengths)
    left = sum(lengths) & 0xffffffff
    put(nav, 0xa0, 'III', r.choice([0, left, left * 2, left // 2, r.randrange(1, 0x100000)]) & 0xffffffff, r.choice([left // 2, lengths[0], 0]),
        r.choice([0, radius, r.randrange(0, 0x40000)]))
    bits = r.sample(range(15), r.choice([0, 3, 5, 6, 7, 9, 12, 15]))
    put(nav, 0xb0, 'I', sum(1 << b for b in bits) | (r.getrandbits(17) << 15 if r.random() < 0.3 else 0))
    regions[navpath.NAV] = nav
    world = struct.unpack_from('<I', regions[0x7cf83c])[0] if 0x7cf83c in regions else BLOCKS
    regions[0x7cf83c] = struct.pack('<I', world)
    stamp = r.choice([0, 5, 0xffffffff, r.getrandbits(32)])
    blocks = bytearray(r.bytes(33 * 33 * 4))
    bx, by = x >> 20, y >> 20
    put(blocks, (by * 33 + bx) * 4, 'I', r.choice([stamp, stamp, stamp + 1, stamp - 1, r.getrandbits(32)]) & 0xffffffff)
    put(nav, 0x48, 'I', stamp)
    regions[world + 0x1b02d8] = blocks
    regions[OUT] = r.bytes(8)
    return Call(0x50ed10, [RAM, r.choice([OUT, OUT, navpath.NAV + 0x10, navpath.NAV + 8]), navpath.NAV], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

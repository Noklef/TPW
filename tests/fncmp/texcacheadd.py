# The texture cache's new surfaces (src/game/gfx/texcache: 0x58d1d0) against the original: sides of the pool's nine (and a few others,
# index -1), formats 0-7, levels -1-3 (and any); the side's count 0-0x100 (a few negative) and its budget 0-30 areas give or take (or
# any, only when a surface is made); the render device's slot 4 (a recorder) answering a surface or 0 (only when the area is positive: else the turns never end), the
# surface's slot 0 succeeding or failing (slot 9 deleting it). The entries come from the bench heap; AnimTextures_Set copies the result.
# usage: texcacheadd.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x58d1d0, 0x58d377), (0x5848b0, 0x584a21), (0x590fd0, 0x590fe7), (0x5906e0, 0x590741), (0x67acc0, 0x67acc1)])
CACHE, DEV, DVT, SURF, SVT, MADE = RAM + 0x400, RAM + 0x2800, RAM + 0x2840, RAM + 0x2900, RAM + 0x2940, RAM + 0x10
SIDES = [0x100, 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1]
SIZES = [2, 2, 2, 4, 0x100, 0x80, 0x40, 0x20]   # (0x7022d8)


def s32(x):
    x &= 0xffffffff
    return x - (x >> 31 << 32)


def case(r):
    log = log_region()
    side = r.choice(SIDES) if r.random() > 0.05 else r.choice([0, 3, 0x30, 0x200, -4])
    index = SIDES.index(side) if side in SIDES else -1
    fmt = r.randrange(8)
    levels = r.choice([0, 1, 2, 3, -1, r.getrandbits(32) - (1 << 31)])
    area = s32(SIZES[fmt] * side * side)
    if levels > 1:
        area = s32(area * 4) // 3 if s32(area * 4) >= 0 else -(-s32(area * 4) // 3)
    cache = bytearray(r.bytes(0x2400))
    count = r.choice([0, 0, 1, 5, 0xf0, 0xfe, 0xff, 0x100, r.randrange(0x100)]) if r.random() > 0.03 else r.choice([-1, -3])
    put(cache, 0x1800 + 4 * index, 'i', count)
    made = r.random() > 0.3 or area <= 0
    k = r.choice([0, 1, 2, 3, 5, r.randrange(31)])
    budget = area * k + r.choice([0, 0, -1, 1, area // 2]) if r.random() > 0.05 or not made else r.getrandbits(32) - (1 << 31)
    put(cache, 0x1860 + 4 * index, 'i', max(min(budget, 0x7fffffff), -0x80000000))
    set_return(log, 4, SURF if made else 0)
    if r.random() < 0.2:
        set_return(log, 0, 0)
    regions = {RAM: r.bytes(0x400), CACHE: cache, HEAP: heap(0x2100), LOG: log, 0x8c4b70: r.bytes(0x1840),
               0x8bd554: struct.pack('<I', fmt), 0x8c4898: r.bytes(4), 0x8bd52c: struct.pack('<I', DEV),
               DEV: struct.pack('<I', DVT) + r.bytes(0xc), DVT: vtable({}, 8), SURF: struct.pack('<I', SVT) + r.bytes(0xc),
               SVT: vtable({0: 4, 9: 1}, 12)}
    return Call(0x58d1d0, [CACHE, side & 0xffffffff, MADE, r.getrandbits(32), levels & 0xffffffff], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

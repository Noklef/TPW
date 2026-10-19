# Format_BuildChannelTable (0x591270, src/game/gfx/color) against the original: the table of 256 records of four channel values for four masks (the usual 565, 555, 4444,
# 8888 and BGR masks, empty masks, masks with gaps, wider than 8 bits, a random word), the rest of the by-value format (a DDPIXELFORMAT: size, flags, FourCC, bit count, then red, green, blue, alpha) random.
# Format_ConvertPixels (0x575160): 4- or 3-byte pixels (alpha bytes 0, 0xff, about 0xf0 and any) through a random table, flipped or not, dst
# steps of 4, 2, 3, 0 or -4, now and then in place; sizes about the count * 0.05 limit, negative ones and (rarely) a zero width (faults).
# usage: channel_table.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x591270, 0x591400), (0x575160, 0x5755e3), (0x67a830, 0x67a860)])
SETS = [(0xf800, 0x7e0, 0x1f, 0), (0x7c00, 0x3e0, 0x1f, 0x8000), (0xf00, 0xf0, 0xf, 0xf000), (0xff0000, 0xff00, 0xff, 0xff000000), (0xff, 0xff00, 0xff0000, 0xff000000),
        (0, 0, 0, 0), (0xffffffff, 1, 0x80000000, 0x55555555)]


def mask(r):
    return r.choice([0, 1, 0xff, 0x1ff, 0xffffffff, r.getrandbits(32), r.getrandbits(32) & r.getrandbits(32), (0xff << r.randrange(25)) & 0xffffffff, 0x3 << r.randrange(31)])


def convert(r):
    w = r.choice([1, 2, 3, 4, 5, 8, -1, -2]) if r.random() > 0.02 else 0
    h = r.choice([1, 2, 3, 4, 5, 6, 0, -1, -3])
    if r.random() < 0.3:
        w, h = r.choice([(5, 4), (4, 5), (8, 5), (5, 8), (4, 4), (3, 7), (-4, -5), (8, 6)])
    bpp = r.choice([4, 4, 3, 3, 2])
    n = max(0, w * h)
    src = bytearray(r.bytes(n * 4 + 8))
    for i in range(3, len(src), 4 if bpp == 4 else 3):
        src[i] = r.choice([0, 0xff, 0xff, 0xf0, 0xef, 1, r.getrandbits(8)])
    step = r.choice([4, 4, 2, 3, 0, -4])
    DST, SRC, TABLE = RAM + 0x200, RAM + 0x400, RAM + 0x1000
    regions = {RAM: r.bytes(0x400), SRC: bytes(src), TABLE: r.bytes(0x1000), 0x87a25c: r.bytes(4)}
    dst = DST
    if r.random() < 0.15:
        dst, step = SRC, r.choice([bpp, 4])
    return Call(0x575160, [dst, SRC, w & 0xffffffff, h & 0xffffffff, TABLE, step & 0xffffffff, bpp, r.getrandbits(32)], regions, conv='c', ret='a')


def case(r):
    if r.deal([0x591270, 0x575160]) == 0x575160:
        return convert(r)
    masks = list(r.choice(SETS)) if r.random() < 0.6 else [mask(r) for _ in range(4)]
    args = [RAM] + [r.getrandbits(32) for _ in range(3)] + masks + [r.getrandbits(32)]
    return Call(0x591270, args, {RAM: bytes(0x1000)}, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)

# The cseg run-length sprite blitter (src/game/gfx/spriterle: 0x6e73c0) against the original: sprites of runs of every length (1-127
# copied, 1-128 skipped) at every alignment, inside the clip and across each of its sides, with the row and column flips (flags 2, 1)
# and the two ghost blends (flags 8, 0x10: a 0x400-byte table, the blended side's values kept below 4); now and then a zero size or a
# position far off. The parameters (0xfaa4d8) are the case's own.
# usage: spriterle.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x6e73c0, 0x6ebfc6)])
PARAMS, SPRITE, DATA, GHOST, BITS = 0xfaa4d8, RAM, RAM + 0x10, RAM + 0x8000, RAM + 0x10000


def rows(r, width, height, small):
    out = bytearray()
    longest = r.choice([8, 40, 128])
    for _ in range(height):
        left = width
        while left:
            n = min(left, r.randrange(1, longest + 1))
            if r.randrange(3) == 0:
                out.append((-n) & 0xff)
            else:
                n = min(n, 127)
                out.append(n)
                out += bytes(r.randrange(4) if small else r.randrange(256) for _ in range(n))
            left -= n
        out.append(0)
    return bytes(out)


def case(r):
    flags = r.deal([0, 1, 2, 3]) | r.deal([0, 0, 8, 0x10, 0x18])
    wide = r.randrange(6) == 0
    width = r.randrange(1, 140) if wide else r.randrange(1, 40)
    height = r.randrange(1, 4) if wide else r.randrange(1, 12)
    if r.randrange(30) == 0:
        width, height = r.choice([(0, height), (width, 0)])
    pitch = r.randrange(width + 8, width + 40) & ~3 if r.randrange(2) else r.randrange(width + 8, width + 40)
    left, top = r.randrange(0, 5), r.randrange(0, 3)
    clip_w = r.randrange(1, pitch - left + 1)
    clip_h = r.randrange(1, height + 6)
    x = r.randrange(-width - 3, clip_w + 3)
    y = r.randrange(-height - 3, clip_h + 3)
    if r.randrange(40) == 0:
        x, y = r.choice([(x, -0x7ffffff0), (x, 0x7ffffff0), (-0x7ffffff0, y), (0x7ffffff0, y)])
    data = rows(r, width, height, flags & 8)
    buf = bytes(r.randrange(4) for _ in range(pitch * (top + clip_h))) if flags & 0x10 and not flags & 8 else r.bytes(pitch * (top + clip_h))
    params = struct.pack('<iiiIIIIIii', pitch, clip_w, clip_h, flags, BITS, GHOST, 0, 0, left, top)
    regions = {PARAMS: params, SPRITE: struct.pack('<IHH', DATA, width, height), DATA: data or b'\0', BITS: buf}
    if flags & 0x18:
        regions[GHOST] = r.bytes(0x400)
    return Call(0x6e73c0, [x & 0xffffffff, y & 0xffffffff, SPRITE], regions)


if __name__ == '__main__':
    B.main(case)

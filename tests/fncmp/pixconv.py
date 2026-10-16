# The pixel converters (src/game/gfx/pixconv: palette 0x639230-0x6394d0, RGB 0x6396b0-0x63a700, 16-bit 0x63ae10-0x63b140, the mask helpers) against the
# originals: pixel formats of 8 to 32 bits with contiguous or odd masks; Convert and ConvertBack start from converters the originals' own Init made.
# usage: pixconv.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x639230, 0x63b2c0)])
OBJ, SRCF, DSTF, PAL, SRC, DST, OUT, OUT2 = RAM, RAM + 0x800, RAM + 0x820, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x4010
FIXED = {0xfaa268: (0xff0000, 0xff00, 0xff, 0, 24, 0), 0xfaa280: (0xff0000, 0xff00, 0xff, 0xff000000, 32, 0)}  # the game's two formats the 16-bit converter knows
KINDS = ['Mask', 'Chan', 'PalInit', 'PalCv', 'PalBack', 'RgbInit', 'RgbCv', 'RgbBack', 'R16Init', 'R16Ctor', 'R16Cv']
CACHE = {}


def fmt(r, bits=None):
    """A pixel format: red, green, blue, alpha masks, bit count, paletted."""
    bits = bits or r.choice([8, 15, 16, 16, 24, 32, 32])
    if r.random() < 0.15:  # odd masks
        return (r.getrandbits(32) & ((1 << bits) - 1) if bits < 32 else r.getrandbits(32), r.getrandbits(16) << r.randrange(8), r.getrandbits(8) << r.randrange(8), 0, bits, 0)
    layouts = {8: [(3, 3, 2, 0), (0, 0, 0, 0)], 15: [(5, 5, 5, 0), (5, 5, 5, 1)], 16: [(5, 6, 5, 0), (5, 5, 5, 1), (4, 4, 4, 4)], 24: [(8, 8, 8, 0)],
               32: [(8, 8, 8, 8), (8, 8, 8, 0), (10, 10, 10, 2)]}
    widths = r.choice(layouts[bits])
    order = r.choice([(0, 1, 2, 3), (2, 1, 0, 3), (3, 0, 1, 2)])  # which channel sits lowest
    masks, shift = [0, 0, 0, 0], 0
    for k in order:
        w = widths[k]
        if w:
            masks[k] = ((1 << w) - 1) << shift
            shift += w
    return tuple(masks) + (bits, 0)


def pack(f):
    return struct.pack('<6I', *f)


def made(r, name, addr, regions, args, reg):
    """The bytes of an object after the original's Init (run once under the emulator) for Convert's starting point."""
    c = Call(addr, args, regions, conv='t', ret='v')
    out = B.original(c)
    return bytes.fromhex(out[3 + list(regions).index(reg)])


def case(r):
    name = r.choice(OPS)
    if name == 'Mask':
        return Call(0x63acb0, [r.choice([0, 0xff, 0xff00, 0xff0000, 0xf800, 0x7e0, 0x1f, 0x3ff, r.getrandbits(32)]), r.choice([0, 0xff, 0xff00, 0xff0000, 0xf800, 0x7e0, 0x1f, 0x3ff, r.getrandbits(32)]),
                                OUT, OUT2], {OUT: r.bytes(4), OUT2: r.bytes(4)}, conv='s', ret='v')
    if name == 'Chan':
        return Call(0x63a700, [OBJ, r.choice([0, 0xff, 0xff00, 0xff0000, 0xf800, 0x7e0, 0x1f, 0x3ff, 0xff000000]), r.choice([0, 0xff, 0xff00, 0xff0000, 0xf800, 0x7e0, 0x1f, 0x3ff, 0xff000000]),
                                OUT, OUT2], {OBJ: r.bytes(0x54), OUT: r.bytes(4), OUT2: r.bytes(4)}, conv='t', ret='v')
    palette = bytearray(r.bytes(0x400))
    s, t = fmt(r), fmt(r)
    regions = {OBJ: r.bytes(0x440), SRCF: pack(s), DSTF: pack(t), PAL: palette, 0xfaa268: pack(FIXED[0xfaa268]), 0xfaa280: pack(FIXED[0xfaa280])}
    if name == 'PalInit':
        t = fmt(r, r.choice([8, 16, 16, 24, 32]))
        regions[DSTF] = pack(t)
        return Call(0x639230, [OBJ, PAL, DSTF], regions, conv='t', ret='v')
    if name == 'RgbInit':
        return Call(0x6396b0, [OBJ, SRCF, DSTF], regions, conv='t', ret='v')
    if name in ('R16Init', 'R16Ctor'):
        if r.random() < 0.7:  # a source the 16-bit converter knows, a 16-bit target
            s = r.choice(list(FIXED.values()))
            regions[SRCF] = pack(s)
        regions[DSTF] = pack(fmt(r, 16))
        return Call(0x63ae90 if name == 'R16Init' else 0x63ae10, [OBJ, SRCF, DSTF], regions, conv='t', ret='v' if name == 'R16Init' else 'a')
    # Convert / ConvertBack: the converter as the original's Init left it
    count = r.choice([0, 1, 2, 3, 4, 7, 16])
    src = r.bytes(4 * count + 8)
    dst = r.bytes(4 * count + 8)
    regions.update({SRC: src, DST: dst})
    if name in ('PalCv', 'PalBack'):
        t = fmt(r, r.choice([8, 16, 16, 24, 32]))
        regions[DSTF] = pack(t)
        regions[OBJ] = made(r, name, 0x639230, regions, [OBJ, PAL, DSTF], OBJ)
        return Call(0x6392c0 if name == 'PalCv' else 0x6394d0, [OBJ, DST, SRC, count], regions, conv='t', ret='v')
    if name in ('RgbCv', 'RgbBack'):
        regions[OBJ] = made(r, name, 0x6396b0, regions, [OBJ, SRCF, DSTF], OBJ)
        return Call(0x639780 if name == 'RgbCv' else 0x639f40, [OBJ, DST, SRC, count], regions, conv='t', ret='v')
    s = r.choice(list(FIXED.values()))  # R16Cv
    regions[SRCF] = pack(s)
    regions[DSTF] = pack(fmt(r, 16))
    regions[OBJ] = made(r, name, 0x63ae90, regions, [OBJ, SRCF, DSTF], OBJ)
    return Call(0x63b140, [OBJ, DST, SRC, r.choice([2, 4, 6, 8, 10, 3, 5])], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

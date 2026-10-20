# The bitmap copy and its static conversion (src/game/gfx/convert: 0x5fc320-0x5fc528) against the originals. The conversion at 0xfa7d18 holds a
# converter whose vtable is recorders (or none), as pixconversion.py has it: its static initialiser, constructor, atexit registration (0x67a7d0
# fncmp's cdecl log on both sides, driver.cpp's Convert env) and destructor. SurfaceInfo_Convert from one bitmap info into another: either
# without bits; the same format (a straight row copy over the smaller size, or in place: nothing); another (pixconv.py's formats, or the source
# paletted with a palette or none: the conversion set up with the width as its flags and the originals' own converter code making the rows, the
# heap failing now and then), in place too (the destination's width cut to its pitch). Sizes up to 8 by 8, pitches of a row or more.
# usage: convert.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=StaticInit,... to pick)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, global_heap, heap, log_region, put, vtable  # noqa: E402
from pixconv import FIXED, fmt, pack  # noqa: E402

B = Bench([(0x5fc320, 0x5fc52b), (0x630880, 0x630c97), (0x639230, 0x63b2c0), (0x642860, 0x6428a5), (0x5f5660, 0x5f568c), (0x67acb0, 0x67acb1),
           (0x67acc0, 0x67acc1), (0x5da3d0, 0x5da3d3)], hooks={0x67a7d0: _cdecl_log})
CALLS = {'StaticInit': 0x5fc320, 'Construct': 0x5fc330, 'RegisterExit': 0x5fc340, 'AtExit': 0x5fc350, 'Convert': 0x5fc360}
CONV = 0xfa7d18
OLD, VT, OUT, DST, SRC, PAL, DBITS, SBITS = RAM, RAM + 0x600, RAM + 0x700, RAM + 0x740, RAM + 0x780, RAM + 0x800, RAM + 0x1000, RAM + 0x1800


def info(r, f, bits):
    i = bytearray(pack(f) + r.bytes(0x10))
    w, h = r.randrange(9), r.randrange(9)
    row = w * ((f[4] + 7) >> 3)
    put(i, 0x18, '3Ii', bits, w, h, r.choice([row, row, row + r.randrange(1, 16), 0x80]))
    return i


def case(r):
    op = r.deal(OPS)
    old = bytearray(r.bytes(0x440))
    put(old, 0, 'I', VT)
    conv = bytearray(r.bytes(0x18))
    put(conv, 8, 'I', OLD if r.random() < 0.9 else 0)
    put(conv, 0x14, 'I', r.choice([0, 1, 2, 3, 4, r.getrandbits(32)]))
    log = log_region()
    regions = {CONV - 8: conv, OLD: old, VT: vtable({0: 1, 1: 3, 2: 3}, size=4), LOG: log, HEAP: heap(fail=r.choice([0, 0, 0, 0, 1]))}
    regions.update(global_heap())
    if op != 'Convert':
        return Call(CALLS[op], [], regions, conv='c', ret='a' if op == 'Construct' else 'v')
    source = r.choice([fmt(r), fmt(r), (0, 0, 0, 0, 8, 1)] + list(FIXED.values()))
    target = source if r.random() < 0.35 else r.choice([fmt(r, r.choice([15, 16, 24, 32])), r.choice(list(FIXED.values()))])
    if r.random() < 0.25:   # (the 16-bit converter's: the game's 24 / 32-bit formats into 16 bits, an even width only)
        source, target = r.choice(list(FIXED.values())), fmt(r, r.choice([15, 16]))
    elif r.random() < 0.15:   # (a format that differs only in its alpha mask or in being paletted)
        source = r.choice([(0, 0, 0, 0, 8, 1), (0xff0000, 0xff00, 0xff, 0, 32, 0), (0x7c00, 0x3e0, 0x1f, 0, 16, 0)])
        target = (0, 0, 0, 0, 8, 0) if source[5] else source[:3] + (0x8000 if source[4] == 16 else 0xff000000,) + source[4:]
        if not source[5] and r.random() < 0.5:   # (never into a paletted one: the index converter's map, pixconversion.py's gap)
            source, target = target, source
    sbits = r.choice([SBITS, SBITS, SBITS, 0])
    dbits = r.choice([DBITS, DBITS, DBITS, sbits, 0])
    regions.update({DST: info(r, target, dbits), SRC: info(r, source, sbits), OUT: r.bytes(4), DBITS: r.bytes(0x800), SBITS: r.bytes(0x800),
                    PAL: r.bytes(0x400), 0xfaa268: pack(FIXED[0xfaa268]) + pack(FIXED[0xfaa280])})
    return Call(CALLS[op], [r.getrandbits(32), OUT, DST, SRC, r.choice([PAL, PAL, 0])], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

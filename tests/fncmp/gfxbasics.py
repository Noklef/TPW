# The rectangle helpers (src/game/math/rect: 0x61af20-0x61b029, 0x62f810-0x62f975, 0x5fb250, 0x5fb770) and the colour to pixel conversion
# (src/game/gfx/color: 0x5fccf0-0x5fd639) against the originals: rectangles around and beyond each other (empty, inverted, extreme), colours in
# the game's static formats (run through their initialisers first) and in made-up generic ones.
# usage: gfxbasics.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

# name: (address, convention, arguments: a rect, b second rect, o output rect, n integers, r nothing; returns)
RECTS = {'Copy': (0x61af20, 'c', 'ab', 'v'), 'Width': (0x61af40, 'c', 'a', 'a'), 'Height': (0x61af50, 'c', 'a', 'a'), 'Intersect': (0x61af60, 'c', 'ab', 'a'),
         'Collapse': (0x61afe0, 'c', 'a', 'v'), 'Normalize': (0x61aff0, 'c', 'a', 'v'), 'Clip': (0x62f810, 't', 'aob', 'a'), 'Overlaps': (0x62f890, 't', 'ab', 'a'),
         'Union': (0x62f8d0, 't', 'aob', 'a'), 'Order': (0x62f940, 't', 'a', 'v'), 'MoveTo': (0x5fb250, 't', 'aii', 'v'), 'Set': (0x5fb770, 't', 'aiiii', 'a')}
INITS = {'I0': 0x5fccf0, 'I1': 0x5fcd40, 'I2': 0x5fcd90, 'I3': 0x5fcde0, 'I4': 0x5fce30, 'I5': 0x5fce80, 'I6': 0x5fced0, 'I7': 0x5fcf20, 'I8': 0x5fcf70,
         'I9': 0x5fcfc0, 'I10': 0x5fd010}
CALLS = dict(RECTS)
B = Bench([(0x61af20, 0x61b030), (0x62f810, 0x62f980), (0x5fb250, 0x5fb290), (0x5fb770, 0x5fb790), (0x5fccf0, 0x5fd640)])
A, Bb, O, CLR, FMT = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x100, RAM + 0x200
STATIC = 0xfa7d28  # the static formats (0x18 bytes each, from 0xfa7d28 to 0xfa7e30)
VALUES = [0, 1, 2, 5, 10, 100, -1, -5, -100, 0x7fffffff, -0x80000000, 0x7ffffffe]


def rect(r):
    if r.random() < 0.7:
        x, y = r.randrange(-10, 20), r.randrange(-10, 20)
        return [x, y, x + r.randrange(-3, 15), y + r.randrange(-3, 15)]
    return [r.choice(VALUES) for _ in range(4)]


def static_formats():
    """The initialisers run once by the emulator: the static formats' bytes."""
    e = B.emu
    e.ranges = None
    e.mem.clear()
    e.write(STATIC, bytes([0xcd]) * 0x120)
    for addr in INITS.values():
        e.reset()
        e.push(0x0)
        pc = addr
        for _ in range(1000):
            if pc == 0:
                break
            pc = e.step(pc)
    return e.read(STATIC, 0x120)


FORMATS = None


def generic_format(r):
    """A made-up pixel format: contiguous masks that do not overlap, or sometimes anything."""
    if r.random() < 0.2:
        return struct.pack('<6I', *[r.getrandbits(32) if k < 4 else r.choice([8, 15, 16, 24, 32, 0]) for k in range(5)], r.choice([0, 0, 1]))
    widths = [r.choice([0, 4, 5, 6, 8]) for _ in range(4)]
    masks, shift = [], 0
    for wd in widths:
        masks.append(((1 << wd) - 1) << shift if wd else 0)
        shift += wd
    return struct.pack('<6I', *masks, shift if r.random() < 0.8 else r.choice([8, 16, 24, 32]), r.choice([0, 0, 0, 1]))


def case(r):
    global FORMATS
    name = r.choice(OPS)
    if name in INITS:
        return Call(INITS[name], [], {STATIC: r.bytes(0x120)}, conv='c', ret='v')
    if name in RECTS:
        address, conv, kinds, ret = RECTS[name]
        regions = {A: struct.pack('<4i', *rect(r)), Bb: struct.pack('<4i', *rect(r)), O: r.bytes(0x10)}
        args = []
        for k in kinds:
            args.append({'a': A, 'b': Bb, 'o': O}.get(k) if k in 'abo' else r.choice(VALUES))
        if r.random() < 0.15 and 'o' in kinds:  # the output may be either input
            args[kinds.index('o')] = r.choice([A, Bb])
        if r.random() < 0.1 and 'b' in kinds:  # a rectangle with itself
            args[kinds.index('b')] = A
        return Call(address, args, regions, conv=conv, ret=ret)
    if FORMATS is None:
        FORMATS = static_formats()
    regions = {STATIC: FORMATS, CLR: struct.pack('<9I', r.getrandbits(32) if r.random() < 0.5 else r.choice([0, 0xffffffff, 0xff00ff00, 0x80ff8000]), *[r.getrandbits(32) for _ in range(8)]),
               FMT: generic_format(r)}
    fmt = r.choice([STATIC + 0x18 * k for k in range(11)] + [FMT, FMT])  # (the formats sit apart; any 0x18-aligned address in the table is only valid at the real ones)
    real = [0xfa7d40, 0xfa7de8, 0xfa7e18, 0xfa7d88, 0xfa7d70, 0xfa7e00, 0xfa7dd0, 0xfa7db8, 0xfa7d58, 0xfa7da0, 0xfa7d28]
    fmt = r.choice(real + [FMT, FMT, FMT])
    if name == 'ToPixel':
        return Call(0x5fd050, [CLR, fmt], regions, conv='t', ret='a')
    if name == 'Equal':
        return Call(0x5fd400, [fmt, r.choice(real + [FMT])], regions, conv='t', ret='a')
    if name == 'Compute':
        return Call(0x5fd460, [CLR + 4, r.choice([0xff0000, 0xff00, 0xff, 0xff000000, 0xf80000, 0xe0, r.getrandbits(32)]), r.choice([0xff00, 0xff, 0xff0000]),
                               r.choice([0xff, 0xff00]), r.choice([0xff000000, 0, 0xff, r.getrandbits(32)]), fmt], regions, conv='t', ret='v')
    if name == 'Swap':
        return Call(0x5fd5a0, [CLR], regions, conv='t', ret='a')
    return Call(0x5fd5c0, [CLR, CLR + 4], regions, conv='t', ret='a')  # Shift


KINDS = ['ToPixel', 'Equal', 'Compute', 'Swap', 'Shift']
OPS = [k for k in os.environ.get('OPS', ','.join(list(RECTS) + list(INITS) + KINDS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

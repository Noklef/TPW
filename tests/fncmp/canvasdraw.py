# The canvas primitives (src/game/gfx/canvasdraw: the four CanvasDriver tables' slots, 0x703650 / 0x703680 / 0x7036b0 / 0x7036e0) against the
# originals, each called directly on a driver of its depth's table: a surface of random pixels (a margin round the clip rectangle, which lies
# inside it) in one of the game's static formats of that depth (the initialisers run once, as gfxbasics.py does) or a made-up one, now and then
# of another bit count; the flags (blend 8 / 0x10, outline 4, the masks' flips 1 / 2, or random); a colour of random ARGB; coordinates in and
# around the clip, wild now and then where the primitive clips them first; masks and sprites as draw8.py's run-length rows, a sprite's table of
# random pixels; the game's clip bottom (0xfa76cc, the text's new lines) at or above the surface's last rows.
# usage: canvasdraw.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import draw8  # noqa: E402
import gfxbasics  # noqa: E402
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x5fb250, 0x5fb290), (0x5fb770, 0x5fb790), (0x5fccf0, 0x617370), (0x62f810, 0x62f990)])
DRIVER, SURFACE, CLIP, COLOUR, RECT, TEXT, MASK, TABLE, DATA, BITS = (RAM, RAM + 0x40, RAM + 0x80, RAM + 0xc0, RAM + 0x100, RAM + 0x200,
                                                                      RAM + 0x300, RAM + 0x400, RAM + 0x800, RAM + 0x4000)
TEXT_BOTTOM = 0xfa76cc
WIDTH, ROWS = 96, 72  # the surface in pixels; the clip lies in [8, 88) x [8, 64)
TABLES = {8: 0x703650, 16: 0x703680, 24: 0x7036b0, 32: 0x7036e0}
# slot: (name, arguments after (surface, clip, flags): as draw8.py's, 'M' the mask, 'P' the sprite's table then x, y and the sprite)
SLOTS = [('Circle', 'xyrc'), ('Disc', 'xyrc'), ('HSpan', 'xync'), ('VSpan', 'xync'), ('Line', 'xyxyc'), ('Pixel', 'xyc'), ('Text', 'xyTc'),
         ('Outline', 'Rc'), ('FillRect', 'Rc'), ('Triangle', 'xyxyxyc'), ('Mask', 'xyMc'), ('Sprite', 'PxyM')]
ADDRESSES = {8: [0x5fd640, 0x600060, 0x601b40, 0x601d10, 0x601ee0, 0x601a00, 0x6038e0, 0x6025c0, 0x6026b0, 0x6028e0, 0x602d70, 0x6033a0],
             16: [0x603ef0, 0x6066c0, 0x608270, 0x608430, 0x608600, 0x608140, 0x609e70, None, 0x608c60, 0x608e80, 0x609320, 0x609910],
             24: [0x60a480, 0x60d4f0, 0x60f090, 0x60f240, 0x60f410, 0x60ef50, 0x610d20, None, 0x60fab0, 0x60fc90, 0x610160, 0x6106e0],
             32: [0x611360, 0x6139a0, 0x615380, 0x615520, 0x6156c0, 0x615250, 0x616de0, None, 0x615cd0, 0x615ea0, 0x616340, 0x616890]}
CALLS = {SLOTS[slot][0] + ('' if slot == 7 else str(depth)): (slot, at, depth) for depth, slots in ADDRESSES.items()
         for slot, at in enumerate(slots) if at}
FORMATS = None
STATIC = [0xfa7d40, 0xfa7de8, 0xfa7e18, 0xfa7d88, 0xfa7d70, 0xfa7e00, 0xfa7dd0, 0xfa7db8, 0xfa7d58, 0xfa7da0, 0xfa7d28]


def surface_format(r, depth, other=0.1):
    """A static format of this depth (its masks, bit count and palettedness), or a made-up one; now and then of another bit count."""
    mine = [f for f in (FORMATS[a - gfxbasics.STATIC:a - gfxbasics.STATIC + 0x18] for a in STATIC)
            if (struct.unpack_from('<I', f, 0x10)[0] + 7) >> 3 == depth >> 3]
    f = bytearray(r.choice(mine) if mine and r.random() < 0.7 else gfxbasics.generic_format(r))
    count = depth if r.random() >= other else r.choice([0, 8, 15, 16, 24, 32])
    return put(f, 0x10, 'I', count)


def zero_some(r, codes, rows):
    """Now and then a literal's bytes 0 here and there (a sprite's index 0; where a clipped row starts inside a literal, the code drops the
    rest of it when the byte reached is 0, and a mask's next row then starts in the literal)."""
    if r.random() < 0.6:
        return codes
    codes, p = bytearray(codes), 0
    for _ in range(rows):
        while codes[p]:
            n = codes[p] if codes[p] < 0x80 else 0
            for i in range(p + 1, p + 1 + n):
                if r.random() < 0.15:
                    codes[i] = 0
            p += n + 1
        p += 1
    return codes


def case(r):
    global FORMATS
    if FORMATS is None:
        FORMATS = gfxbasics.static_formats()
    slot, at, depth = CALLS[r.deal(OPS)]
    if slot == 7:   # (the outline goes through any depth's spans)
        depth = r.choice(list(TABLES))
    name, shape = SLOTS[slot]
    size = depth >> 3
    pitch = WIDTH * size + r.choice([0, 0, 4, 12])
    left, top = r.randrange(8, 48), r.randrange(8, 36)
    clip = r.choice([(8, 8, 88, 64)] * 2 + [(left, top, r.randrange(left, 89), r.randrange(top, 65))])
    flags = r.choice([0, 0, 8, 0x10, 0x18, 4, 0xc]) | (r.getrandbits(2) if 'M' in shape else 0)
    if r.random() < 0.05:
        flags = r.getrandbits(32)
    surface = bytearray(surface_format(r, depth, 0.4 if name == 'Disc' else 0.1) + r.bytes(0x10))   # (the disc's rows start by the surface's own)
    put(surface, 0x18, 'I', BITS)
    put(surface, 0x1c, 'II', WIDTH, ROWS)
    put(surface, 0x24, 'i', pitch)
    colour = bytearray(r.bytes(0x24))
    if r.random() < 0.3:
        put(colour, 3, 'B', r.choice([0, 0xff, 0x80]))
    regions = {DRIVER: struct.pack('<I', TABLES[depth]), SURFACE: surface, CLIP: struct.pack('<4i', *clip), COLOUR: colour, BITS: r.bytes(pitch * ROWS + 16),
               gfxbasics.STATIC: FORMATS, TEXT_BOTTOM: struct.pack('<i', r.choice([clip[3], ROWS - 8, ROWS - 6, r.randrange(clip[1], ROWS - 5)]))}
    args = [DRIVER, SURFACE, CLIP, flags]
    wild = len(shape) <= 4 and name not in ('Disc', 'Circle')
    xs = [draw8.coord(r, clip[0], clip[2], wild) for _ in range(3)]
    ys = [draw8.coord(r, clip[1], clip[3], wild) for _ in range(3)]
    for ch in shape:
        if ch == 'x':
            args.append(xs.pop())
        elif ch == 'y':
            args.append(ys.pop())
        elif ch == 'r':   # (a negative radius: the disc fills without end, the circle blended writes its ends unclipped)
            args.append(r.choice([0, 1, 1, 2, 3, r.randrange(4, 30), r.randrange(4, 30), r.randrange(30, 90)] + ([-1, -2, -5] if name == 'Circle' else [])))
        elif ch == 'n':
            args.append(r.choice([0, 1, -1, r.randrange(-90, 90), r.randrange(-90, 90), r.randrange(-90, 90)] + ([r.getrandbits(32)] if wild else [])))
        elif ch == 'R':
            rx = [draw8.coord(r, clip[0], clip[2], False) for _ in range(2)]
            ry = [draw8.coord(r, clip[1], clip[3], False) for _ in range(2)]
            regions[RECT] = struct.pack('<4i', rx[0], ry[0], rx[1], ry[1])
            args.append(RECT)
        elif ch == 'T':
            if r.random() < 0.6 and clip[2] - clip[0] >= 6 and clip[3] - clip[1] >= 6:   # (the first line inside the clip)
                args[4], args[5] = r.randrange(clip[0], clip[2] - 5), r.randrange(clip[1], clip[3] - 5)
            chars = [r.choice([r.randrange(0x20, 0x7f)] * 6 + [9, 9, 10, r.randrange(0x7f, 0xb0), r.getrandbits(8) or 1]) for _ in range(r.randrange(0, 24))]
            if r.random() < 0.3:   # (a new line's test on the game's clip bottom, at its edge: a line of glyphs after it)
                regions[TEXT_BOTTOM] = struct.pack('<i', min(args[5] + 14 + r.choice([0, 0, 0, -1, 1]), ROWS))
                chars = [c for c in chars if c != 10] + [10] + [r.randrange(0x21, 0x7f) for _ in range(4)]
            if r.random() < 0.15:   # (far left)
                args[4] = (args[4] - r.choice([0x10000, 0x20000, 0x1000000])) & 0xffffffff
            regions[TEXT] = bytes(chars) + bytes(2)
            args.append(TEXT if r.random() < 0.95 else 0)
        elif ch == 'M':
            w, h = r.choice([r.randrange(1, 12), r.randrange(1, 60)]), r.choice([r.randrange(1, 12), r.randrange(1, 60)])
            if r.random() < 0.03:
                w, h = r.choice([(0, h), (w, 0)])
            regions[MASK] = put(bytearray(8), 0, 'IHH', DATA, w, h)
            regions[DATA] = zero_some(r, draw8.rle(r, max(w, 1), max(h, 1)), max(h, 1)) + bytes(256)
            args.append(MASK)
        elif ch == 'P':
            regions[TABLE] = r.bytes(0x400)
            args.append(TABLE)
        elif ch == 'c':
            args.append(COLOUR)
    return Call(at, [a & 0xffffffff for a in args], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

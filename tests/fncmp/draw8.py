# The 8-bit drawing primitives and the shared slots (src/game/gfx/draw8: the 0x7034d8 table's sixteen) against the originals, each called
# directly on the table's driver: a surface of random pixels (a margin round the clip rectangle, which lies inside it), the context's flags
# (outline 4, ghost 8 / 0x10, remap 0x20, the sprites' flips 1 / 2), the remap and ghost tables random; coordinates in and around the clip,
# now and then on its edges or wild where the primitive clips them first; colours random with whatever stack bytes above the index. Slot 14
# hands over to the cseg blitter (0x6e73c0, not in the listing): fncmp's 4-argument logger on both sides (driver.cpp's Draw_SetEnv), its
# fourth argument (the caller's stack) logged as 0. drawrgb.py runs the 16/24/32-bit tables through make().
# usage: draw8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import fncmp  # noqa: E402
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, log_region, put, strings  # noqa: E402

B = Bench([(0x61af40, 0x61af70), (0x61b1e0, 0x61f3b0), (0x628aa0, 0x628b70), (0x629bf0, 0x629c60), (0x62f810, 0x62f8a0)],
          hooks={0x6e73c0: fncmp._cdecl_log4})
STATE, PARAMS, PALETTE = 0xfa76b8, 0xfaa4d8, 0xfa71f8
SURFACE, REMAP, RECT, TEXT, SPRITE, DATA, BITS, GHOST = (RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x600, RAM + 0x2000, RAM + 0x8000,
                                                         RAM + 0x10000)
WIDTH, ROWS = 96, 72  # the surface in pixels; the clip lies in [8, 88) x [8, 64)
# slot: (name, arguments after the driver: 'x' / 'y' a coordinate, 'r' a radius, 'n' a length, 'R' the rectangle, 'T' / 'W' the text,
# 'S' the sprite, 'c' the colour (two dwords))
SLOTS = [('Pixel', 'xyc'), ('Box', 'Rc'), ('BoxOutline', 'Rc'), ('BoxFill', 'Rc'), ('Circle', 'xyrc'), ('CircleOutline', 'xyrc'),
         ('CircleFill', 'xyrc'), ('Line', 'xyxyc'), ('HLine', 'xync'), ('VLine', 'xync'), ('Triangle', 'xyxyxyc'), ('Clear', 'c'), ('Text', 'xyTc'),
         ('TextW', 'xyWc'), ('Sprite', 'xyS'), ('SpriteShape', 'xySc')]
TABLE8 = 0x7034d8
ADDRESSES8 = [0x61baf0, 0x61bc90, 0x628aa0, 0x61eed0, 0x629bf0, 0x61c630, 0x61e100, 0x61bce0, 0x61c2e0, 0x61c490, 0x61eab0, 0x61ee30, 0x61f0a0,
              0x61f220, 0x61b1e0, 0x61b270]
SHARED = {1, 2, 4, 10}  # the slots every table shares (Draw_Box, Draw_BoxOutline, Draw_Circle, Draw_Triangle)


def coord(r, lo, hi, wild):
    """A coordinate around the clip [lo, hi): mostly near it, sometimes on its edges, wild (where the primitive clips first) now and then."""
    k = r.random()
    if k < 0.15:
        return r.choice([lo - 1, lo, hi - 1, hi, hi + 1])
    if wild and k < 0.2:
        return r.getrandbits(32) - (1 << 31)
    return r.randrange(lo - 24, hi + 24)


def rle(r, w, h):
    """A run-length sprite's rows: runs of skips (n < 0) and literals (n > 0, n bytes following), mostly filling the width (now and then
    overshooting it or ending early), each row ended by 0."""
    out = bytearray()
    for _ in range(h):
        left = w
        while left > 0 and r.random() < 0.95:
            n = r.randrange(1, min(left, 40) + 1)
            if r.random() < 0.1:
                n += r.randrange(1, 8)   # past the width
            if r.random() < 0.4:
                out.append(-min(n, 128) & 0xff)
            else:
                n = min(n, 127)
                out.append(n)
                out += r.bytes(n)
            left -= n
        out.append(0)
    return out + bytes(64)


def make(r, slot, at, table, size, wild=True):
    """A case calling slot `slot` (at `at`) of the table at `table` for pixels of `size` bytes."""
    name, shape = SLOTS[slot]
    pitch = WIDTH * size + r.choice([0, 0, 4, 12])
    left, top = r.randrange(8, 48), r.randrange(8, 36)
    clip = r.choice([(8, 8, 88, 64)] * 2 + [(left, top, r.randrange(left, 89), r.randrange(top, 65))])
    if name == 'SpriteShape' and clip[1] == clip[3]:   # (no height: the original's row count is a byte, which wraps and runs on)
        clip = (clip[0], clip[1], clip[2], clip[3] + 1)
    flags = r.choice([0, 0, 0, 4, 8, 0x10, 0x20, 0x24, 0x28, 0x30]) | (r.getrandbits(2) if 'S' in shape else 0)
    if r.random() < 0.05:
        flags = r.getrandbits(32)
    state = bytearray(r.bytes(0x74))
    put(state, 0, 'I', REMAP)
    put(state, 8, 'iiii', *clip)
    put(state, 0x18, 'I', STATE + 0x38)
    put(state, 0x30, 'I', SURFACE)
    put(state, 0x38, 'II', table, flags)
    put(state, 0x4c, 'I', BITS)
    put(state, 0x70, 'I', GHOST)
    surface = put(bytearray(r.bytes(0x28)), 0x24, 'i', pitch)
    regions = {STATE: state, SURFACE: surface, BITS: r.bytes(pitch * ROWS), REMAP: r.bytes(0x100), GHOST: r.bytes(0x10000), PALETTE: r.bytes(0x400)}
    args = [STATE + 0x38]
    xs = [coord(r, clip[0], clip[2], wild and len(shape) <= 4) for _ in range(3)]
    ys = [coord(r, clip[1], clip[3], wild and len(shape) <= 4) for _ in range(3)]
    for ch in shape:
        if ch == 'x':
            args.append(xs.pop())
        elif ch == 'y':
            args.append(ys.pop())
        elif ch == 'r':
            args.append(r.choice([0, 1, 1, 2, 3, -1, -2, -5, r.randrange(4, 30), r.randrange(4, 30), r.randrange(30, 90)]))
        elif ch == 'n':
            args.append(r.choice([0, 1, -1, r.randrange(-90, 90), r.randrange(-90, 90), r.randrange(-90, 90)] + ([r.getrandbits(32)] if wild else [])))
        elif ch == 'R':
            rx = sorted([coord(r, clip[0], clip[2], False), coord(r, clip[0], clip[2], False)])
            ry = sorted([coord(r, clip[1], clip[3], False), coord(r, clip[1], clip[3], False)])
            if r.random() < 0.1:
                rx.reverse()
            if r.random() < 0.1:
                ry.reverse()
            regions[RECT] = put(bytearray(16), 0, 'iiii', rx[0], ry[0], rx[1], ry[1])
            args.append(RECT)
        elif ch in 'TW':
            chars = [r.choice([r.randrange(0x20, 0x7f)] * 6 + [9, 9, 10, r.randrange(0x7f, 0xb0), r.getrandbits(8 if ch == 'T' else 16) or 1])
                     for _ in range(r.randrange(0, 24))]
            if r.random() < 0.15:   # (far left: a tab stop keeps x's low 16 bits only)
                args[1] = (args[1] - r.choice([0x10000, 0x20000, 0x1000000])) & 0xffffffff
            text = b''.join(c.to_bytes(1 if ch == 'T' else 2, 'little') for c in chars) + bytes(2)
            regions[TEXT] = text
            args.append(TEXT if r.random() < 0.95 else 0)
        elif ch == 'S':
            w, h = r.choice([r.randrange(1, 12), r.randrange(1, 60)]), r.choice([r.randrange(1, 12), r.randrange(1, 60)])
            if r.random() < 0.03 and (size > 1 or name == 'Sprite'):   # (no rows at 8 bits: the shape's row count wraps, as above)
                w, h = r.choice([(0, h), (w, 0)])
            regions[SPRITE] = put(bytearray(8), 0, 'IHH', DATA, w, h)
            regions[DATA] = rle(r, max(w, 1), max(h, 1))
            args.append(SPRITE)
        elif ch == 'c':
            args += [r.getrandbits(32), r.getrandbits(32)]
    if name == 'Sprite':   # (the blitter's parameters; its fourth "argument" the caller's stack)
        regions[PARAMS] = r.bytes(0x28)
        log = log_region()
        strings(log, 0x3d, 0, 8)
        regions[LOG] = log
    return Call(at, [a & 0xffffffff for a in args], regions, conv='t', ret='v')


CALLS = {name: (slot, ADDRESSES8[slot]) for slot, (name, _) in enumerate(SLOTS)}


def case(r):
    slot, at = CALLS[r.deal(OPS)]
    return make(r, slot, at, TABLE8, 1)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

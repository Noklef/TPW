# The antialiased lines (src/game/gfx/aaline) against the original: 0x5edad0, 0x5ee580, 0x5ef040 and 0x5efb10 (1, 2, 3 and 4 bytes a
# pixel) on surfaces up to 24 x 20 pixels (pitches at, above and now and then below the row, now and then negative), in the game's
# static formats or made-up ones (gfxbasics), clip rectangles inside the surface, empty or now and then reaching past it, end points
# inside, around and far outside it (both orders, all octants, equal now and then), any colours.
# usage: aaline.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Draw8,Draw16,Draw24,Draw32)
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import gfxbasics  # noqa: E402
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x5edad0, 0x5f0610), (0x5fd050, 0x5fd650)])
KINDS = {'Draw8': (0x5edad0, 1), 'Draw16': (0x5ee580, 2), 'Draw24': (0x5ef040, 3), 'Draw32': (0x5efb10, 4)}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]
SURF, CLIP, CLR, FROM, TO, PIX = RAM, RAM + 0x40, RAM + 0x60, RAM + 0x90, RAM + 0xa0, RAM + 0x1000
REAL = {1: [(0xe0, 0x1c, 0x3, 0, 8)], 2: [(0xf800, 0x7e0, 0x1f, 0, 16), (0x7c00, 0x3e0, 0x1f, 0x8000, 16), (0xf00, 0xf0, 0xf, 0xf000, 16)],
        3: [(0xff0000, 0xff00, 0xff, 0, 24), (0xff, 0xff00, 0xff0000, 0, 24)], 4: [(0xff0000, 0xff00, 0xff, 0xff000000, 32), (0xff, 0xff00, 0xff0000, 0xff000000, 32)]}
FORMATS = None


def point(r, w, h):
    if r.random() < 0.1:
        return [r.randrange(-300, 300), r.randrange(-300, 300)]
    return [r.randrange(-6, w + 6), r.randrange(-6, h + 6)]


def case(r):
    global FORMATS
    if FORMATS is None:
        FORMATS = gfxbasics.static_formats()
    address, n = KINDS[r.deal(OPS)]
    w, h = r.randrange(1, 25), r.randrange(1, 21)
    row = w * n
    pitch = r.choice([row, row, row + r.choice([1, 2, 4, 7]), max(1, row - r.randrange(1, 3))])
    size = h * pitch + row + 4
    bits = PIX
    if r.random() < 0.15:   # bottom-up
        bits, pitch = PIX + (h - 1) * pitch, -pitch
    if r.random() < 0.6:
        fmt = struct.pack('<6I', *r.choice(REAL[n]), 0)
    else:
        fmt = gfxbasics.generic_format(r)
    surf = fmt + struct.pack('<IIIi', bits, w, h, pitch)
    if r.random() < 0.95:
        x0, y0 = r.randrange(0, w), r.randrange(0, h)
        clip = [x0, y0, r.randrange(x0, w + 1), r.randrange(y0, h + 1)] if r.random() < 0.4 else [0, 0, w, h]
    else:   # past the surface's right or bottom edge (the slack and the next rows): crashes now and then
        clip = [0, 0, w + r.randrange(0, 3), h + r.randrange(0, 2)]
    a = point(r, w, h)
    b = point(r, w, h) if r.random() < 0.95 else list(a)
    colour = struct.pack('<9I', r.getrandbits(32) if r.random() < 0.6 else r.choice([0, 0xffffffff, 0xff00ff00, 0x80ff8000]), *[r.getrandbits(32) for _ in range(8)])
    regions = {SURF: surf, CLIP: struct.pack('<4i', *clip), CLR: colour, FROM: struct.pack('<2i', *a), TO: struct.pack('<2i', *b),
               PIX: r.bytes(size), gfxbasics.STATIC: FORMATS}
    return Call(address, [r.getrandbits(32), SURF, CLIP, CLR, FROM, TO], regions, conv='c')


if __name__ == '__main__':
    B.main(case)

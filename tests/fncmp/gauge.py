# The framed gauge's plain members (src/game/gfx/gauge: 0x47faa0-0x47fbe0) against the
# original: init, marks, frame and colours on a gauge of random bytes; and its draws (0x47fc00, 0x47fc50) through the display front end
# and the 8-bit primitives on a 64-pixel-pitch surface: frames and marks in and around the clip rectangle (the first mark's y often
# negative), plain, remapped (0x20) or blended (8, 0x10) drawing. Setup (0x4806a0) on a window rectangle of random s16s with the screen
# scales 0xfaa5c0 / 0xfaa5c4 and a random game palette (0xfa71f8).
# usage: gauge.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x47faa0, 0x47fbf3),  # 0x47fbe0's ret 8 is at 0x47fbf0
           (0x47fc00, 0x47fc50), (0x47fc50, 0x47fcd0), (0x5f9780, 0x5f97c0), (0x5f9860, 0x5f98d0), (0x5f98d0, 0x5f9940), (0x5f9a80, 0x5f9ac0),
           (0x61af40, 0x61af50), (0x61af50, 0x61af60), (0x61c2e0, 0x61c490), (0x61c490, 0x61c630), (0x61eed0, 0x61f0a0), (0x62f810, 0x62f890),
           (0x62f890, 0x62f8d0), (0x62f940, 0x62f980), (0x4806a0, 0x480810), (0x5f73a0, 0x5f76b7)])
CALLS = {'Init': (0x47faa0, 0), 'MarksX': (0x47fb10, 3), 'MarksY': (0x47fb30, 3), 'Rect': (0x47fb50, 4),
         'Colour0': (0x47fb80, 2), 'Colour1': (0x47fba0, 2), 'Colour2': (0x47fbc0, 2), 'Colour3': (0x47fbe0, 2),
         'DrawBack': (0x47fc00, 0), 'DrawAxes': (0x47fc50, 0), 'Setup': (0x4806a0, 6)}
STATE, TABLE8 = 0xfa76b8, 0x7034d8
SURFACE, BITS, REMAP, GHOST = RAM + 0x100, RAM + 0x1000, RAM + 0x200, RAM + 0x10000
PITCH, ROWS = 64, 56


def coord(r, top):
    return r.choice([r.randrange(-20, top + 20)] * 2 + [r.randrange(top)] * 6 + [-1, 0, top, r.getrandbits(32) - (1 << 31)])


def draw(r, at):
    """A gauge's draw: its frame and marks and colours random-ish, the draw context's clip inside the surface, its flags."""
    gauge = bytearray(r.bytes(0x50))
    put(gauge, 4, 'i', r.choice([0, 1, 7, 30, -5, r.getrandbits(32) - (1 << 31)]))
    xs, ys = [coord(r, 60), coord(r, 60)], [coord(r, 48), coord(r, 48)]
    if r.random() < 0.8:   # in order (left <= right, top <= bottom), as the game sets them
        xs.sort()
        ys.sort()
    put(gauge, 8, 'iiii', xs[0], ys[0], xs[1], ys[1])
    put(gauge, 0x1c, 'i', r.choice([-1, -5, -30, 0, 3, coord(r, 48)]))
    put(gauge, 0x24, 'i', r.choice([0, 5, 20, -40, -1, coord(r, 48)]))
    left, top = r.randrange(0, 40), r.randrange(0, 30)
    clip = r.choice([(0, 0, 60, 48), (left, top, r.randrange(left, 61), r.randrange(top, 49))])
    state = bytearray(r.bytes(0x74))
    put(state, 0, 'I', REMAP)
    put(state, 8, 'iiii', *clip)
    put(state, 0x18, 'I', STATE + 0x38)
    put(state, 0x30, 'I', SURFACE)
    put(state, 0x38, 'II', TABLE8, r.choice([0, 0, 0, 0x20, 8, 0x10, 4]))
    put(state, 0x4c, 'I', BITS)
    put(state, 0x70, 'I', GHOST)
    surface = put(bytearray(r.bytes(0x40)), 0x24, 'I', PITCH)
    put(surface, 0, 'II', PITCH, ROWS)
    regions = {RAM: gauge, STATE: state, SURFACE: surface, BITS: r.bytes(PITCH * ROWS), REMAP: r.bytes(0x100), GHOST: r.bytes(0x10000)}
    return Call(at, [RAM], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    at, n = CALLS[op]
    if op.startswith('Draw'):
        return draw(r, at)
    if op == 'Setup':   # (the window's corners mostly in order and on screen; the scales the screen's size or wild)
        window = bytearray(r.bytes(0x10))
        xs, ys = sorted(r.randrange(-100, 800) for _ in range(2)), sorted(r.randrange(-100, 600) for _ in range(2))
        if r.random() < 0.2:
            xs, ys = [r.getrandbits(16) for _ in range(2)], [r.getrandbits(16) for _ in range(2)]
        put(window, 4, 'HHHH', xs[0] & 0xffff, ys[0] & 0xffff, xs[1] & 0xffff, ys[1] & 0xffff)
        scales = r.choice([(480, 640), (600, 800), (768, 1024), (r.getrandbits(16), r.getrandbits(16))])
        regions = {RAM: bytearray(r.bytes(0x50)), RAM + 0x100: window, 0xfaa5c0: struct.pack('<HHHH', scales[0], r.getrandbits(16), scales[1], r.getrandbits(16)),
                   0xfa71f8: r.bytes(0x400)}
        args = [r.choice([r.randrange(-50, 700), r.getrandbits(32)]) & 0xffffffff for _ in range(n)]
        return Call(at, [RAM, RAM + 0x100] + args, regions, conv='c', ret='v')
    args = [r.choice([r.randrange(-50, 700), r.getrandbits(32)]) & 0xffffffff for _ in range(n)]
    return Call(at, [RAM] + args, {RAM: bytearray(r.bytes(0x50))}, conv='t', ret='a' if op == 'Init' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

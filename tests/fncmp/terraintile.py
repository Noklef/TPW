# The terrain tile blend (src/game/terrain/terrain_blend: 0x55f780 TerrainBlend_Tile, thiscall on the Lnd) against the original: an Lnd of
# 2 to 3 x 2 to 3 tiles at a tile size of 16 or 32 (now and then 8 or 64), its control map's cells mostly on, weights random or at the ends,
# the level in use's 13 textures a little larger than a tile (the window wraps); the game's five ramps (0x8793a8-0x8798a8) built from random
# (centre, width) pairs as Lnd_Read builds them, or random bytes; the weight maps (0x8798a8 W1, 0x8798b8 W2, 0x879398 W3) at the tile size
# or another (made over through the heap); the scratch (0x8798c8) now and then none; an overlay of ARGB4444 / ARGB8888 / other pixels, turned
# and mirrored by its flags, or none.
# usage: terraintile.py [CASES [SEED]] [--wild|--both] [--show N]   (env SIZES=24,16: the tile sizes, in turn)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

B = Bench([(0x55f780, 0x560ff0), (0x55e780, 0x55eaa0), (0x55eaa0, 0x55f060), (0x5e7840, 0x5e78b0), (0x67acb0, 0x67acb1)])
LND, OVERLAY, OUT, SCRATCH = RAM, RAM + 0x300, RAM + 0x1000, RAM + 0x5000
GLOBALS = 0x879398   # W3, the five ramps (0x8793a8 select B, 0x8794a8 select C, 0x8795a8 A, 0x8796a8 B, 0x8797a8 C), W1, W2, the scratch
SIZES = [int(k) for k in os.environ.get('SIZES', '').split(',') if k]
TEXTURES = (4, 4, 1, 4, 4, 1, 1, 1, 4, 4, 1, 1, 1)   # the level's: colour pair A, mask A, B's, select B pair, C's, select C pair


def ramp(centre, width):
    """Lnd_BuildRamp (0x55e710)."""
    half = (width + 1) >> 1
    lo, hi = (centre - half + 0x8000) % 0x10000 - 0x8000, (centre + half + 0x8000) % 0x10000 - 0x8000
    return bytes(0 if i < lo else 0xff if i > hi else ((i * 255 - lo * 255) // (width + 1)) & 0xff for i in range(256))


def case(r):
    size = r.deal(SIZES) if SIZES else r.choice([16, 16, 16, 32, 32, 8, 64]) if r.random() < 0.97 else r.choice([24, 4])
    tx, ty = r.choice([2, 3]), r.choice([2, 3])
    level = r.randrange(3)
    regions = {HEAP: heap()}
    free = [RAM + 0x20000]

    def place(data):
        at = free[0]
        regions[at] = data
        free[0] = (at + len(data) + 0x10f) & ~0xff   # (0x10 of slack read past a window's end now and then)
        return at

    cw, ch = 4 * tx + 1, 4 * ty + 1
    control = bytearray(r.bytes(cw * ch * 4))
    on = r.random()
    for i in range(cw * ch):
        if r.random() < on + 0.3:
            control[4 * i] = 0xff
        for k in (1, 2, 3):
            if r.random() < 0.4:
                control[4 * i + k] = r.choice([0, 0xff])
    lnd = bytearray(r.bytes(0x2c0))
    put(lnd, 0, 'BB', size, level)
    put(lnd, 0xc, '2I', tx, ty)
    put(lnd, 0x14, '4I', cw, ch, 4, place(bytes(control)))
    for t, bpp in enumerate(TEXTURES):
        w, h = size + r.choice([1, 3, size // 2 or 1, size]), size + r.choice([1, 2, size // 2 or 1, size])
        if bpp == 1 and r.random() < 0.3:   # (masks and selectors at their ends)
            data = bytes(r.choice([0, 0xff]) for _ in range(w * h))
        else:
            data = r.bytes(w * h * bpp + 0x10)
        put(lnd, 0x24 + 0xd8 * level + 8 + 0x10 * t, '4I', w, h, bpp, place(data))
    regions[LND] = lnd

    g = bytearray(r.bytes(0x534))
    pairs = [(r.randrange(256), r.choice([0, 1, 16, 64, 255, r.randrange(256)])) for _ in range(5)]
    for k, table in enumerate((2, 4, 0, 1, 3)):   # (0x8793a8 + 0x100 k: the ramps in the order A, B, select B, C, select C at index table)
        if r.random() < 0.9:
            g[0x10 + 0x100 * k:0x110 + 0x100 * k] = ramp(*pairs[table])
    for off in (0, 0x510, 0x520):   # W3, W1, W2
        if r.random() < 0.9:
            put(g, off, '4I', size, size, 1, place(r.bytes(size * size)))
        else:
            put(g, off, '4I', r.choice([size * 2, 0]), size, 1, 0)
    put(g, 0x530, 'I', SCRATCH if r.random() < 0.97 else 0)
    regions[GLOBALS] = g
    regions[OUT] = r.bytes(size * size * 4)
    regions[SCRATCH] = r.bytes(size * size * 4)

    overlay = 0
    if r.random() < 0.6:
        bpp = r.choice([2, 4, 4, 3])
        ow, oh = r.choice([size, size * 2, size + 3]), r.choice([size, size * 2, size + 5])
        pixels = place(r.bytes(ow * oh * max(bpp, 2))) if r.random() < 0.9 else 0
        flags = 0
        for bit in (0x08, 0x10, 0x20, 0x40):
            if r.random() < 0.3:
                flags |= bit
        regions[OVERLAY] = struct.pack('<5I', pixels, ow, oh, bpp, flags | r.getrandbits(32) & 0xffffff00 & -(r.random() < 0.3))
        overlay = OVERLAY
    return Call(0x55f780, [LND, OUT, r.randrange(tx), r.randrange(ty), overlay], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

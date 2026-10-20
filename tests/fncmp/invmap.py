# The inverse colour map (src/game/gfx/invmap: 0x642150-0x64285b, thiscall) against the originals: Build over 1 to 8 palette colours at 0 to 4 bits
# a channel (a cube of up to 16^3 cells), MaxFill, the destructors (flags any, freed into the heap region), and each loop called on its own from the
# state Build sets for a colour, on a distance buffer either part-built by the original's Build of the colours before it or random, the loops' statics
# (0xfaa500) as the outer loop leaves them or random within the cube.
# usage: invmap.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,DelDtor,Dtor,Build,Red,Green,Blue,MaxFill)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

B = Bench([(0x642150, 0x64285c), (0x67acb0, 0x67acb1)])
OBJ, PAL, DIST, MAP = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x6000
STATICS = 0xfaa500
CALLS = {'Ctor': 0x642150, 'DelDtor': 0x642160, 'Dtor': 0x642180, 'Build': 0x642190, 'Red': 0x642340, 'Green': 0x642460, 'Blue': 0x6426b0,
         'MaxFill': 0x642840}
M = 0xffffffff


def palette(r):
    """Up to 256 colours R, G, B, x: random, or a few repeated (ties), or near the cube's corners."""
    pal = bytearray(r.bytes(0x400))
    if r.random() < 0.3:
        for i in range(0, 0x400, 4):
            pal[i:i + 4] = pal[(i % 12):(i % 12) + 4]
    elif r.random() < 0.3:
        for i in range(0, 0x400):
            if i % 4 != 3:
                pal[i] = r.choice([0, 1, 0x7f, 0x80, 0xfe, 0xff, r.randrange(0, 0x100, 0x10)])  # (on cell edges: ties between colours)
    return pal


def setup(bits, pal, index):
    """The object's fields as Build sets them before its RedLoop for colour index (struct offsets +4 .. +0x54)."""
    shift = (8 - bits) & 31
    cm = 1 << bits
    x, xsqr = 1 << shift, 1 << ((2 * shift) & 31)
    p = pal[4 * index:4 * index + 3]
    rc, gc, bc = p[0] >> shift, p[1] >> shift, p[2] >> shift
    half = x // 2
    rd, gd, bd = (p[0] - x * rc - half) & M, (p[1] - gc * x - half) & M, (p[2] - bc * x - half) & M
    cd = (rd * rd + gd * gd + bd * bd) & M
    ri, gi, bi = ((rc + 1) * xsqr - x * p[0]) * 2 & M, ((gc + 1) * xsqr - p[1] * x) * 2 & M, ((bc + 1) * xsqr - p[2] * x) * 2 & M
    cell = rc * cm * cm + bc + gc * cm
    f = {4: index, 8: bc, 0xc: gc, 0x10: rc, 0x14: gd, 0x18: rd, 0x1c: cd, 0x20: bi, 0x24: gi, 0x28: ri, 0x34: DIST + 4 * cell, 0x40: MAP + cell,
         0x44: cm, 0x48: cm * cm, 0x4c: x, 0x50: xsqr, 0x54: cm}
    return f, (rc, gc, bc)


def dist(r, cells):
    k = r.random()
    if k < 0.3:
        return bytearray(b'\xff' * 4 * cells)
    out = bytearray()
    for _ in range(cells):
        out += struct.pack('<I', M if r.random() < 0.3 else r.randrange(0, 0x30000) if k < 0.8 else r.getrandbits(32))
    return out


def case(r):
    op = r.deal(OPS)
    address = CALLS[op]
    obj = bytearray(r.bytes(0x58))
    regions = {OBJ: obj}
    if op in ('Ctor', 'Dtor'):
        return Call(address, [OBJ], regions, conv='t', ret='a' if op == 'Ctor' else 'v')
    if op == 'DelDtor':
        regions[HEAP] = heap()
        return Call(address, [OBJ, r.choice([0, 1, 1, 2, 3, M, r.getrandbits(32)])], regions, conv='t', ret='a')
    bits = r.choice([0, 1, 2, 2, 3, 3, 3, 4])
    cm = 1 << bits
    cells = cm ** 3
    if op == 'MaxFill':
        n = r.choice([0, 1, 2, 3, 4, 5, -1 & M, -2 & M, 0x80000000])
        size = n ** 3 if n < 6 else 0
        regions[DIST] = r.bytes(4 * size + 8)
        return Call(address, [DIST, n], regions, conv='s', ret='v')
    pal = palette(r)
    regions.update({PAL: pal, DIST: dist(r, cells) + r.bytes(8), MAP: r.bytes(cells + 8), STATICS: r.bytes(0x34)})
    if op == 'Build':
        first = r.choice([0, 0, 1, 5, 250, 255])
        count = r.choice([0, 1, 2, 3, 4, 6, 8, -1 & M])
        count = min(count, 256 - first) if count != M else count
        return Call(address, [OBJ, first, count, PAL, bits, DIST, MAP], regions, conv='t', ret='v')
    index = r.randrange(8)
    if index and r.random() < 0.5:  # the distances and map as Build left them for the colours before
        before = Call(0x642190, [OBJ, 0, index, PAL, bits, DIST, MAP], dict(regions), conv='t', ret='v')
        out = B.original(before)
        keys = list(before.regions)
        for k in (DIST, MAP, STATICS):
            regions[k] = bytes.fromhex(out[3 + keys.index(k)])
    f, (rc, gc, bc) = setup(bits, pal, index)
    for off, v in f.items():
        put(obj, off, 'I', v)
    st = bytearray(regions[STATICS])
    if op == 'Red':
        return Call(address, [OBJ], regions, conv='t', ret='a')
    restart = r.choice([0, 1, 1, r.getrandbits(32)])
    plane = r.randrange(cm)
    if op == 'Green':
        here = gc
        if not restart:  # greenHere / greenMin / greenMax / greenInc from an earlier plane
            lo = r.randrange(cm)
            hi = r.randrange(lo, cm)
            here = r.randrange(cm)
            put(st, 0x24, 'I', here)
            put(st, 0x14, 'I', lo)
            put(st, 0x30, 'I', hi)
            put(st, 0x28, 'I', r.choice([f[0x24], r.getrandbits(32)]))
        cell = plane * cm * cm + here * cm + bc
        put(obj, 0x30, 'I', DIST + 4 * cell)
        put(obj, 0x3c, 'I', MAP + cell)
        put(obj, 0x18, 'I', r.choice([f[0x1c], r.randrange(0x30000), r.getrandbits(32)]))
        regions[STATICS] = st
        return Call(address, [OBJ, restart], regions, conv='t', ret='a')
    here = bc
    if not restart:  # blueHere / blueMin / blueMax / blueInc from an earlier row
        lo = r.randrange(cm)
        hi = r.randrange(lo, cm)
        here = r.randrange(cm)
        put(st, 0xc, 'I', here)
        put(st, 0x8, 'I', lo)
        put(st, 0x18, 'I', hi)
        put(st, 0x20, 'I', r.choice([f[0x20], r.getrandbits(32)]))
    cell = plane * cm * cm + r.randrange(cm) * cm + here
    put(obj, 0x2c, 'I', DIST + 4 * cell)
    put(obj, 0x38, 'I', MAP + cell)
    put(obj, 0x14, 'I', r.choice([f[0x1c], r.randrange(0x30000), r.getrandbits(32)]))
    regions[STATICS] = st
    return Call(address, [OBJ, restart], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

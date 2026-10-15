# 0x448000-0x470000 (render mode, model part tables, frame globals, model holders, the terrain
# attribute grid) against the originals.
# usage: scene.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x44a590, 0x44a640), (0x44b140, 0x44b2e0), (0x44b3c0, 0x44b410), (0x44e570, 0x44e5fc),
           (0x4587b0, 0x4587d9), (0x45bed0, 0x45bf13), (0x450420, 0x45046d), (0x450a30, 0x450a78),
           (0x450b00, 0x450b24)])
HOLDER, MODEL, PARTS, FACES, FACES2 = (RAM + k * 0x1000 for k in range(5))
REF, H2, MAT, SLOTS, MAP, GRID, CELLS, GRID2, CELLS2, OUT = (RAM + k * 0x1000 for k in range(5, 15))


def mode(r, op):
    bits = r.choice([r.getrandbits(32), 1 << r.randrange(3, 15), 0, r.getrandbits(32) & 0x7838])
    if op in ('Size', 'Depth'):
        return Call({'Size': 0x44a590, 'Depth': 0x44a5d0}[op], [], {0x79d86c: put(bytearray(4), 0, 'I', bits)}, ret='a')
    return Call({'ResetScale': 0x44a600, 'Epsilon': 0x44a620}[op], [], {0x79d9e4: r.bytes(4), 0x79d9e8: r.bytes(4)})


def parts(r, op):
    """A holder (HOLDER: +0 -> HOLDER + 0x20, +8 -> HOLDER + 0x20, whose +4 is MODEL) and a model with
    +0x48 parts at +0x7c (PARTS) and faces at +0x70 (FACES, 0xa0 each) and +0x74 (FACES2, 0x58 each)."""
    n = r.choice([0, 1, r.randrange(1, 12)])
    holder, model, ps = r.bytes(0x40), r.bytes(0x80), r.bytes(12 * 0x14)
    put(holder, 0, 'I', HOLDER + 0x20)
    put(holder, 8, 'I', HOLDER + 0x20)
    put(holder, 0x24, 'I', MODEL)
    first, base = r.randrange(0, 13), r.randrange(0, 4)
    put(model, 0x44, '3H', first, base, n)
    put(model, 0x70, 'I', FACES)
    put(model, 0x74, 'I', FACES2)
    put(model, 0x7c, 'I', PARTS)
    keys = [r.getrandbits(32) for _ in range(3)]
    for i in range(12):
        put(ps, i * 0x14, 'I', r.choice([r.getrandbits(32), 1 << r.randrange(32), 0, r.getrandbits(32) & ~0x3da1f83]))
        put(ps, i * 0x14 + 4, 'I', r.choice(keys + [r.getrandbits(32)]))
        put(ps, i * 0x14 + 8, 'I', r.choice(keys + [r.getrandbits(32)]))
    mask = r.choice([r.getrandbits(32), 1 << r.randrange(32), 0, r.getrandbits(32) & ~0x3da1f83, 0x3da1f83 & r.getrandbits(32),
                     r.getrandbits(32) & ~0x3da1f82 | 1])
    key = r.choice(keys + [r.getrandbits(32)])
    regs = {HOLDER: holder, MODEL: model, PARTS: ps, FACES: r.bytes(16 * 0xa0), FACES2: r.bytes(16 * 0x58)}
    if op in ('MarkMask', 'MarkValue'):
        return Call({'MarkMask': 0x44b140, 'MarkValue': 0x44b1b0}[op], [HOLDER, mask if op == 'MarkMask' else key], regs)
    if op == 'Count':
        return Call(0x44b2b0, [HOLDER, mask], regs, ret='a')
    if op == 'Nth':
        return Call(0x44b270, [HOLDER, mask, r.choice([0, 1, 2, r.randrange(n + 2), r.getrandbits(32)])], regs, ret='a')
    return Call({'Key4': 0x44b220, 'Key8': 0x44b3c0}[op], [HOLDER, mask, key], regs, ret='a')


def frame(r, op):
    """The per-frame globals: bounds 0x7b0600, flags 0x79e884, point 0x79e248 and its flag 0x79dcd4."""
    if op == 'ResetBounds':
        return Call(0x44e570, [], {0x7b0600: r.bytes(24)})
    if op == 'AddFlags':
        return Call(0x44e5b0, [r.choice([r.getrandbits(32), 1 << r.randrange(32), 0])], {0x79e884: r.bytes(4)})
    return Call(0x44e5d0, [r.choice([REF, REF, 0])], {0x79e248: r.bytes(12), 0x79dcd4: r.bytes(4), REF: r.bytes(12)})


def holder(r, op):
    """A holder (HOLDER: +4 -> MODEL, +8 -> H2 whose +0x28 -> MAT whose +4 -> SLOTS) reached through
    REF and the ride table 0x7a4610; the model's +0x78 -> MAT (the matrix block, translation at +0x40)."""
    h, model, h2, mat = r.bytes(0x40), r.bytes(0x80), r.bytes(0x40), r.bytes(0x60)
    put(h, 4, 'I', MODEL)
    put(h, 8, 'I', H2)
    put(model, 0x78, 'I', MAT)
    put(h2, 0x28, 'I', MAT)
    i = r.choice([0, 1, r.randrange(0x30)])
    slots = r.choice([SLOTS, SLOTS, (-i * 0x14) & 0xffffffff])  # the last comes to null
    put(mat, 4, 'I', slots)
    ride = r.randrange(4)
    table = bytearray(r.bytes(16))
    put(table, 4 * ride, 'I', HOLDER)
    regs = {HOLDER: h, MODEL: model, H2: h2, MAT: mat, REF: put(bytearray(4), 0, 'I', HOLDER), 0x7a4610: table,
            SLOTS: r.bytes(0x400)}
    if op == 'FlagModel':
        return Call(0x45bed0, [REF], regs)
    if op == 'SetPosition':
        return Call(0x45bef0, [HOLDER] + [r.getrandbits(32) for _ in range(3)], regs)
    return Call(0x4587b0, [ride, i], regs, ret='a')


def land(r, op):
    """The terrain map (0x7a0854 -> MAP, +0x6c -> GRID: width +0x18, height +0x1c, count +6 (u16),
    cells +0x2c -> CELLS) and a second grid (GRID2, cells CELLS2)."""
    m, g, g2 = r.bytes(0x80), bytearray(r.bytes(0x40)), r.bytes(0x40)
    w, h = r.choice([0, 1, r.randrange(1, 17)]), r.choice([0, 1, r.randrange(1, 17)])
    put(g, 0x18, '2I', w, h)
    put(g, 6, 'H', r.choice([0, 1, r.randrange(1, 0x3ff)]))
    put(g, 0x2c, 'I', r.choice([CELLS, CELLS, 0]))
    put(g2, 0x2c, 'I', r.choice([CELLS2, CELLS2, 0]))
    put(m, 0x6c, 'I', r.choice([GRID, GRID, GRID, 0]))
    regs = {MAP: m, GRID: g, GRID2: g2, CELLS: r.bytes(0x1000), CELLS2: r.bytes(0x1000), OUT: r.bytes(8),
            0x7a0854: put(bytearray(4), 0, 'I', r.choice([MAP, MAP, MAP, 0]))}
    if op == 'IsParkLand':
        x = r.choice([r.randrange(w + 2), r.getrandbits(32)])
        y = r.choice([r.randrange(h + 2), r.getrandbits(32)])
        if w and h and r.random() < 0.8:
            # Inside, or one past the right / bottom edge (x == width still lands on a real cell).
            x, y = r.choice([(r.randrange(w), r.randrange(h))] * 2 + [(w, r.randrange(h)), (r.randrange(w), h)])
        put(g, 0x2c, 'I', CELLS)
        return Call(0x450420, [x, y], regs, ret='a')
    if op == 'CopyAttrs':
        return Call(0x450a30, [r.choice([MAP, MAP, MAP, 0]), GRID2], regs)
    put(regs[0x7a0854], 0, 'I', MAP)
    put(m, 0x6c, 'I', GRID)
    return Call(0x450b00, [OUT, OUT + 4], regs)


KINDS = {'Size': mode, 'Depth': mode, 'ResetScale': mode, 'Epsilon': mode, 'MarkMask': parts, 'MarkValue': parts,
         'Count': parts, 'Nth': parts, 'Key4': parts, 'Key8': parts, 'ResetBounds': frame, 'AddFlags': frame,
         'SetPoint': frame, 'FlagModel': holder, 'SetPosition': holder, 'SlotKey': holder, 'IsParkLand': land,
         'CopyAttrs': land, 'GridSize': land}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    op = r.choice(OPS)
    return KINDS[op](r, op)


if __name__ == '__main__':
    B.main(case)

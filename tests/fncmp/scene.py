# 0x448000-0x470000 (render mode, model part tables, frame globals, model holders, the terrain
# attribute grid, procedural terrain textures, painting cells, texture remaps, the highlight,
# flashing holders, the GlobalAlloc wrappers, id tables, part states and their matrices, with
# the matrix helpers they use)
# against the originals.
# usage: scene.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, fb, global_heap, heap, put  # noqa: E402

B = Bench([(0x44a590, 0x44b140), (0x44b140, 0x44b2e0), (0x44b3c0, 0x44b410), (0x44e570, 0x44e5fc),
           (0x4587b0, 0x4587d9), (0x45bed0, 0x45bf13), (0x450420, 0x45046d), (0x4508b0, 0x4508f1), (0x450950, 0x450a24), (0x450a30, 0x450a78), (0x466f90, 0x46702f),
           (0x450b00, 0x450b24), (0x457a00, 0x457a88), (0x46de00, 0x46e125), (0x56dfb0, 0x56dfdd),
           (0x45d620, 0x45d68c), (0x441840, 0x4418cb), (0x469180, 0x469405), (0x469470, 0x4698ef),
           (0x45b9d0, 0x45ba15), (0x468760, 0x4688a7), (0x44b4c0, 0x44b505),
           (0x475550, 0x47558b), (0x46f150, 0x46f180), (0x46f240, 0x46f350), (0x46f420, 0x46f5c0),
           (0x4702d0, 0x470630), (0x473a40, 0x473c30), (0x67b410, 0x67b440), (0x450e60, 0x450e99),
           (0x45ba70, 0x45ba92), (0x45bdc0, 0x45bdca), (0x44e510, 0x44e531), (0x473c70, 0x473e30),
           (0x429ee0, 0x429fd4), (0x429fe0, 0x429ff4), (0x45bf20, 0x45bf46), (0x45bf50, 0x45bf83),
           (0x46fad0, 0x46fb41), (0x457e10, 0x457e26), (0x457e30, 0x457e63), (0x452e10, 0x452e63),
           (0x45ba20, 0x45ba63)], sines=256)
HOLDER, MODEL, PARTS, FACES, FACES2 = (RAM + k * 0x1000 for k in range(5))
REF, H2, MAT, SLOTS, MAP, GRID, CELLS, GRID2, CELLS2, OUT = (RAM + k * 0x1000 for k in range(5, 15))
SETS, ENTRIES, TILES, MDL, MESHES, MFACES, REMAP = (RAM + k * 0x1000 for k in range(15, 22))
HL, VERTS, HLBUF, SAVED, IDS = (RAM + k * 0x1000 for k in range(22, 27))
PSA, PSF, PSN, PSM = (RAM + k * 0x1000 for k in range(27, 31))
PMA, PMN, PMG, PMS = (RAM + k * 0x1000 for k in range(31, 35))
LA, LH, VEC = (RAM + k * 0x1000 for k in range(35, 38))


def partstates(r, op):
    """An instance (PSA: +4 -> model at PSA + 0x100, +0x28 -> states at PSA + 0x200, entries at
    PSA + 0x400) whose model has n parts (PSA + 0x300) with faces base.. in two arrays (PSF: the
    first `first` 0xa0 bytes each, then 0x58 at PSF + 0xa00; each face's +4 -> PSM) and a node
    tree in PSN (0x60 apart; +8 sibling, +0xc child, +0x50 face index); heap() for GlobalAlloc."""
    n, base = r.choice([0, 1, r.randrange(1, 13)]), r.randrange(0, 5)
    first = r.randrange(base + n + 1)
    psa, psf = bytearray(r.bytes(0x1000)), bytearray(r.bytes(0x1000))
    put(psa, 4, 'I', PSA + 0x100)
    put(psa, 0x28, 'I', PSA + 0x200)
    put(psa, 0x144, '3H', first, base, n)
    put(psa, 0x170, '4I', PSF, PSF + 0xa00, PSN, PSA + 0x300)
    bits = [2, 8, 0x10, 0x20, 0x40, 0x10000, 0x40000, 0x30]
    for i in range(12):
        flags = r.getrandbits(32) if r.random() < 0.2 else sum(r.sample(bits, r.randrange(4)))
        put(psa, 0x300 + 0x14 * i, 'I', flags)
    for k in range(base + n):
        f = k * 0xa0 if k < first else 0xa00 + (k - first) * 0x58
        put(psf, f, 'I', r.getrandbits(32) & ~0x20000400 | r.choice([0, 0, 0x400, 0x20000000]))
        put(psf, f + 4, 'I', PSM + 4 * k)
    put(psa, 0x200, '5I', r.getrandbits(32), PSA + 0x400, r.choice([0, 1, 2, r.getrandbits(32)]), r.randrange(8),
        r.choice([HEAP + 0x40, 0, r.getrandbits(32)]))
    if op == 'PsFree':
        put(psa, 0x204, 'I', r.choice([PSA + 0x400, HEAP + 0x80, 0]))
        put(psa, 0x28, 'I', r.choice([PSA + 0x200, PSA + 0x200, 0]))
    for i in range(12):
        put(psa, 0x40c + 0x14 * i, 'i', r.choice([-1, 0, r.randrange(1, 6), r.getrandbits(31)]))
    psn = bytearray(r.bytes(0x1000))
    count = r.choice([1, r.randrange(1, 11)])
    parent = {k: r.randrange(k) for k in range(1, count)}
    kids = {}
    for k in range(1, count):
        kids.setdefault(parent[k], []).append(k)
    for k in range(count):
        sib = kids[parent[k]] if k else [0]
        j = sib.index(k)
        nxt = sib[j + 1] if j + 1 < len(sib) else None
        child = kids.get(k, [None])[0]
        put(psn, 0x60 * k + 8, '2I', PSN + 0x60 * nxt if nxt else 0, PSN + 0x60 * child if child else 0)
        put(psn, 0x60 * k + 0x50, 'H', r.choice([r.randrange(max(base - 2, 0), base + n + 3), r.getrandbits(16)]))
    regs = {PSA: psa, PSF: psf, PSN: psn, PSM: r.bytes(0x100), HEAP: heap(), **global_heap()}
    if op == 'PsCreate':
        return Call(0x44a870, [PSA], regs)
    if op == 'PsMark':
        return Call(0x44aa10, [PSA, PSN + 0x60 * r.randrange(count), PSA + 0x100], regs, ret='a')
    if op == 'PsFree':
        return Call(0x44aae0, [PSA], regs)
    return Call(0x44b4c0, [PSA, r.randrange(max(n, 1))], regs)


def attach(r, buf, off):
    """An attachment at buf + off: face < 16, angles 0x20 back, t and s mostly in 0..1, a lift."""
    put(buf, off - 0x20, '3I', r.f(7), r.f(7), r.f(7))
    put(buf, off, '2H', r.choice([r.randrange(16), r.randrange(16), r.getrandbits(4)]), 0x20)
    put(buf, off + 4, '3I', r.pos(1) if r.random() < 0.8 else r.f(), r.pos(1) if r.random() < 0.8 else r.f(), r.f())


def partmatrices(r, op):
    """PsUpdate: an instance (PMA: +4 -> model at PMA + 0x100, +0x28 -> states at PMA + 0x200,
    entries at PMA + 0x400, slots in PMS) whose model has n parts (PMA + 0x300; attachments from
    PMA + 0x820, 0x40 apart) and a node tree in PMN (0x100 apart; +4 a host node, +0x10 a
    matrix; all share the geometry in PMG). PsPoint / PsAim call the attachment helpers alone."""
    pma = bytearray(r.bytes(0x1000))
    if op in ('MxPreMul', 'MxPreMul3'):
        r.floats(pma, range(0, 0x80, 4), r.choice([1, 10, None]))
        b = PMA + r.choice([0x40, 0x40, 0x40, 0])
        return Call({'MxPreMul': 0x4702d0, 'MxPreMul3': 0x4704c0}[op], [PMA, b], {PMA: pma})
    if op in ('MxRotX4', 'MxRotY4', 'MxRotZ4'):
        r.floats(pma, range(0, 0x40, 4), r.choice([1, 10, None]))
        if r.random() < 0.2:  # NaNs that differ only in sign: which one an addition keeps
            for o in range(0, 0x40, 4):
                put(pma, o, 'I', r.choice([r.f(), 0x7fc00000, 0xffc00000]))
        return Call({'MxRotX4': 0x46f240, 'MxRotY4': 0x46f420, 'MxRotZ4': 0x46f4f0}[op], [PMA, r.f(1), r.f(1)],
                    {PMA: pma})
    if op == 'PsPoint':
        r.floats(pma, range(0, 0x30, 4))
        attach(r, pma, 0x100)
        return Call(0x44b040, [PMA, PMA + 12, PMA + 24, PMA + 36, PMA + 0x100, PMA + 0x200, PMA + 0x20c], {PMA: pma})
    if op == 'PsAim':
        r.floats(pma, range(0, 0x24, 4))
        if r.random() < 0.15:
            pma[12:24] = pma[0:12]
        if r.random() < 0.1:  # signalling NaNs in up: fld/fstp quiets them, a copy would not
            put(pma, 24 + 4 * r.randrange(3), 'I', r.choice([0x7f800001, 0xff812345]))
        r.floats(pma, range(0x200, 0x240, 4))
        attach(r, pma, 0x100)
        return Call(0x44a640, [PMA, PMA + 12, PMA + 24, PMA + 0x100, PMA + 0x200], {PMA: pma})
    n, base = r.randrange(1, 13), r.randrange(0, 5)
    put(pma, 0, 'I', r.getrandbits(32) | r.choice([1, 1, 1, 0]))
    put(pma, 4, 'I', PMA + 0x100)
    put(pma, 0x28, 'I', PMA + 0x200)
    put(pma, 0x146, 'H', base)
    put(pma, 0x178, '2I', PMN, PMA + 0x300)
    put(pma, 0x200, '2I', r.getrandbits(32) | r.choice([1, 1, 1, 0]), PMA + 0x400)
    bits = [0x40, 0x40000, 0x40040, 0x40040, 0x10, 0x20]
    for i in range(n):
        put(pma, 0x300 + 0x14 * i, 'I', r.getrandbits(32) if r.random() < 0.1 else sum(r.sample(bits, r.randrange(3))))
        put(pma, 0x30c + 0x14 * i, 'I', PMA + 0x820 + 0x40 * i)
        attach(r, pma, 0x820 + 0x40 * i)
        put(pma, 0x400 + 0x14 * i, '2I', r.choice([0, 0x20, 0x21, 0x21, 0x22, 0x28, 0x24, 1, r.getrandbits(32)]), PMS + 0x40 * i)
    pmg = bytearray(r.bytes(0x1000))
    r.floats(pmg, range(0, 0x180, 4))
    for k in range(32):
        put(pmg, 0x200 + 2 * k, 'H', r.randrange(32))
    for k in range(16):
        put(pmg, 0x300 + 8 * k, '4H', r.randrange(16) | r.choice([0, 0x8000]), r.randrange(32), r.randrange(32),
            r.randrange(32))
    r.floats(pmg, range(0x400, 0x4c0, 4), 1)
    pmn = bytearray(r.bytes(0x1000))
    count = r.choice([1, r.randrange(1, 13)])
    parent = {k: r.randrange(k) for k in range(1, count)}
    kids = {}
    for k in range(1, count):
        kids.setdefault(parent[k], []).append(k)
    nodebits = [0x20000000, 0x20000000, 0x10000000, 0x10000, 0x40000000, 0x200000, 0x200000]
    for k in range(count):
        o = 0x100 * k
        sib = kids[parent[k]] if k else [0]
        j = sib.index(k)
        nxt = sib[j + 1] if j + 1 < len(sib) else None
        child = kids.get(k, [None])[0]
        put(pmn, o, '2I', sum(r.sample(nodebits, r.randrange(5))), PMN + 0x100 * r.randrange(count))
        put(pmn, o + 8, '2I', PMN + 0x100 * nxt if nxt else 0, PMN + 0x100 * child if child else 0)
        r.floats(pmn, range(o + 0x10, o + 0x50, 4), r.choice([1, 10, None]))
        put(pmn, o + 0x50, 'H', base + r.randrange(n) if r.random() < 0.95 else r.getrandbits(16) % (base + 13))
        put(pmn, o + 0x5c, 'H', r.randrange(17))
        put(pmn, o + 0x60, '2I', PMG, PMG + 0x400)
        put(pmn, o + 0x70, 'I', PMG + 0x300)
        put(pmn, o + 0x94, 'I', PMG + 0x200)
    return Call(0x44ab30, [PMA], {PMA: pma, PMN: pmn, PMG: pmg, PMS: r.bytes(0x400)})


STATICS = {'HolderOne': 0x45bf50, 'HolderEps': 0x45bf70, 'TerrOne': 0x457e30, 'TerrEps': 0x457e50,
           'Tolerances': 0x452e10, 'ScratchOne': 0x452e30, 'ScratchEps': 0x452e50, 'GmFlag': 0x45ba20,
           'GmOne': 0x45ba30, 'GmEps': 0x45ba50}


def mode(r, op):
    bits = r.choice([r.getrandbits(32), 1 << r.randrange(3, 15), 0, r.getrandbits(32) & 0x7838])
    if op in ('Size', 'Depth'):
        return Call({'Size': 0x44a590, 'Depth': 0x44a5d0}[op], [], {0x79d86c: put(bytearray(4), 0, 'I', bits)}, ret='a')
    if op in STATICS:  # the units' unused statics (model/holder, terrain/procedural, model/scratch)
        return Call(STATICS[op], [], {a: r.bytes(4) for a in (0x7a1cac, 0x7a1cdc, 0x7a0b28, 0x7a0b48, 0x79f7dc,
                                                                0x79fb1c, 0x79fb24, 0x79fb28, 0x7a1c60, 0x7a1c64,
                                                                0x7a1c68)})
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
    if op.startswith('Slot'):  # model/slots: the slot table 0x790398 (0xb0 each, +4 the holder)
        slot = r.randrange(4)
        table = bytearray(r.bytes(4 * 0xb0))
        put(table, slot * 0xb0 + 4, 'I', HOLDER)
        regs[0x790398] = table
        if n and r.random() < 0.7:  # a part with the key and flag 0x400
            i = r.randrange(n)
            put(ps, i * 0x14, 'I', r.getrandbits(32) | 0x400)
            put(ps, i * 0x14 + 4, 'I', key)
        if op == 'SlotHolder':
            return Call(0x429fe0, [slot], regs, ret='a')
        return Call(0x429ee0 if op == 'SlotClear' else 0x429f60, [key, slot], regs)
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
    if op == 'SetScale':  # the matrix's axes (MAT + 0x10) scaled by the vector at VEC
        regs[VEC] = struct.pack('<3I', r.f(), r.f(), r.f())
        return Call(0x45bf20, [HOLDER, VEC], regs)
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
    if op == 'Cross':  # two vectors at OUT, OUT + 12; the result over either, or apart
        regs[OUT] = r.floats(bytearray(36), range(0, 36, 4), r.choice([1, 100, None]))
        a, b = r.choice([(OUT, OUT + 12), (OUT + 12, OUT)])
        return Call(0x4508b0, [a, b, r.choice([OUT + 24, OUT + 24, OUT, OUT + 12, OUT + 4])], regs)
    if op == 'CopyAttrs':
        return Call(0x450a30, [r.choice([MAP, MAP, MAP, 0]), GRID2], regs)
    if op == 'RestoreCells':  # the loaded grid 0x79fc54 -> GRID2
        regs[0x79fc54] = struct.pack('<I', GRID2)
        return Call(0x457e10, [], regs)
    if op == 'Footprint':  # (x*, y*, angle, outX*, outY*, width, height)
        regs[OUT] = r.bytes(16)
        angle = r.choice([0, 90, 180, 270, 90, 180, 270, 45, 360, r.getrandbits(32)])
        return Call(0x466f90, [OUT, OUT + 4, angle, OUT + 8, OUT + 12, r.randrange(-2, 9) & 0xffffffff, r.randrange(-2, 9) & 0xffffffff], regs)
    if op == 'CloneAttrs':  # the grid's +4 dwords at +0x28 (CELLS + 0x800) and +6 at +0x2c
        put(g, 4, 'H', r.choice([0, 1, r.randrange(1, 0x1ff)]))
        put(g, 6, 'H', r.choice([0, 1, r.randrange(1, 0x3ff)]))
        put(g, 0x28, 'I', r.choice([CELLS + 0x800, CELLS + 0x800, 0]))
        regs.update({CELLS: r.bytes(0x1000) + r.bytes(0x800), HEAP: heap(0x4000), **global_heap()})
        return Call(0x450950, [r.choice([MAP, MAP, MAP, 0]), OUT], regs)
    put(regs[0x7a0854], 0, 'I', MAP)
    put(m, 0x6c, 'I', GRID)
    return Call(0x450b00, [OUT, OUT + 4], regs)


def terrain(r, w=None, h=None):
    """The map 0x7a0854 -> MAP (+0x6c -> GRID, cells CELLS) and the loaded grid 0x79fc54 -> GRID2
    (cells CELLS2), with a w x h grid holding w * h cells; flags 0x7a1a8c and 0x8bd550."""
    w = r.choice([1, r.randrange(1, 17)]) if w is None else w
    h = r.choice([1, r.randrange(1, 17)]) if h is None else h
    m, g, g2 = bytearray(r.bytes(0x80)), bytearray(r.bytes(0x40)), bytearray(r.bytes(0x40))
    put(g, 0x18, '2I', w, h)
    put(g, 6, 'H', r.choice([w * h, 0, 1]))
    put(g, 0x2c, 'I', CELLS)
    put(g2, 0x2c, 'I', CELLS2)
    put(m, 0x6c, 'I', GRID)
    bits = r.choice([0x6000, 0x2000, 0x4000, 0]) | (r.getrandbits(32) & ~0x6000)
    cells = bytearray(r.bytes(0x400))
    for i in range(0, 0x400, 4):  # fewer cells with 0x81 so most get changed
        if r.random() < 0.7:
            cells[i] &= 0x7e
    return {MAP: m, GRID: g, GRID2: g2, CELLS: cells, CELLS2: r.bytes(0x400),
            0x7a0854: put(bytearray(4), 0, 'I', MAP), 0x79fc54: put(bytearray(4), 0, 'I', GRID2),
            0x7a1a8c: put(bytearray(4), 0, 'I', bits), 0x78d8d8: put(bytearray(4), 0, 'I', r.choice([0, 1, 2])),
            0x8bd550: put(bytearray(4), 0, 'I', r.getrandbits(32) & r.choice([0xffffffff, 0xefffffff]))}


def procedural(r, op):
    regs = terrain(r)
    address = {'TexEnable': 0x56dfb0, 'TexDisable': 0x56dfd0, 'ProcEnter': 0x457a00, 'ProcLeave': 0x457a40,
               'ProcOn': 0x457a70}.get(op)
    if address:
        return Call(address, [], regs, ret='a' if op == 'ProcOn' else 'v')
    return Call(0x46ded0, [r.choice([0, 1, r.getrandbits(32)])], regs)


def paint(r, op):
    """PaintCell (map, sets, x, y, set, tile, rotation): sets {count, ENTRIES}, 8 entries of 12 bytes
    (+4 tile count, +8 -> TILES + 0x100 * k), tiles of 12 bytes (texture u16 at +8). Entries past
    the count may hold anything (the unwrapped set's count is checked first)."""
    w, h = r.randrange(1, 17), r.randrange(1, 17)
    regs = terrain(r, w, h)
    count = r.choice([1, 2, 3, r.randrange(1, 7), 0, -1])
    entries = bytearray(r.bytes(8 * 12))
    for k in range(8):
        tiles = r.randrange(1, 9) if k < count else r.choice([0, -1, r.randrange(1, 9)])
        put(entries, 12 * k + 4, '2I', tiles & 0xffffffff, TILES + 0x100 * k)
    regs.update({SETS: put(bytearray(8), 0, '2I', count & 0xffffffff, ENTRIES), ENTRIES: entries,
                 TILES: r.bytes(0x800)})
    if r.random() < 0.3:
        put(regs[0x79fc54], 0, 'I', r.choice([0, GRID2]))
    x = r.choice([r.randrange(w), r.randrange(w), -1, w, r.getrandbits(32)])
    y = r.choice([r.randrange(h), r.randrange(h), -1, h, r.getrandbits(32)])
    s, t = r.choice([(0, 8), (0, 0x37), (0, r.randrange(16)), (r.randrange(1, max(count, 1) + 3), r.randrange(16))])
    rotation = r.choice([0, 90, 180, 270, 45, r.getrandbits(32)])
    return Call(0x46df50, [MAP, SETS, x, y, s, t, rotation], regs)


def remap(r, op):
    """A model MDL (+0x44 meshes at +0x70 -> MESHES, 0xa0 each: +0x5a faces at +0x6c -> MFACES + 0x100 * k,
    0x10 each; +0x50 its textures; +0x6c a grid or 0) renumbered through REMAP (64 entries)."""
    base = r.getrandbits(32) & ~7
    model, meshes = bytearray(r.bytes(0x80)), bytearray(r.bytes(3 * 0xa0))
    n = r.choice([0, 1, 2, 3])
    put(model, 0x44, 'H', n)
    put(model, 0x50, 'I', base)
    put(model, 0x6c, 'I', r.choice([GRID, 0]))
    put(model, 0x70, 'I', MESHES)
    faces = bytearray(r.bytes(0x300))
    for k in range(3):
        put(meshes, 0xa0 * k + 0x5a, 'H', r.choice([0, 1, r.randrange(9)]))
        put(meshes, 0xa0 * k + 0x6c, 'I', MFACES + 0x100 * k)
        for j in range(8):
            put(faces, 0x100 * k + 0x10 * j, 'I', r.choice([0, (base + 8 * r.randrange(64) + r.randrange(8)) & 0xffffffff]))
    regs = terrain(r)
    cells = regs[CELLS]
    count = r.choice([0, 1, r.randrange(64)])
    put(regs[GRID], 6, 'H', count)
    for i in range(count):
        put(cells, 4 * i + 2, 'H', r.randrange(64))
    table = bytearray(256)
    for i in range(64):
        put(table, 4 * i, 'i', r.choice([0, -1, r.randrange(1, 100), -r.randrange(1, 100)]))
    regs.update({MDL: model, MESHES: meshes, MFACES: faces, REMAP: table})
    return Call(0x46de00, [MDL, REMAP, r.getrandbits(32), r.choice([0, 1, r.randrange(1000), -5]), r.getrandbits(32)], regs)


def ids(r, op):
    """An id table at IDS + 0x10 (0x100 entries; ids from 1) and its counters used, holes, next at
    IDS: consistent (ids 1..used + holes taken but for the holes, next the first free) or not."""
    table = [0] * 0x100
    used, holes = r.randrange(12), r.choice([0, 0, 1, r.randrange(5)])
    for i in range(1, used + holes + 1):
        table[i] = r.getrandbits(32) | 1
    for i in r.sample(range(1, used + holes + 1), min(holes, used + holes)):
        table[i] = 0
    free = [i for i in range(1, 0x40) if not table[i]]
    nxt = free[0]
    if r.random() < 0.3:
        used, holes, nxt = r.randrange(16), r.randrange(6), r.randrange(1, 24)
    if r.random() < 0.1:
        table[r.randrange(1, 24)] ^= r.getrandbits(32)
    region = struct.pack('<4I', used, holes, nxt, 0) + struct.pack('<256I', *table)
    regs = {IDS: region}
    a = [IDS, IDS + 4, IDS + 8]
    if op == 'IdAdd':
        cap = r.choice([used, used + 1, r.randrange(20), 0xff])
        return Call(0x468760, a + [cap, IDS + 0x10, r.getrandbits(32), r.choice([-1, -1, -1, r.randrange(24)])], regs, ret='a')
    taken = [i for i in range(1, 24) if table[i]]
    victim = r.choice(taken) if taken and r.random() < 0.8 else r.randrange(24)
    return Call(0x468810, a + [IDS + 0x10, victim], regs)


def globalmem(r, op):
    if op == 'GAlloc':
        regs = {HEAP: heap(), **global_heap()}
        return Call(0x45b9d0, [r.choice([0, 1, 4, r.randrange(0x1000)]), r.choice([0, 1, r.getrandbits(32)])], regs, ret='a')
    if op == 'GCounted':
        return Call(0x45ba70, [r.choice([0, 1, 7, r.getrandbits(32)])], {HEAP: heap(), **global_heap()}, ret='a')
    return Call(0x45ba00, [r.choice([0, r.getrandbits(32)])], {HEAP: heap(), **global_heap()}, ret='a')


def leaves(r, op):
    """Small leaves: the vertex scratch buffer (count 0x79f7d0, buffer 0x79f7e0), the holder's
    awake flag, and the frame start (flags 0x79e884; an animator at 0x79fcb4 that only ticks:
    flag 4, no layers active (+0xa8), its holder (+8) with nothing to attach (+0x28))."""
    scratch = put(bytearray(r.bytes(0x14)), 0, 'I', r.choice([0, 1, 5, 100, r.randrange(0x300)]))
    put(scratch, 0x10, 'I', r.choice([0, r.getrandbits(32)]))
    regs = {HEAP: heap(), **global_heap(), 0x79f7d0: scratch}
    if op == 'VScratch':
        return Call(0x450e60, [], regs)
    if op == 'VScratchFree':
        return Call(0x450e80, [], regs)
    if op == 'Awake':
        return Call(0x45bdc0, [LH], {LH: r.bytes(8)}, ret='a')
    anim = put(bytearray(r.bytes(0x100)), 4, 'I', r.getrandbits(32) & ~0x20000000 | 4 | r.choice([0, 0x20000000]))
    put(anim, 8, 'I', LH)
    put(anim, 0xa8, 'I', 0)
    holder = put(bytearray(r.bytes(0x40)), 0x28, 'I', 0)
    return Call(0x44e510, [], {LA: anim, LH: holder, 0x79fcb4: struct.pack('<I', r.choice([0, LA])),
                               0x79e880: r.bytes(8)})


def highlight(r, op):
    """Holders A (HL) and B (HL + 0x40): +4 flags, +8 -> HL + 0x80 / 0xc0, whose +4 -> model at
    HL + 0x100 / 0x180 (+0x30 flags, +0x44 meshes at +0x70 -> HL + 0x200 / 0x400); a mesh's +0x64
    -> VERTS + 0x100 * k. Globals: current 0x7a1ce0, target 0x7afda0, suspended 0x7afda4, buffer
    0x7afd00 -> HLBUF, saved pointers 0x7afd94 -> SAVED. Flashing: +0xd0 links from 0x7afe14; a
    model's base +0x9c is HL + 0x800 (+0x5c, meshes at +0x70 -> HL + 0xa00), itself or the other
    model; +0x5c is the base's or not; +0x4a / +0x58 counts; heap() for the GlobalAlloc slots."""
    op = op.rstrip('2')  # weighted twice
    hl = bytearray(r.bytes(0x1000))
    a, b = HL, HL + 0x40
    for k, h in enumerate((a, b)):
        o = h - HL
        flags = r.getrandbits(32) & ~0x4020000 | r.choice([0, 0x20000]) | r.choice([0, 0x4000000])
        put(hl, o + 4, '2I', flags, HL + 0x80 + 0x40 * k)
        put(hl, 0x84 + 0x40 * k, 'I', HL + 0x100 + 0x80 * k)
        model = 0x100 + 0x80 * k
        put(hl, model + 0x44, 'H', r.choice([0, 1, 2, 3]))
        put(hl, model + 0x70, 'I', HL + 0x200 + 0x200 * k)
        put(hl, model + 0x9c, 'I', r.choice([HL + 0x800] * 3 + [HL + model, HL + 0x180 - 0x80 * k]))
        put(hl, model + 0x4a, 'H', r.choice([0, 1, r.randrange(0x40)]))
        for j in range(3):
            mesh = 0x200 + 0x200 * k + 0xa0 * j
            put(hl, mesh, 'I', r.getrandbits(32) & ~0x4000 | r.choice([0, 0, 0x4000]))
            first, count = r.choice([0, 1, r.randrange(5)]), r.choice([0, 1, r.randrange(7)])
            put(hl, mesh + 0x5c, '2H', count, first)
            put(hl, mesh + 0x64, 'I', VERTS + 0x100 * (3 * k + j) if first or r.random() < 0.8 else 0)
            put(hl, mesh + 0x58, 'H', r.choice([0, 1, r.randrange(9)]))
    put(hl, 0x844, 'H', 3)
    put(hl, 0x870, 'I', HL + 0xa00)
    for j in range(3):
        put(hl, 0xa00 + 0xa0 * j + 0x64, 'I', VERTS + 0x800 + 0x100 * j if r.random() < 0.8 else 0)
    shared = r.getrandbits(32)
    for model in (0x15c, 0x1dc, 0x85c):
        put(hl, model, 'I', shared if r.random() < 0.6 else r.getrandbits(32))
    listed = r.sample([a, b], r.choice([0, 1, 2, 2]))
    for h, nxt in zip(listed, listed[1:] + [0]):
        put(hl, h - HL + 0xd0, 'I', nxt)
    who = [a, b, 0]
    if op == 'HlUnflash':
        if not listed:
            listed = [r.choice([a, b])]
            put(hl, listed[0] - HL + 0xd0, 'I', 0)
        who = listed
    regs = {HL: hl, VERTS: r.bytes(0x1000), HLBUF: r.bytes(0x1000), SAVED: struct.pack('<16I', *(VERTS + 0xc00 + 0x40 * i for i in range(16))),
            0x7a1ce0: put(bytearray(4), 0, 'I', r.choice(who)), 0x7afda0: put(bytearray(4), 0, 'I', r.choice(who)),
            0x7afda4: put(bytearray(4), 0, 'I', r.choice([0, 0, 1])), 0x7afd00: put(bytearray(4), 0, 'I', HLBUF),
            0x7afd94: put(bytearray(4), 0, 'I', SAVED), 0x7a1c7c: r.bytes(4), 0x7afdb0: r.bytes(0x60),
            0x7afe14: put(bytearray(4), 0, 'I', listed[0] if listed else 0), HEAP: heap(), **global_heap(),
            0x7afd90: struct.pack('<I', r.randrange(0x100)), 0x7afd9c: struct.pack('<I', r.randrange(0x100)),
            0x7afe10: struct.pack('<I', r.choice([0, r.randrange(0x100)])),
            0x7afd04: struct.pack('<f', r.choice([0, 1, 2, r.uniform(0, 2), r.uniform(-3, 5)])) if r.random() < 0.9 else r.bytes(4),
            0x7b497c: put(bytearray(4), 0, 'I', r.choice([fb(r.uniform(0, 0.1)), fb(r.uniform(-2, 2)), r.getrandbits(32)]))}
    regs[HL] = hl
    if op == 'HlUnflash' and r.random() < 0.5:  # a highlighted holder: released and highlighted again
        h = who[0] - HL
        put(hl, h + 4, 'I', struct.unpack_from('<I', hl, h + 4)[0] | 0x20000)
    if op == 'HlUnflash' and struct.unpack_from('<I', hl, who[0] - HL + 4)[0] & 0x20000:
        # highlighted again after its meshes take the base's pointers: a null one would read
        # near 0 (+0x5e entries in), which only the emulator survives
        for j, mesh in enumerate([0x200 + 0xa0 * j for j in range(3)] + [0x400 + 0xa0 * j for j in range(3)] +
                                 [0xa00 + 0xa0 * j for j in range(3)]):
            if not struct.unpack_from('<I', hl, mesh + 0x64)[0]:
                put(hl, mesh + 0x64, 'I', VERTS + 0x600 + 0x20 * j)
    car_args = []
    if op == 'HlCar':  # coaster/registry Coaster_FlashCar: cars (HL + 0xc00, 0xc80; +0x70 -> ride a / b)
        for h in (a, b):  # flashing (0x4000000) exactly when listed
            f = struct.unpack_from('<I', hl, h - HL + 4)[0]
            put(hl, h - HL + 4, 'I', f | 0x4000000 if h in listed else f & ~0x4000000)
        cars = [HL + 0xc00, HL + 0xc80]
        for k, car in enumerate(cars):
            put(hl, car - HL + 0x70, 'I', k)
        for off in (0x54, 0x58, 0x60, 0x64):
            put(hl, 0xe00 + off, 'I', r.choice(cars + [0]))
        put(hl, 0xe00 + 0x144, 'I', r.choice(cars + [0]))
        regs[0x7a4610] = struct.pack('<2I', a, b)
        car_args = [HL + 0xe00, r.choice([0, 1, 2, 3, r.getrandbits(32)])]
    flag = r.choice([0, 1, r.getrandbits(32)])
    calls = {'HlCar': (0x441840, car_args), 'HlUpdate': (0x469180, []), 'HlTarget': (0x469340, [r.choice(who)]), 'HlTints': (0x469350, []),
             'HlClear': (0x469470, []), 'HlCurrent': (0x45d620, []), 'HlShown': (0x45d630, [flag]),
             'HlSuspend': (0x45d660, [flag]), 'HlAlloc': (0x469510, []), 'HlFree': (0x469550, []),
             'HlFlash': (0x469580, [r.choice([a, b])]), 'HlUnflash': (0x469650, [r.choice(who)]),
             'HlPulse': (0x469840, [])}
    address, args = calls[op]
    return Call(address, args, regs, ret='a' if op == 'HlCurrent' else 'v')


KINDS = {'Size': mode, 'Depth': mode, 'ResetScale': mode, 'Epsilon': mode, 'MarkMask': parts, 'MarkValue': parts,
         'Count': parts, 'Nth': parts, 'Key4': parts, 'Key8': parts, 'SlotClear': parts, 'SlotSet': parts, 'SlotHolder': parts, 'ResetBounds': frame, 'AddFlags': frame,
         'SetPoint': frame, 'FlagModel': holder, 'SetPosition': holder, 'SetScale': holder, 'SlotKey': holder,
         'HolderOne': mode, 'HolderEps': mode, 'TerrOne': mode, 'TerrEps': mode, 'Tolerances': mode,
         'ScratchOne': mode, 'ScratchEps': mode, 'GmFlag': mode, 'GmOne': mode, 'GmEps': mode, 'RestoreCells': land, 'IsParkLand': land,
         'Cross': land, 'CopyAttrs': land, 'CloneAttrs': land, 'Footprint': land, 'GridSize': land, 'TexEnable': procedural, 'TexDisable': procedural,
         'ProcEnter': procedural, 'ProcLeave': procedural, 'ProcOn': procedural, 'ProcCells': procedural,
         'PaintCell': paint, 'PaintCell2': paint, 'Remap': remap, 'HlUpdate': highlight, 'HlUpdate2': highlight,
         'HlTarget': highlight, 'HlCar': highlight, 'HlTints': highlight, 'HlClear': highlight, 'HlCurrent': highlight,
         'HlShown': highlight, 'HlSuspend': highlight, 'HlAlloc': highlight, 'HlFree': highlight,
         'HlFlash': highlight, 'HlFlash2': highlight, 'HlUnflash': highlight, 'HlUnflash2': highlight,
         'HlPulse': highlight, 'GAlloc': globalmem, 'GFree': globalmem, 'GCounted': globalmem, 'VScratch': leaves,
         'VScratchFree': leaves, 'Awake': leaves, 'FrameBegin': leaves, 'IdAdd': ids, 'IdRemove': ids,
         'PsCreate': partstates, 'PsMark': partstates, 'PsFree': partstates, 'PsRelease': partstates,
         'PsUpdate': partmatrices, 'PsPoint': partmatrices, 'PsAim': partmatrices, 'MxPreMul': partmatrices,
         'MxPreMul3': partmatrices, 'MxRotX4': partmatrices, 'MxRotY4': partmatrices, 'MxRotZ4': partmatrices}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    op = r.choice(OPS)
    return KINDS[op](r, op)


if __name__ == '__main__':
    B.main(case)

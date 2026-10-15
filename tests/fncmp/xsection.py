# The coaster cross sections (src/game/coaster/xsection: 0x42d130-0x42eb68) against the
# original: the setup and its steps -- 20 buckets of points or index entries packed into
# GlobalMem blocks (global_heap()), the section records pointed at them, the pieces, and the
# unused points dropped. Point counts stay at most 20 and point indices below 20 (beyond, the
# original overruns a stack array; see xsection.h).
# usage: xsection.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

B = Bench([(0x42d130, 0x42eb68), (0x45b9d0, 0x45ba15), (0x45ba00, 0x45ba30), (0x43fd60, 0x4402d0)], sines=256)
COASTER, SECTIONS, SHARED, POINTS, ENTRIES, USED, TO = (RAM + 0x1000 * k for k in range(7))
SOURCE = RAM + 0x10000
# (first bucket, bucket stride, entry size)
POINT_BUCKETS, INDEX_BUCKETS = (0x100, 0x194, 20), (0x2090, 0xf4, 12)


def count(r, live, wild=True):
    n = r.randrange(0, 21) if live else 0
    if wild and r.random() < 0.03:
        n = r.choice([-1, -5, 21, 25, r.getrandbits(32)])
    return n & 0xffffffff


def index(r, below=20):
    return r.choice([0, 1, r.randrange(max(below, 1)), r.randrange(max(below, 1)), r.randrange(20)])


def source(r, op):
    """A section description; for the setup, one whose point indices stay below 20."""
    tame = op == 'Setup' and r.random() < 0.9
    src = bytearray(r.bytes(0x3c00))
    density = r.choice([0.0, 0.1, 0.3, 0.6, 1.0]) if not tame else r.choice([0.3, 0.6, 1.0])
    buckets = {}
    for name, (first, stride, size) in (('points', POINT_BUCKETS), ('indices', INDEX_BUCKETS)):
        counts = [count(r, r.random() < density, op != 'Setup') for _ in range(20)]  # the setup: at most 20 (see the top)
        if op != 'Setup' and r.random() < 0.05:  # counts that cancel out
            counts[0], counts[1] = 3, (-3) & 0xffffffff
        buckets[name] = counts
        for b, n in enumerate(counts):
            put(src, first + stride * b + stride - 4, 'I', n)
            for i in range(20):
                o = first + stride * b + size * i
                if name == 'points':
                    put(src, o + 4, 'I', r.f())
                else:
                    put(src, o, 'HxxHxxH', r.getrandbits(16), index(r), index(r))
    nsec = r.choice([0, 1, 3, r.randrange(12), r.randrange(-2, 12)] if not tame else [1, 2, 3, r.randrange(1, 12)])
    put(src, 0x3530, 'i', nsec)
    for kind in ('points', 'indices'):
        live = [b + 1 for b, n in enumerate(buckets[kind]) if n] or [1]
        buckets[kind + '_live'] = live
    for k in range(max(nsec, 0)):
        o = 0x33a0 + 0x14 * k
        pts, idx = buckets['points_live'], buckets['indices_live']
        if tame:
            put(src, o, 'ii', r.choice(pts[:2] if r.random() < 0.5 else pts), r.choice(idx))
            put(src, o + 0xc, 'i', 0)
            if r.random() < 0.2:  # second points: a bucket with the same count as the first
                first = struct_get(src, o)
                same = [b for b in pts if buckets['points'][b - 1] == buckets['points'][first - 1]]
                put(src, o + 0xc, 'i', r.choice(same))
        else:
            pick = lambda live: r.choice([1, 1, 2, r.choice(live), r.choice(live), r.randrange(1, 21), 0, -1, 25])  # noqa: E731
            put(src, o, 'ii', pick(pts), pick(idx))
            put(src, o + 0xc, 'i', r.choice([0, 0, pick(pts), r.choice(pts)]))
    pieces = r.choice([0, 1, 2, r.randrange(21)])
    put(src, 0x38a4, 'I', pieces if not (not tame and r.random() < 0.03) else r.getrandbits(32))
    for k in range(20):
        o = 0x3534 + 0x2c * k
        for j in range(8):
            put(src, o + 4 * j, 'I', r.choice([0, 0, r.f(), r.f(180), 0x80000000, 0x7fc00000]))
        top = max(nsec, 1)
        number = lambda: r.choice([1, r.randrange(1, top + 1), r.randrange(1, top + 1), 0, top + 1, r.getrandbits(32)])  # noqa: E731
        put(src, o + 0x20, 'III', number() if not tame or r.random() < 0.05 else r.randrange(1, top + 1),
            r.choice([0, 0, number()]), r.choice([0, 0, number()]))
    put(src, 0x3b00, 'II', r.choice([0, 0, 1, r.randrange(max(nsec, 1) + 2)]), r.choice([0, 0, 1, r.randrange(max(nsec, 1) + 2)]))
    return src


def struct_get(buf, off):
    return int.from_bytes(buf[off:off + 4], 'little', signed=True)


def sections(r, n):
    """n section records over the POINTS / ENTRIES pages, some sharing their points."""
    secs, points, entries = bytearray(r.bytes(0x18 * 21)), bytearray(r.bytes(0x1000)), bytearray(r.bytes(0x1000))
    lists = [POINTS + 0x190 * k for k in range(8)]
    for s in range(n):
        o = 0x18 * s
        npts, nidx = r.choice([0, 1, r.randrange(21), r.randrange(21), 20]), r.choice([0, 1, r.randrange(21)])
        put(secs, o, 'IIII', npts, nidx, r.choice(lists[:3] + lists), ENTRIES + 0xf0 * (s % 16))
        put(secs, o + 0x10, 'I', r.choice([0, 0, 0, r.getrandbits(32)]))
        for i in range(20):
            e = 0xf0 * (s % 16) + 12 * i
            if e + 12 <= len(entries):
                put(entries, e, 'IHHH', r.getrandbits(32) & ~1 | r.choice([0, 0, 0, 1]), r.getrandbits(16),
                    index(r, npts), index(r, npts))
    return secs, points, entries


EMIT = RAM + 0x20000  # Emit: section, points, second points, key, offset, min, max, object, mesh, buffers
E_SEC, E_P, E_Q, E_KEY, E_OFF, E_MIN, E_MAX, E_OBJ, E_MESH, E_POS, E_NORM, E_UV = (EMIT + 0x1000 * k for k in range(12))


def angle(r):
    return r.choice([r.f(7), r.f(7), r.f(7), 0, r.f(1e4), r.f()])


def emit(r):
    """CrossSection_Emit: at most 50 points when blending (its stack buffer)."""
    n = r.choice([0, 1, 4, 50, r.randrange(51), r.randrange(51), -1, -7])
    blend = r.random() < 0.6
    if blend and n < 0:  # the original runs off its buffer
        n = 0
    sec = bytearray(r.bytes(0x18))
    put(sec, 0, 'iIIIII', n, r.getrandbits(32), E_P, r.getrandbits(32), E_Q if blend else 0, r.f())
    if r.random() < 0.3:
        put(sec, 0x14, 'I', r.choice([0, 0x80000000, 0x7fc00000, r.f(), r.f(1e-3)]))
    pts = lambda: bytearray(b''.join(r.choice([r.f(), r.f(), r.f(10), r.f(0.01), 0]).to_bytes(4, 'little') for _ in range(5 * 50)))  # noqa: E731
    key = bytearray(r.bytes(0x20))
    lo = r.f(100)
    put(key, 0, 'I', lo)
    put(key, 0xc + 4, 'II', r.choice([r.f(100), r.f(100), lo]), r.choice([r.f(1), r.f(1), 0, 0x3f800000, r.f()]))
    start = r.choice([0, 1, 2, 3, r.randrange(40)])
    obj = bytearray(r.bytes(0x78))
    put(obj, 0x38, 'I', E_MESH)
    mesh = bytearray(r.bytes(0x20))
    put(mesh, 0x10, 'III', E_POS, E_NORM, E_UV)
    bounds = lambda: bytearray(b''.join(r.choice([r.f(), r.f(100), 0x7f7fffff, 0xff7fffff, 0x7fc00000]).to_bytes(4, 'little') for _ in range(3)))  # noqa: E731
    regions = {E_SEC: sec, E_P: pts(), E_Q: pts(), E_KEY: key, E_OFF: bytearray(b''.join(r.f(500).to_bytes(4, 'little') for _ in range(3))),
               E_MIN: bounds(), E_MAX: bounds(), E_OBJ: obj, E_MESH: mesh, E_POS: bytearray(r.bytes(0x300)),
               E_NORM: bytearray(r.bytes(0x2a0)), E_UV: bytearray(r.bytes(0x200)),
               0x792218: bytearray(E_OBJ.to_bytes(4, 'little')), 0x79c600: bytearray(start.to_bytes(4, 'little'))}
    args = [r.choice([r.f(100), lo, r.f()]), angle(r), angle(r), angle(r), E_OFF, E_MIN, E_MAX,
            r.choice([0, 1, r.getrandbits(32)]), E_SEC, E_KEY + 0xc]
    return Call(0x43fd60, args, regions)


def case(r):
    op = r.choice(OPS)
    if op == 'Emit':
        return emit(r)
    coaster = bytearray(r.bytes(0xa0))
    put(coaster, 0x40, 'I', r.f())
    regions = {COASTER: coaster, SHARED: bytearray(r.bytes(4)), HEAP: heap(0x8000), **global_heap()}
    if op in ('Points', 'Indices', 'Pieces', 'Setup'):
        regions[SOURCE] = source(r, op)
        regions[SECTIONS] = bytearray(r.bytes(0x18 * 21))
        if op == 'Points':
            return Call(0x42d6d0, [COASTER, SECTIONS, SOURCE, SHARED], regions, ret='a')
        if op == 'Indices':
            return Call(0x42e200, [COASTER, SECTIONS, SOURCE], regions, ret='a')
        if op == 'Pieces':
            n = r.choice([0, 1, 3, r.randrange(21)])
            return Call(0x42d130, [COASTER, SECTIONS, n, SOURCE, r.choice([0, 1, r.getrandbits(32)])], regions, ret='a')
        return Call(0x42d5a0, [COASTER, SOURCE], regions, ret='a')
    n = r.choice([0, 1, 2, 3, r.randrange(21)])
    secs, points, entries = sections(r, n)
    regions.update({SECTIONS: secs, POINTS: points, ENTRIES: entries})
    if op == 'Drop':
        return Call(0x42d440, [COASTER, SECTIONS, n], regions)
    used = bytearray(r.bytes(0x100))
    for i in range(0x40):
        put(used, 4 * i, 'i', r.choice([0, 1, 1, r.randrange(-3, 40), r.getrandbits(31)]))
    regions.update({USED: used, TO: bytearray(r.bytes(0x400))})
    secs_to = r.choice([TO, TO, int.from_bytes(secs[8:12], 'little')])
    return Call(0x42d390, [USED, SECTIONS, secs_to], regions)


OPS_ALL = ['Points', 'Indices', 'Pieces', 'Pack', 'Drop', 'Setup', 'Setup', 'Emit', 'Emit']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

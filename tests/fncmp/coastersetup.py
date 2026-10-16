# A coaster type set up from its description (src/game/coaster/setup: 0x42f390) and its pylon
# controls (0x42eb70) and cars and trains (0x42f1a0) steps against the originals -- each
# description's checks, names copied into GlobalMem blocks (global_heap()), fields copied when
# non-zero (floats by the x87 compare, NaN counting as zero), degrees turned to radians. The
# whole setup also runs the cross sections (xsection.py's coherent descriptions) and appends
# the static track entries at 0x74cde0 (varied here). Also the other allocating helpers: a
# car's coupling set up / freed (0x4344c0 / 0x433240), the queue rings' init / free (0x43b080 /
# 0x43b0c0), the coaster tables (0x43e980) and their setup with the id table (0x435500).
# usage: coastersetup.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402
import xsection  # noqa: E402

B = Bench([(0x42d130, 0x42eb68), (0x42eb70, 0x42fad0), (0x45b9d0, 0x45ba15), (0x45ba00, 0x45ba30),
           (0x5da3c0, 0x5da3d0), (0x4344c0, 0x434566), (0x44b140, 0x44b2e0),
           (0x43b080, 0x43b0d1), (0x433240, 0x433277), (0x43e980, 0x43e9bb), (0x435500, 0x43552d),
           (0x468750, 0x46875b)])
COASTER, NAMES, SOURCE, HOLDER = RAM, RAM + 0x1000, RAM + 0x10000, RAM + 0x20000
ZEROS = [0, 0x80000000, 0x7fc00000, 0xffc00000, 0x7f800001]  # zero, -0 and NaNs: all count as zero


def number(r):
    return r.choice([0, 0, r.f(), r.f(360), r.choice(ZEROS), r.getrandbits(32), 0x7f800000])


def word(r):
    return r.choice([0, 0, 1, -1 & 0xffffffff, r.randrange(-300, 300) & 0xffffffff, r.getrandbits(32)])


class Names:
    def __init__(self, r):
        self.r, self.buf = r, bytearray()

    def new(self):
        r = self.r
        n = r.choice([0, 1, 7, r.randrange(40)])
        at = NAMES + len(self.buf)
        self.buf += bytes(r.randrange(1, 256) for _ in range(n)) + b'\0'
        return at


def pylons(r, src, names, fail=0.08):
    count = r.choice([0, 1, 2, 2])  # two fit before the count; a third would start at it
    put(src, 0x3a20, 'I', count)
    for i in range(2):
        o = 0x38a8 + 0xbc * i
        bad = r.random() < fail
        put(src, o, 'I', r.choice([0, names.new()]) if bad else names.new())
        put(src, o + 4, 'I', r.choice([0, 1, r.getrandbits(32)]) if bad else r.choice([1, r.getrandbits(32)]))
        put(src, o + 8, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        put(src, o + 0xc, 'I', number(r) if bad else r.choice([r.f(), r.f(), r.getrandbits(32)]))
        for k in range(0x10, 0xbc, 4):
            put(src, o + k, 'I', word(r) if k in (0x14, 0x18, 0x1c, 0x54, 0x58) else number(r))


def trains(r, src, names, fail=0.08):
    count = r.choice([0, 1, 2, 3, 3] if fail > 0.05 else [1, 2, 3, 3])  # three fit before the count
    put(src, 0x3a78, 'I', count)
    for i in range(3):
        o = 0x3a24 + 0x1c * i
        bad = r.random() < fail
        put(src, o, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        put(src, o + 4, 'I', r.choice([0, 1, r.getrandbits(32)]) if bad else r.choice([1, r.getrandbits(32)]))
        put(src, o + 8, 'I', r.choice([0, names.new()]) if bad else names.new())
        put(src, o + 0xc, 'III', word(r), number(r), number(r))
        put(src, o + 0x18, 'I', number(r))
    car = lambda: r.choice([0, 0, 1, 2, r.randrange(max(count, 1)), count, r.getrandbits(32)])  # noqa: E731
    put(src, 0x3a7c, 'IIII', r.choice([0, 0, 1, 3, r.getrandbits(32)]), car(), car(), car())
    put(src, 0x3a8c, 'III', word(r), number(r), word(r))


def setup(r, src, names, regions):
    n = r.choice([0, 1, 3, 20, r.randrange(21), r.randrange(21)] if r.random() < 0.1 else [r.randrange(1, 21)])
    put(src, 0xfc, 'I', n)
    for i in range(20):
        put(src, 0xc + 0xc * i, 'III', names.new(), r.choice([0, 0, 1, r.getrandbits(32)]), number(r))
    pylons(r, src, names, 0.02)
    trains(r, src, names, 0.02)
    put(src, 0x3ac8, 'I', r.choice([0, 0, 1, 6, r.randrange(7)]))
    for k in range(0x3a98, 0x3ac8, 4):
        put(src, k, 'I', r.getrandbits(32))
    for k in range(0x3acc, 0x3b80, 4):
        if k == 0x3b00 or k == 0x3b04:  # the cross sections' (xsection.source)
            continue
        put(src, k, 'I', word(r) if k in (0x3ae8, 0x3aec, 0x3af0, 0x3af4, 0x3af8, 0x3afc, 0x3b74, 0x3b78, 0x3b7c)
            else number(r))
    statics = bytearray(r.bytes(0x34))  # the static track entries (count 3 in the game)
    put(statics, 0x30, 'I', r.choice([3, 3, 0, 1, 2]))
    regions[0x74cde0] = statics


def case(r):
    op = r.choice(OPS)
    names = Names(r)
    regions = {COASTER: bytearray(r.bytes(0x360)), HEAP: heap(0x10000), **global_heap()}
    if op == 'Coupling':  # spec (+0xc count, +0x10 first key) and type (+0x14 holder index)
        base, n = r.getrandbits(32), r.choice([0, 1, 2, r.randrange(6)])
        spec = put(bytearray(r.bytes(0x200)), 0xc, 'II', n, base)
        put(spec, 0x114, 'I', r.randrange(4))
        holder, model, parts = bytearray(r.bytes(0x40)), bytearray(r.bytes(0x80)), bytearray(r.bytes(12 * 0x14))
        put(holder, 0, 'I', HOLDER + 0x20)
        put(holder, 8, 'I', HOLDER + 0x20)
        put(holder, 0x24, 'I', HOLDER + 0x40)
        put(model, 0x48, 'H', r.choice([0, 3, 12]))
        put(model, 0x7c, 'I', HOLDER + 0x100)
        for i in range(12):  # keys near the spec's, some with flag 0x400000
            put(parts, i * 0x14, 'I', r.getrandbits(32) | r.choice([0, 0x400000, 0x400000]))
            put(parts, i * 0x14 + 4, 'I', (base + r.randrange(-1, 7)) & 0xffffffff)
        regions.update({SOURCE: spec, HOLDER: holder + model + bytearray(0x40) + parts,
                        0x7ae6a8: put(bytearray(16), 0, '4I', *[HOLDER] * 4)})
        return Call(0x4344c0, [r.getrandbits(32), COASTER + 0x78, SOURCE, SOURCE + 0x100], regions)
    if op == 'RingInit':
        return Call(0x43b080, [COASTER + 0xb0, r.choice([0, 1, 8, r.randrange(64)])], regions)
    if op == 'RingFree':
        put(regions[COASTER], 0xb4, 'I', r.choice([0, HEAP + 0x40, r.getrandbits(32)]))
        return Call(0x43b0c0, [COASTER + 0xb0], regions)
    if op == 'CouplingFree':  # the car's part (+0x100) flag 1 at +4 keeps the first array
        put(regions[COASTER], 0x100, 'I', COASTER + 0x200)
        put(regions[COASTER], 0x78, '3I', r.getrandbits(32), *(r.choice([0, HEAP + 0x40, r.getrandbits(32)]) for _ in range(2)))
        return Call(0x433240, [COASTER, COASTER + 0x78], regions)
    if op == 'Tables':
        regions.update({0x792400: bytearray(4), 0x7925dc: bytearray(4), 0x792408: bytearray(4)})
        return Call(0x43e980, [r.choice([0, 1, r.randrange(300)]), r.choice([0, 1, r.randrange(300)])], regions)
    if op == 'InitTables':  # 0x435500: the tables from the counts at 0x790fd0 / 0x791cfc, the id table
        regions.update({0x792400: bytearray(4), 0x7925dc: bytearray(4), 0x792408: bytearray(4),
                        0x790fd0: put(r.bytes(4), 0, 'I', r.choice([0, 1, r.randrange(300)])),
                        0x791cfc: put(r.bytes(4), 0, 'I', r.choice([0, 1, r.randrange(300)])),
                        0x790b00: r.bytes(8), 0x791ce0: r.bytes(4)})
        return Call(0x435500, [], regions)
    if op == 'Type':
        src = xsection.source(r, 'Setup')
        setup(r, src, names, regions)
        put(regions[COASTER], 0x1c8, 'I', r.choice([0x4004, r.getrandbits(32)]))
        args, at = [COASTER, r.getrandbits(32), SOURCE], 0x42f390
    else:
        src = bytearray(r.bytes(0x3a98))
        (pylons if op == 'Pylons' else trains)(r, src, names)
        args, at = [COASTER, SOURCE], 0x42eb70 if op == 'Pylons' else 0x42f1a0
    regions.update({NAMES: names.buf + r.bytes(16), SOURCE: src})
    return Call(at, args, regions, ret='a')


OPS_ALL = ['Pylons', 'Trains', 'Type', 'Type', 'Coupling', 'RingInit', 'RingFree', 'CouplingFree', 'Tables', 'InitTables']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

# A coaster type set up from its description (src/game/coaster/setup: 0x42f390) and its pylon
# controls (0x42eb70) and cars and trains (0x42f1a0) steps against the originals -- each
# description's checks, names copied into GlobalMem blocks (global_heap()), fields copied when
# non-zero (floats by the x87 compare, NaN counting as zero), degrees turned to radians. The
# whole setup also runs the cross sections (xsection.py's coherent descriptions) and appends
# the static track entries at 0x74cde0 (varied here).
# usage: coastersetup.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402
import xsection  # noqa: E402

B = Bench([(0x42d130, 0x42eb68), (0x42eb70, 0x42fad0), (0x45b9d0, 0x45ba15), (0x45ba00, 0x45ba30),
           (0x5da3c0, 0x5da3d0)])
COASTER, NAMES, SOURCE = RAM, RAM + 0x1000, RAM + 0x10000
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


OPS_ALL = ['Pylons', 'Trains', 'Type', 'Type']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

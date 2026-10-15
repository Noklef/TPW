# The coaster cross sections (src/game/coaster/xsection: 0x42d6d0, 0x42e200) against the
# original: 20 buckets of points or index triples packed into GlobalMem blocks (global_heap()),
# and the section records pointed at them.
# usage: xsection.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

B = Bench([(0x42d6d0, 0x42eb68), (0x45b9d0, 0x45ba15), (0x45ba00, 0x45ba30)])
COASTER, SECTIONS, SHARED, SOURCE = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x10000
# (first bucket, bucket stride, entry size) per op
LAYOUT = {'Points': (0x100, 0x194, 20), 'Indices': (0x2090, 0xf4, 12)}


def bucket(r, live):
    n = r.randrange(0, 21) if live else 0
    if r.random() < 0.03:
        n = r.choice([-1, -5, 21, 25, r.getrandbits(32)])
    return n & 0xffffffff


def case(r):
    op = r.choice(OPS)
    first, stride, size = LAYOUT[op]
    src = bytearray(r.bytes(0x3600))
    density = r.choice([0.0, 0.1, 0.3, 0.6, 1.0])
    counts = [bucket(r, r.random() < density) for _ in range(20)]
    if r.random() < 0.05:  # counts that cancel out
        counts[0], counts[1] = 3, (-3) & 0xffffffff
    for b, n in enumerate(counts):
        put(src, first + stride * b + stride - 4, 'I', n)
        if op == 'Points':
            for i in range(20):
                put(src, first + stride * b + size * i + 4, 'I', r.f())
    nsec = r.choice([0, 1, 3, r.randrange(12), r.randrange(-2, 12)])
    put(src, 0x3530, 'i', nsec)
    live = [b + 1 for b, n in enumerate(counts) if n] or [1]
    for k in range(max(nsec, 0)):
        o = 0x33a0 + 0x14 * k
        pick = lambda: r.choice([1, 1, 2, r.choice(live), r.choice(live), r.randrange(1, 21), 0, -1, 25])  # noqa: E731
        put(src, o, 'ii', pick(), pick())
        put(src, o + 0xc, 'i', r.choice([0, 0, pick(), r.choice(live)]))
    coaster = bytearray(r.bytes(0x98))
    put(coaster, 0x40, 'I', r.f())
    regions = {COASTER: coaster, SECTIONS: bytearray(r.bytes(0x18 * 12)), SHARED: bytearray(r.bytes(4)), SOURCE: src,
               HEAP: heap(0x6000), **global_heap()}
    if op == 'Points':
        return Call(0x42d6d0, [COASTER, SECTIONS, SOURCE, SHARED], regions, ret='a')
    return Call(0x42e200, [COASTER, SECTIONS, SOURCE], regions, ret='a')


OPS_ALL = ['Points', 'Indices']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

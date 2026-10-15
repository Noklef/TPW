# The coaster cross sections (src/game/coaster/xsection: 0x42e200) against the original: 20
# buckets of index triples packed into GlobalMem blocks (global_heap()), and the section records
# pointed at them.
# usage: xsection.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

B = Bench([(0x42e200, 0x42eb68), (0x45b9d0, 0x45ba15), (0x45ba00, 0x45ba30)])
COASTER, SECTIONS, SOURCE = RAM, RAM + 0x1000, RAM + 0x10000


def case(r):
    src = bytearray(r.bytes(0x3600))
    density = r.choice([0.0, 0.1, 0.3, 0.6, 1.0])
    for b in range(20):
        n = r.randrange(0, 21) if r.random() < density else 0
        if r.random() < 0.03:
            n = r.choice([-1, -5, 21, 25, r.getrandbits(32)])
        put(src, 0x2180 + 0xf4 * b, 'i' if n < 0x80000000 else 'I', n)
    if r.random() < 0.05:  # counts that cancel out
        put(src, 0x2180, 'i', 3)
        put(src, 0x2274, 'i', -3)
    nsec = r.choice([0, 1, 3, r.randrange(12), r.randrange(-2, 12)])
    put(src, 0x3530, 'i', nsec)
    for k in range(max(nsec, 0)):
        put(src, 0x33a4 + 0x14 * k, 'i', r.choice([1, 1, 2, r.randrange(1, 21), r.randrange(1, 21), 0, -1, 25]))
    regions = {COASTER: bytearray(r.bytes(0x98)), SECTIONS: bytearray(r.bytes(0x18 * 12)), SOURCE: src,
               HEAP: heap(0x4000), **global_heap()}
    return Call(0x42e200, [COASTER, SECTIONS, SOURCE], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

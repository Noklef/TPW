# The track ride record pools (src/game/world/trackpools) against the originals: pools whose first entries are in use up to some index (none, a
# few, all), or no pool at all, with random contents in the rest.
# usage: trackpools.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=BFind,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

# name: (address, size, stack arguments, record size, entries, pool pointer global, count global)
CALLS = {'BFind': (0x54b130, 0x40, 0, 0x1c, 0x400, 0x877b78, 0x877b7c), 'BAlloc': (0x54b170, 0x70, 2, 0x1c, 0x400, 0x877b78, 0x877b7c),
         'CFind': (0x54b1e0, 0x40, 0, 0x20, 0x800, 0x877b70, 0x877b74), 'CAlloc': (0x54b210, 0x80, 3, 0x20, 0x800, 0x877b70, 0x877b74)}
B = Bench([(a, a + n) for (a, n, *_r) in CALLS.values()] + [(0x5da3c0, 0x5da3c1)])
POOL = RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    address, _n, nargs, size, entries, pool_at, count_at = CALLS[name]
    pool = bytearray(r.bytes(size * entries))
    used = r.choice([0, 1, 2, 5, entries - 1, entries, r.randrange(entries + 1)])
    for i in range(entries):
        pool[i * size] = (pool[i * size] | 1) if i < used else (pool[i * size] & 0xfe)
    if r.random() < 0.3:  # a few later entries in use as well
        for _ in range(3):
            j = r.randrange(entries)
            pool[j * size] |= 1
    base = POOL if r.random() > 0.1 else 0
    regions = {POOL: pool, pool_at & ~0xf: bytearray(r.bytes(0x20))}
    put(regions[pool_at & ~0xf], pool_at & 0xf, 'I', base)
    put(regions[pool_at & ~0xf], (count_at & 0xf) + (count_at & ~0xf) - (pool_at & ~0xf), 'I', r.choice([0, 1, 100, 0x7fffffff]))
    return Call(address, [r.getrandbits(32) for _ in range(nargs)], regions, conv='c', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

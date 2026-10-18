# The triangles' depth sort (src/game/render/depthsort: 0x5653d0) against the original: up to 300 keys of few or many values (all equal,
# already sorted either way too) with their 6-byte triangles, lo and hi anywhere in them (lo past hi, equal, the whole run).
# usage: depthsort.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x5653d0, 0x56548b)])
KEYS, TRIS = RAM + 0x100, RAM + 0x1000


def case(r):
    n = r.choice([1, 2, 3, 4, 7, 9, 16, 40, 300]) if r.random() < 0.6 else r.randrange(1, 120)
    kinds = r.choice([1, 2, 3, 8, 0])
    keys = [r.randrange(kinds) * 0x1000 + r.choice([0, 0x80000000]) if kinds else r.getrandbits(32) for _ in range(n)]
    if r.random() < 0.15:  # already sorted either way
        keys.sort(reverse=r.random() < 0.5)
    lo, hi = 0, n - 1
    if r.random() < 0.4:
        lo = r.randrange(n)
        hi = r.randrange(max(lo - 1, 0), n)
    regions = {KEYS: struct.pack('<%dI' % n, *keys), TRIS: r.bytes(6 * n)}
    return Call(0x5653d0, [KEYS, TRIS, lo, hi], regions)


if __name__ == '__main__':
    B.main(case)

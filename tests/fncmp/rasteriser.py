# The span routine choice (src/game/render/rasteriser: 0x569f60) against the original: flags with and
# without 0x100, 0x200000, 0x200, 0x800, 0x10000000, 0x20000000 and 0x40000000, any routine table.
# usage: rasteriser.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x569f60, 0x56a02e)])
BITS = [0x100, 0x200000, 0x200, 0x800, 0x10000000, 0x20000000, 0x40000000]


def case(r):
    flags = r.getrandbits(32) if r.random() < 0.3 else sum(b for b in BITS if r.random() < 0.5) | (r.getrandbits(32) & 0x8ff)
    return Call(0x569f60, [flags & 0xffffffff], {0x6f19f0: r.bytes(0x40), 0xfbe000: r.bytes(0x60)}, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)

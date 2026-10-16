# The live objects list (src/game/core/liveobjects: 0x5775d0, 0x577600) against the originals:
# lists from empty to full (and beyond), objects present once, twice or not at all.
# usage: liveobjects.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Add,Remove)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, put  # noqa: E402

B = Bench([(0x5775d0, 0x5775f3), (0x577600, 0x57764b)])
CALLS = {'Add': 0x5775d0, 'Remove': 0x577600}
LIST = 0x87a6d8


def case(r):
    op = r.choice(OPS)
    n = r.choice([0, 1, 2, 5, 0x1ff, 0x200, 0x201, r.randrange(0x200)] + ([-1] if op == 'Remove' else []))
    objs = [r.choice([0x1000, 0x2000, 0x3000, r.getrandbits(32)]) for _ in range(0x200)]
    mem = bytearray(0x808)
    for i, o in enumerate(objs):
        put(mem, i * 4, 'I', o)
    put(mem, 0x800, 'I', n & 0xffffffff)
    obj = r.choice([0x1000, 0x2000, 0x3000, objs[r.randrange(0x200)], r.getrandbits(32)])
    return Call(CALLS[op], [obj], {LIST: mem}, conv='c', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

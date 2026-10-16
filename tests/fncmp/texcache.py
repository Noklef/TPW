# The texture cache items' base methods (src/game/gfx/texcache: 0x58b300-0x58b390) against the
# originals: flag words with and without 0x10000000 and the low bits.
# usage: texcache.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,SetFlags,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x58b300, 0x58b39b)])
CALLS = {'Ctor': 0x58b300, 'SetFlags': 0x58b330, 'Flag1': 0x58b350, 'Flag2': 0x58b360, 'Flag4': 0x58b370,
         'SetFlag1': 0x58b380, 'ClearFlag1': 0x58b390}


def case(r):
    op = r.choice(OPS)
    item = bytearray(r.bytes(0x10))
    put(item, 4, 'I', r.choice([0, 7, 0x10000000, 0x10000007, 0xffffffff, r.getrandbits(32)]))
    args = [RAM]
    if op == 'SetFlags':
        args.append(r.choice([0, 1, 6, 0x10000000, r.getrandbits(32)]))
    return Call(CALLS[op], args, {RAM: item}, conv='t', ret='v' if 'Set' in op or 'Clear' in op else 'a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

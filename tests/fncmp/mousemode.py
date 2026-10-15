# The mouse modes' constructors, destructors and empty methods (src/game/world/mousemode:
# 0x46c450-0x46d000) against the originals: each on a random object (the destructors' frees
# through heap()).
# usage: mousemode.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap  # noqa: E402

B = Bench([(0x46c450, 0x46c453), (0x46c460, 0x46c480), (0x46c580, 0x46c5c6), (0x46c650, 0x46c653),
           (0x46c6a0, 0x46c6b0), (0x46c6d0, 0x46c702), (0x46c710, 0x46c72e), (0x46cbc0, 0x46cbe1),
           (0x46ced0, 0x46ced7), (0x46cfc0, 0x46cfd0), (0x46cff0, 0x46d000)])
# op: (address, how many arguments after self)
CALLS = {'Nop4': (0x46c450, 4), 'Nop2': (0x46c650, 2), 'Dtor': (0x46ced0, 0), 'DelDtor': (0x46c460, 1),
         'DerivedDelDtor': (0x46c710, 1), 'Tile': (0x46c580, 1), 'TileKind': (0x46c5a0, 2),
         'OnlineBase': (0x46c6a0, 0), 'PlaceStaff': (0x46c6d0, 4), 'StaffDrop': (0x46cbc0, 2),
         'Aa0': (0x46cfc0, 0), 'Ad0': (0x46cff0, 0)}


def case(r):
    op = r.choice(OPS)
    at, n = CALLS[op]
    args = [r.choice([0, 1, 2, 3, r.getrandbits(32)]) for _ in range(n)]
    regions = {RAM: bytearray(r.bytes(0x40)), HEAP: heap(0x100)}
    ret = 'v' if op in ('Nop4', 'Nop2', 'Dtor') else 'a'
    return Call(at, [RAM] + args, regions, conv='t', ret=ret)


OPS_ALL = list(CALLS)
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

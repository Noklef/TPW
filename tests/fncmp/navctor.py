# The navigation block's constructor and destructor of src/game/world/nav (0x50ffe0 Nav_Ctor, 0x50f200 Nav_Dtor, 0x4f7c30 its adjustor from +0xd4) against the originals: an object of random bytes (the
# constructor sets the block and its five integer pairs; the destructor runs their destructor, which does nothing, and must leave every byte as it was).
# DtorJump: the jump 0x4f8ad0 to the destructor.
# usage: navctor.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Dtor,DtorD4,DtorJump)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x4f7c30, 0x4f7c3b), (0x50ffe0, 0x5100f0), (0x50f1f0, 0x50f240), (0x5d1750, 0x5d1751), (0x67acf0, 0x67ada0), (0x67afa0, 0x67b0d0),
           (0x4f8ad0, 0x4f8ad6)])
OPS = {'Ctor': (0x50ffe0, 'a'), 'Dtor': (0x50f200, 'v'), 'DtorD4': (0x4f7c30, 'v'), 'DtorJump': (0x4f8ad0, 'v')}


def case(r):
    address, ret = OPS[r.choice(PICK)]
    return Call(address, [RAM], {RAM: r.bytes(0x200)}, conv='t', ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

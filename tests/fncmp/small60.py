# The three small callers of src/game/core/small60 (0x4254c0, 0x4de110, 0x4ddf40) against the originals: small58.py's holder in the renderer (Animator), its queue walk
# (BackTo) and objectmsg.py's queue of persons (None: the position of the person with no handle).
# usage: small60.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Animator,BackTo,None)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import objectmsg  # noqa: E402
import small58  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4254c0, 0x4254f0), (0x45bdd0, 0x45be90), (0x4d8300, 0x4d8340), (0x4d96f0, 0x4d97f0), (0x4d99c0, 0x4d99f0), (0x4ddf40, 0x4de200), (0x4de670, 0x4de7f0),
           (0x502430, 0x502470), (0x50b350, 0x50b370), (0x522770, 0x5227a0), (0x522850, 0x522870), (0x536320, 0x536360), (0x576540, 0x5765d0), (0x5da3c0, 0x5da3c1)])
ANIM = RAM + 0x2000


def case(r):
    name = r.choice(OPS)
    if name == 'Animator':
        c = small58.sleep(r)
        c.regions[ANIM] = put(put(bytearray(r.bytes(0x40)), 0, 'I', r.getrandbits(32)), 0x24, 'I', RAM)
        return Call(0x4254c0, [ANIM], c.regions, conv='c', ret='a')
    if name == 'BackTo':
        c = small58.back(r)
        c.address, c.ret = 0x4de110, 'a'
        return c
    c = objectmsg.queue(r, 'Pos')
    c.address, c.args, c.ret = 0x4ddf40, c.args[:1], 'a'
    return c


OPS = [k for k in os.environ.get('OPS', 'Animator,BackTo,None').split(',') if k]

if __name__ == '__main__':
    B.main(case)

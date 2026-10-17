# The thing of kind 0xe and the person base constructors (src/game/world/things, person: 0x502d20, 0x518e20, 0x518e00) against the originals: things_methods.py's thing table and cells for the thing
# constructor (a thing of kind 0xe, with room for its record array), a plain object of random bytes for the person constructors.
# usage: thing_ctors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Thing,Class,Parts)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import things_methods  # noqa: E402
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x4d91f0, 0x4d9290), (0x502d20, 0x502d70), (0x50afe0, 0x50b090), (0x50ba80, 0x50bac0), (0x50f1f0, 0x50f200), (0x50ffe0, 0x5100f0), (0x516270, 0x516320),
           (0x518e00, 0x518e20), (0x518e20, 0x518e70), (0x5d1750, 0x5d1751), (0x67acf0, 0x67ada0), (0x67b050, 0x67b0d0), (0x5da3c0, 0x5da3c1)])
OBJ = RAM + 0x8000


def case(r):
    name = r.choice(OPS)
    if name == 'Thing':
        c = things_methods.construct(r)
        regions = dict(c.regions)
        regions.pop(things_methods.NEWTHING, None)
        regions[OBJ] = r.bytes(0x1500)
        return Call(0x502d20, [OBJ], regions, conv='t', ret='a')
    if name == 'Class':
        return Call(0x518e20, [OBJ, r.getrandbits(32)], {OBJ: r.bytes(0x200)}, conv='t', ret='a')
    if name == 'Parts':
        return Call(0x518e00, [OBJ, r.getrandbits(32)], {OBJ: r.bytes(0x200)}, conv='t', ret='a')
    raise AssertionError(name)


OPS = [k for k in os.environ.get('OPS', 'Thing,Class,Parts').split(',') if k]

if __name__ == '__main__':
    B.main(case)

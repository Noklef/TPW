# The camera (src/game/world/camera: 0x42a0f0-0x42a182, 0x42a410, 0x42b070) against the
# original: the static initialisers, the follow test and the matrix closeness test 0x42b070.
# usage: camera.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x42a0f0, 0x42a183), (0x42a410, 0x42a43a), (0x42b070, 0x42b1b6)])
INITS = {'One': (0x42a0f0, 0x7909e8, 4), 'Epsilon': (0x42a110, 0x790a94, 4), 'Zoom': (0x42a130, 0x7909ec, 4),
         'Yaw': (0x42a140, 0x790a38, 4), 'E0': (0x42a150, 0x7909e0, 4), 'Point': (0x42a160, 0x7908f0, 12)}


def case(r):
    op = r.choice(OPS)
    if op == 'Close':  # two matrices, often nearly equal so the 1e-5 threshold decides
        scale = r.choice([1, 1, 2, None])  # near 1, a nudge of a few ulps is near 1e-5
        a = [r.f(scale) for _ in range(16)]
        b = [r.choice([v, v, v, v, v, r.f(scale)]) for v in a]
        ma, mb = bytearray(64), bytearray(64)
        for i in range(16):
            put(ma, 4 * i, 'I', a[i])
            put(mb, 4 * i, 'I', b[i])
            if r.random() < 0.2:  # a nudge of a few units in the last place
                put(mb, 4 * i, 'I', (a[i] + r.randrange(-40, 41)) & 0xffffffff)
        return Call(0x42b070, [RAM, RAM + 0x100], {RAM: ma, RAM + 0x100: mb}, ret='a')
    if op in INITS:
        at, var, size = INITS[op]
        return Call(at, [], {var: bytearray(r.bytes(size))})
    followed = r.choice([0, 0x1234560, r.getrandbits(32)])
    thing = r.choice([followed, followed, 0, r.getrandbits(32)])
    flags = bytearray(r.bytes(4))
    if r.random() < 0.6:
        flags[0] |= 4
    regions = {0x790ab0: flags, 0x790988: bytearray(r.choice([bytes(4), r.bytes(4), r.bytes(4)])),
               0x790a44: bytearray(4)}
    put(regions[0x790a44], 0, 'I', followed)
    return Call(0x42a410, [thing], regions, ret='a')


OPS_ALL = list(INITS) + ['Follow'] * 6 + ['Close'] * 6
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

# The camera (src/game/world/camera: 0x42a0f0-0x42a182 and the leaves 0x42a410-0x42cd40)
# against the original: the static initialisers, the follow tests, the part face, the view
# setter, flag 0x80, the matrix closeness test 0x42b070 and the cell clamp.
# usage: camera.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x42a0f0, 0x42a183), (0x42a410, 0x42a43a), (0x42a4e0, 0x42a4fb), (0x42a720, 0x42a75f), (0x42aab0, 0x42ab1d),
           (0x42afd0, 0x42afed), (0x42b070, 0x42b1b6), (0x42cd40, 0x42cd7f)])
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
    if op == 'Face':  # instance -> X, X + 4 -> model; the index straddles the +0x44 count
        first, base = r.choice([0, 1, 5, 0xffff, r.getrandbits(16)]), r.choice([0, 2, r.getrandbits(16)])
        part = r.choice([0, 1, 3, 7, r.getrandbits(32), (first - base) & 0xffffffff])
        model = put(bytearray(r.bytes(0x78)), 0x44, 'HH', first, base)
        regions = {RAM: put(bytearray(4), 0, 'I', RAM + 0x100), RAM + 0x100: put(bytearray(8), 4, 'I', RAM + 0x200),
                   RAM + 0x200: model, RAM + 0x300: put(bytearray(4), 0, 'I', part)}
        return Call(0x42a720, [RAM, RAM + 0x300], regions, ret='a')
    if op == 'View':  # each pointer null or a value; z is read whenever x is given
        regions = {0x790a88: bytearray(r.bytes(12)), 0x7908f0: bytearray(r.bytes(12)), 0x7909ec: bytearray(r.bytes(4)),
                   0x790a38: bytearray(r.bytes(4)), 0x74c9bc: bytearray(r.bytes(4)), RAM: bytearray(r.bytes(0x20))}
        x = r.choice([0, RAM])
        z = RAM + 4 if x else r.choice([RAM + 4, 0])
        return Call(0x42aab0, [x, z] + [r.choice([0, RAM + 8 + 4 * i]) for i in range(3)], regions)
    if op == 'Flag80':
        return Call(0x42afd0, [r.choice([0, 1, r.getrandbits(32)])], {0x790ab0: bytearray(r.bytes(4))})
    if op == 'Clamp':  # signed cells against the map size; a size of 0 or less clamps to it - 1
        def size():
            return r.choice([0, 1, 64, 128, 256, r.getrandbits(32)])

        def cell():
            return r.choice([0, -1, 1, 63, 64, 127, 128, 255, 256, r.getrandbits(32) - (1 << 31)]) & 0xffffffff
        regions = {RAM: put(bytearray(8), 0, 'II', cell(), cell()), 0x7a0314: put(bytearray(4), 0, 'I', size()),
                   0x7a0374: put(bytearray(4), 0, 'I', size())}
        return Call(0x42cd40, [RAM, RAM + 4], regions)
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
    if op == 'Followed':
        return Call(0x42a4e0, [], regions, ret='a')
    return Call(0x42a410, [thing], regions, ret='a')


OPS_ALL = list(INITS) + ['Follow'] * 6 + ['Close'] * 6 + ['Followed'] * 3 + ['Face', 'View', 'Clamp'] * 4 + ['Flag80'] * 2
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

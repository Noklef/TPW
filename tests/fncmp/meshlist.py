# Readying meshes (src/game/render/meshlist: 0x5744d0, 0x574110) against the originals: zero-ended arrays of
# up to 5 meshes, with and without flags 0x20 and 0x10000000, one mesh listed twice, empty arrays; and any
# directions (zero, tiny, huge and NaN (signalling ones too) now and then) made unit length.
# usage: meshlist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ready,Direction)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x574110, 0x5741a7), (0x5744d0, 0x574526)])
OPS = [k for k in os.environ.get('OPS', 'Ready,Direction').split(',') if k]
MESHES, LIST = RAM + 0x1000, RAM + 0x2000


def case(r):
    if r.deal(OPS) == 'Direction':
        v = [r.f() if r.random() < 0.7 else r.choice([0, 0x80000000, r.f(1e-20), r.f(1e19), 0x7f800001, 0xff812345]) for _ in range(3)]
        if r.random() < 0.15:
            v = [r.choice([0, 0x80000000]) for _ in range(3)]
        return Call(0x574110, [RAM] + v, {RAM: r.bytes(0x40)}, conv='t')
    count = r.choice([0, 1, 2, 3, 5])
    meshes = bytearray(r.bytes(0x100 * 5))
    for i in range(5):
        flags = r.getrandbits(32) & ~0x10000020
        flags |= r.choice([0, 0x20]) | r.choice([0, 0x10000000, 0, 0])
        put(meshes, 0x100 * i + 0x50, 'I', flags)
    pick = [r.randrange(5) for _ in range(count)]
    if count > 1 and r.random() < 0.2:
        pick[1] = pick[0]
    entries = [MESHES + 0x100 * i for i in pick] + [0]
    regions = {RAM: r.bytes(0x40), MESHES: meshes, LIST: struct.pack('<%dI' % len(entries), *entries),
               0x879de8: r.bytes(8)}
    return Call(0x5744d0, [RAM, LIST], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

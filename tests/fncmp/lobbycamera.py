# The lobby camera's step (src/game/frontend/lobbyscene: 0x5e1210) against the original: an owner of random floats (the angle any, small,
# large past the x87's 2^63 trig limit or special, but none from 1e6 to 2^63), its model at +0xc with a random target at +0x3c, the frame
# scale at [0xf82884] + 8 (0, 1, small or any) and the camera at [0xf83778] that gets the eased position and target.
# usage: lobbycamera.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, bf, fb, put  # noqa: E402

B = Bench([(0x5e1210, 0x5e1340)])
OWNER, MODEL, FRAME, CAMERA = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300


def case(r):
    owner = r.floats(bytearray(r.bytes(0x60)), range(0x18, 0x58, 4))
    put(owner, 0xc, 'I', MODEL)
    angle = r.choice([r.f(), r.f(10), fb(r.uniform(-7, 7)), fb(r.choice([1e19, -1e19, 3e38])), r.choice([0, 0x7fc00000, 0x7f800001, 0x7f800000])])
    if 1e6 < abs(bf(angle)) < 2.0 ** 63:   # (left out, as in mat4.py: the x87 reduces these by a 66-bit pi, which neither CRT stands in for)
        angle = fb(r.uniform(-1e6, 1e6))
    put(owner, 0x18, 'I', angle)
    model = r.floats(bytearray(r.bytes(0x50)), range(0x3c, 0x48, 4))
    frame = put(bytearray(r.bytes(0x10)), 8, 'I', r.choice([fb(1.0), 0, fb(r.uniform(0, 2)), r.f()]))
    regions = {OWNER: owner, MODEL: model, FRAME: frame, CAMERA: r.bytes(0x24), 0xf82884: struct.pack('<I', FRAME),
               0xf83778: struct.pack('<I', CAMERA)}
    return Call(0x5e1210, [OWNER], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

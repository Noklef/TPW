# The lobby camera applied for a frame (src/game/frontend/lobbyscene: 0x5d8cb0) against the original: Mat4_Perspective, ViewQuad_SetFov, Camera_SetLookAt, Camera_Update and Render_FadeFog are recorders, each with a bench of
# its own. The fade +0x5c at, below or above its target +0x60, 0 or less, a little or past 0xffff; the angle +4 any bits (NaNs reach the recorders as they are).
# usage: lobbycameraframe.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

PERSPECTIVE, FOV, LOOK, UPDATE, FOG = 16, 17, 18, 19, 20
B = Bench([(0x5d8cb0, 0x5d8d63)], hooks={0x578be0: _recorder(PERSPECTIVE, 3), 0x56b790: _recorder(FOV, 1), 0x5d8de0: _recorder(LOOK, 0), 0x42b1c0: _recorder(UPDATE, 0, cdecl=True), 0x5d96c0: _recorder(FOG, 0)})
CAMERA = RAM


def fade(r):
    return r.choice([0, 1, 8, 0xff, 0x100, 0x1ff, 0xffff, 0x10000, 0x7fffffff, 0x80000000, 0xffffffff, 0xfffffff8, r.randrange(0x10000), r.getrandbits(32)])


def case(r):
    camera = bytearray(r.bytes(0x68))
    put(camera, 4, 'I', r.choice([0x3f800000, 0x7f800001, 0xffc00000, r.getrandbits(32)]))
    here = fade(r)
    put(camera, 0x5c, '2I', here, r.choice([here, here, fade(r), fade(r)]))
    return Call(0x5d8cb0, [CAMERA], {LOG: log_region(), CAMERA: camera, 0x879d54: r.bytes(4), 0x8bcbc8: r.bytes(4), 0x8bd4e4: r.bytes(4)}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

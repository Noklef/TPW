# The view quad's corners from a field of view (src/game/render/viewquad: 0x56b790) against the original: the twelve floats over random
# bytes, from fovs of 90, near it, small, large (past 2^63 for fptan too) and special, the aspect at 0x8bcbcc likewise.
# usage: viewquad.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

B = Bench([(0x56b790, 0x56b827)])
QUAD = RAM + 0x100


def case(r):
    fov = r.choice([0x42b40000, r.f(), r.f(180), r.pos(180), r.f(1e6), r.f(1e12), r.f(1e18), r.f(1e21), fb(r.choice([180.0, 360.0, -180.0, 0.0, 1e22, -3e21, math.inf]))])
    aspect = r.choice([fb(4 / 3), fb(0.75), r.f(), r.f(4)])
    regions = {QUAD: r.bytes(0x30), 0x8bcbcc: struct.pack('<I', aspect)}
    return Call(0x56b790, [QUAD, fov], regions, conv='t')


if __name__ == '__main__':
    B.main(case)

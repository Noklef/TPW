# The matrix object methods (src/game/math/mat4: 0x578800, 0x578bc0) against the originals: any
# matrix bits, overlapping copies included.
# usage: mat4.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Identity,Assign)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x578800, 0x578820), (0x578bc0, 0x578bd4)])
CALLS = {'Identity': 0x578800, 'Assign': 0x578bc0}


def case(r):
    op = r.choice(OPS)
    regions = {RAM: r.bytes(0x100)}
    if op == 'Identity':
        return Call(0x578800, [RAM + r.choice([0, 0x40])], regions, conv='t')
    # copied upwards a dword at a time, so a source just below the matrix smears
    m = RAM + r.choice([0, 0x40, 0x80])
    return Call(0x578bc0, [m, RAM + r.choice([0, 0x40, 0x80, 0x44, 0x7c, 0x84])], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

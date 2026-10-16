# The direction steps of src/game/world/dirstep against the originals: positions anywhere, directions the four right angles,
# their neighbours and anything.
# usage: dirstep.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=A,B)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

CALLS = {'A': 0x52a3a0, 'B': 0x52a420}
B = Bench([(0x52a3a0, 0x52a418), (0x52a420, 0x52a490), (0x4d8c00, 0x4d8c20)])
DIRS = [0, 90, 180, 270, 1, 89, 91, 179, 181, 269, 271, 360, 450, -90, 0x7fffffff, 0x80000000, 0xffffffff]


def case(r):
    name = r.choice(OPS)
    pos = struct.pack('<ii', r.choice([0, 1, -1, 100, 0x7fffffff, -0x80000000, r.randrange(-500, 500)]),
                      r.choice([0, 1, -1, 100, 0x7fffffff, -0x80000000, r.randrange(-500, 500)]))
    direction = r.choice(DIRS) & 0xffffffff if r.random() < 0.8 else r.getrandbits(32)
    # the position's address varies too: the original's fallback result is that address (its low byte shows in StepB)
    at = r.choice([0, 1, 3, 5, 0x10, 0x1f, 0x7f, 0x80, 0xff])
    return Call(CALLS[name], [RAM + at, RAM + at + 4, direction], {RAM: bytes(at) + pos + bytes(0x20)}, conv='c', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

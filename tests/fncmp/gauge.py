# The framed gauge's plain members (src/game/gfx/gauge: 0x47faa0-0x47fbe0) against the
# original: init, marks, frame and colours on a gauge of random bytes.
# usage: gauge.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x47faa0, 0x47fbf3)])  # 0x47fbe0's ret 8 is at 0x47fbf0
CALLS = {'Init': (0x47faa0, 0), 'MarksX': (0x47fb10, 3), 'MarksY': (0x47fb30, 3), 'Rect': (0x47fb50, 4),
         'Colour0': (0x47fb80, 2), 'Colour1': (0x47fba0, 2), 'Colour2': (0x47fbc0, 2), 'Colour3': (0x47fbe0, 2)}


def case(r):
    op = r.choice(OPS)
    at, n = CALLS[op]
    args = [r.choice([r.randrange(-50, 700), r.getrandbits(32)]) & 0xffffffff for _ in range(n)]
    return Call(at, [RAM] + args, {RAM: bytearray(r.bytes(0x50))}, conv='t', ret='a' if op == 'Init' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

# The tiled backdrop's setters (src/game/render/backdrop: 0x5777a0, 0x577940, 0x577970) against the
# originals: any object bytes, any values.
# usage: backdrop.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear5c,SetRect,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x5777a0, 0x5777a8), (0x577940, 0x577965), (0x577970, 0x57798f)])
CALLS = {'Clear5c': 0x5777a0, 'SetRect': 0x577940, 'SetSourceRect': 0x577970}


def case(r):
    op = r.choice(OPS)
    args = [RAM] + ([] if op == 'Clear5c' else [r.getrandbits(32) for _ in range(4)])
    return Call(CALLS[op], args, {RAM: r.bytes(0x60)}, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

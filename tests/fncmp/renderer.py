# The renderer object's accessors (src/game/render/renderer: 0x576260, 0x5763f0, 0x576400) against
# the originals: any object bytes, any values.
# usage: renderer.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetRestoreCallback,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x576260, 0x57626a), (0x5763f0, 0x5763fa), (0x576400, 0x576404)])
CALLS = {'SetRestoreCallback': 0x576260, 'SetStateFlags': 0x5763f0, 'StateFlags': 0x576400}


def case(r):
    op = r.choice(OPS)
    args = [RAM]
    if op != 'StateFlags':
        args.append(r.choice([0, 0x21, 0x2821, 0x54e2b0, r.getrandbits(32)]))
    return Call(CALLS[op], args, {RAM: r.bytes(0x80)}, conv='t', ret='a' if op == 'StateFlags' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

# The texture format helpers (src/game/render/texformat: 0x5839a0, 0x5839e0) against the originals:
# any settings bytes, any current format.
# usage: texformat.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetArgb32,Table)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5839a0, 0x5839ee)])
CALLS = {'SetArgb32': 0x5839a0, 'Table': 0x5839e0}


def case(r):
    op = r.choice(OPS)
    settings = bytearray(r.bytes(0x90))
    put(settings, 4, 'I', r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    return Call(CALLS[op], [RAM], {RAM: settings}, conv='t', ret='a' if op == 'Table' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

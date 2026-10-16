# The render state setters (src/game/render/state: 0x5836b0, 0x5d9680-0x5d96a0) against the
# originals: flags words in any state, fog settings of any bits.
# usage: renderstate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Touch,FogOn,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5836b0, 0x5836c2), (0x5d9680, 0x5d96c4)])
CALLS = ['Touch', 'FogOn', 'FogOff', 'SetFog']


def case(r):
    op = r.choice(OPS)
    regions = {RAM: r.bytes(0x10), 0x8bcbc0: r.bytes(0x10), 0x8bcc40: r.bytes(8), 0x8bd470: r.bytes(0x30)}
    if op == 'Touch':
        return Call(0x5836b0, [RAM], regions, conv='t')
    if op == 'SetFog':
        return Call(0x5d96a0, [RAM, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t')
    return Call({'FogOn': 0x5d9680, 'FogOff': 0x5d9690}[op], [RAM], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

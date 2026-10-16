# The weather renderer's setters (src/game/render/weatherfx: 0x57d910, 0x57dc40, 0x57dc70) against
# the originals: kind bits in any state, any values.
# usage: weatherfx.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetDropCount,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x57d910, 0x57d91d), (0x57dc40, 0x57dc78)])
CALLS = {'SetDropCount': 0x57d910, 'SetFallSpeed': 0x57dc40, 'RequestReload': 0x57dc70}


def case(r):
    op = r.choice(OPS)
    fx = bytearray(r.bytes(0x3340))
    put(fx, 0, 'I', r.choice([0, 1, 2, 3, 0xfffffffc, r.getrandbits(32)]))
    args = [RAM]
    if op != 'RequestReload':
        args.append(r.choice([0, 0x3f4ccccd, r.getrandbits(32)]))
    return Call(CALLS[op], args, {RAM: fx}, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

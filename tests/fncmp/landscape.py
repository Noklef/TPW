# The landscape renderer's accessors (src/game/terrain/landscape: 0x5856e0, 0x585710, 0x585ef0,
# 0x585f00) against the originals: points 0 to 3 (and a little either side), any values.
# usage: landscape.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetPoint,SetDetail,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x5856e0, 0x585705), (0x585710, 0x58571d), (0x585ef0, 0x585ef7), (0x585f00, 0x585f07)])
CALLS = {'SetPoint': 0x5856e0, 'SetDetail': 0x585710, 'List': 0x585ef0, 'Point': 0x585f00}


def case(r):
    op = r.choice(OPS)
    regions = {RAM + 0x1f00: r.bytes(0x100), RAM + 0x2ec0: r.bytes(0x10)}
    if op == 'SetPoint':
        args = [RAM, r.choice([0, 1, 2, 3, -1, 4, 5])] + [r.getrandbits(32) for _ in range(3)]
        return Call(0x5856e0, args, regions, conv='t')
    if op == 'SetDetail':
        return Call(0x585710, [RAM, r.choice([0, 1, 2, 3, 4, r.getrandbits(32)])], regions, conv='t')
    return Call(CALLS[op], [RAM], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

# The postcard sender's result callback (src/game/frontend/postcard: 0x4a7bf0) against the original: done 0, 1 or any, the result 0-4
# mostly (5, -1 or any now and then), with dequeiter.py's command queue surroundings for the post (CommandQueue_Post).
# usage: postcard.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dequeiter  # noqa: E402
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x47f410, 0x47f61e), (0x4a7bf0, 0x4a7c3c), (0x5c8bb0, 0x5c8bd0), (0x632200, 0x632222), (0x632230, 0x632290), (0x632290, 0x6322dc),
           (0x6322e0, 0x632310), (0x632310, 0x632343), (0x67acb0, 0x67acb1)])


def case(r):
    regions = {dequeiter.MAP: struct.pack('<5I', *[dequeiter.BLOCKS + 0x1000 * i for i in range(5)]), dequeiter.BLOCKS: r.bytes(0x5000)}
    dequeiter.queue(r, regions)
    done = r.choice([0, 1, 1, 1, r.getrandbits(32)])
    result = r.choice([0, 1, 2, 3, 4] * 3 + [5, 0xffffffff, r.getrandbits(32)])
    return Call(0x4a7bf0, [done, result], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)

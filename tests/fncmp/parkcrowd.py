# The park's first people (Game_StartCrowd 0x516700, src/game/world/parkopen) against the original. The game object is a region at its fields +0x1da700 (the
# random state +0x1da708 of World_Rand, real); what it calls is recorders: Game_OpenPark, the two positions (each answers its out argument, a new word
# written through it each call), the ride count, the three constructors (their cell a WORD). Allocations fail now and then (the heap's countdown).
# usage: parkcrowd.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, echo, fill, heap, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x516700, 0x5168e3), (0x516330, 0x516370), (0x5da3c0, 0x5da3c1)],
          hooks={0x519ef0: _recorder(1, 2), 0x4d8610: _recorder(2, 2), 0x4d8690: _recorder(3, 2), 0x4ca8e0: _recorder(4, 2, com=True),
                 0x4faec0: _recorder(5, 1), 0x4d9db0: _recorder(6, 1), 0x4d4250: _recorder(7, 1)})
GAME = RAM + 0x10000 - 0x1da700


def case(r):
    log = log_region()
    for k in (2, 3):
        echo(log, k, 0)
        strings(log, k, 0, out=1)
        fill(log, k, 0, struct.pack('<8H', *(r.getrandbits(16) for _ in range(8))), which=k - 2, records=8)
    for k in (5, 6, 7):
        put(log, LOG_BYTES + 4 * k, 'I', 1 << 16)
    n = r.choice([0, 0, 1, 2, 3, 5, -1, -0x80000000, r.randrange(8)])
    set_return(log, 4, n & 0xffffffff)
    size = 0x10 + 26 * 0x230 + 2 * max(n, 0) * 0x230
    fail = r.choice([0, 0, 0, 1, 2, 3, 15, r.randrange(1, 16)])
    block = heap(size, fail)
    if n > 0 and r.random() < 0.3:
        # Past the countdown's 15: the next block set so that the k-th allocation's block is address 0, the answer of a failure. (The blocks before it are never touched: the constructors are recorders.)
        block = heap(size)
        put(block, 0, 'I', 0 - (r.randrange(26, 26 + 2 * n) - 1) * 0x230 & 0xffffffff)
    regions = {LOG: log, HEAP: block, RAM + 0x10000: bytearray(r.bytes(0x50))}
    return Call(0x516700, [GAME], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

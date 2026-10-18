# The tiny vtable stubs and getters (src/game/core/trivial) against the originals: any object
# bytes, stack arguments for the ones that pop them.
# usage: trivial.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Get3e4,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x5b96a0, 0x5b96a7), (0x5b96b0, 0x5b96b7), (0x5c5520, 0x5c5523), (0x5c5550, 0x5c5554), (0x5c55b0, 0x5c55b4),
           (0x5cec70, 0x5cec73), (0x5d1410, 0x5d1416), (0x5d1750, 0x5d1751), (0x5da3d0, 0x5da3d3),
           (0x5e7310, 0x5e7319), (0x5e7720, 0x5e7736), (0x414670, 0x41467a), (0x44e600, 0x44e610)])
# name: (address, convention, number of stack arguments, returns)
CALLS = {'Get3e4': (0x5b96a0, 't', 0, 'a'), 'Get3a4': (0x5b96b0, 't', 0, 'a'), 'Self': (0x5c5520, 't', 0, 'a'),
         'Part14': (0x5c5550, 't', 0, 'a'), 'Part3c': (0x5c55b0, 't', 0, 'a'), 'Ret4': (0x5cec70, 's', 1, 'v'),
         'One': (0x5d1410, 't', 0, 'a'), 'Ret': (0x5d1750, 'c', 0, 'v'),
         'Ret12': (0x5da3d0, 's', 3, 'v'), 'CopyC': (0x5e7310, 't', 0, 'a'), 'Pair': (0x5e7720, 't', 2, 'a'),
         'Low16': (0x414670, 'c', 1, 'a'), 'Out1': (0x44e600, 'c', 4, 'a')}


def case(r):
    address, conv, n, ret = CALLS[r.choice(OPS)]
    regions = {RAM: r.bytes(0x400)}
    args = [r.getrandbits(32) for _ in range(n)]
    if conv == 't':
        args = [RAM + r.choice([0, 0x10, 0x40])] + ([RAM + r.choice([0x20, 0x100, 0x2, 0x40, 0x12]) for _ in range(n)] if n else [])
    if address == 0x44e600:   # (out)
        args[3] = RAM + r.choice([0, 0x20, 0x102])
    return Call(address, args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

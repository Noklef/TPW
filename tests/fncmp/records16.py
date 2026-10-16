# The 16-byte record helpers of src/game/core/records16 against the originals: arrays of 0-6 records whose characters are spaces,
# commas, brackets and letters, ranges that are empty, reversed or whole.
# usage: records16.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Sp,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, returns, size)
CALLS = {'Sp': (0x6ae2e0, 'b', 0x20), 'Swap': (0x6ae300, 'v', 0x90), 'FindS': (0x6ae390, 'a', 0x38), 'FindN': (0x6ae3d0, 'a', 0x38),
         'Rev': (0x6ae440, 'v', 0x70), 'Rtl': (0x6ae4b0, 'v', 0xa0)}
B = Bench([(a, a + n) for (a, _r, n) in CALLS.values()] + [(0x6ae2b0, 0x6ae2d4), (0x6ae410, 0x6ae43d)])
POOL = RAM + 0x1000
CHARS = [0x20, 0x2c, 0x28, 0x29, 0x41, 0, 0x28, 0x29, 0x120, 0xffff, 0x5c0, 0x5d0, 0x5b0, 0x5eb]


def case(r):
    name = r.choice(OPS)
    address, ret, _n = CALLS[name]
    n = r.choice([0, 1, 2, 3, 4, 6])
    data = bytearray(r.bytes(0x100))
    for i in range(8):
        data[0x10 * i:0x10 * i + 2] = (r.choice(CHARS)).to_bytes(2, 'little')
    first = POOL + (0x10 * r.randrange(0, 2) if r.random() < 0.3 else 0)
    last = POOL + 0x10 * n + (0x10 * r.randrange(0, 2) if r.random() < 0.2 else 0)
    if name == 'Sp':
        args = [POOL + 0x10 * r.randrange(0, 4)]
    else:
        args = [first, last]
    return Call(address, args, {POOL: data}, conv='c', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

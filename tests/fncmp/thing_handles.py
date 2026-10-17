# The small helpers of src/game/core/thing_handles against the originals: the thing table slots (any handle with garbage above the low word), cell coordinates, four buffers
# (some null, some the shared one).
# usage: thing_handles.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, global_heap, heap, put  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Thing': (0x4cd300, 's', 'a', 0x30), 'Cell': (0x4cd4e0, 'c', 'a', 0x20), 'Free': (0x413340, 't', 'v', 0x40)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x5f5680, 0x5f5690), (0x5da3c0, 0x5da3c1)])


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    if name == 'Thing':
        slots = bytearray(r.bytes(20 * 12))
        handle = r.randrange(12) | r.choice([0, 0, r.getrandbits(16) << 16])
        return Call(address, [handle], {0x7cfb90: slots}, conv=conv, ret=ret)
    if name == 'Cell':
        return Call(address, [RAM, r.choice([0, 1, 0x7f, 0x80, r.getrandbits(32)]), r.choice([0, 1, 0x7f, 0x80, r.getrandbits(32)])], {RAM: r.bytes(8)}, conv=conv, ret=ret)
    obj = bytearray(r.bytes(0x600))
    for k in range(4):
        put(obj, 0x4f4 + 4 * k, 'I', r.choice([0, 0x6fd880, HEAP + 0x40 + 0x40 * k, HEAP + 0x40 + 0x40 * k]))
    return Call(address, [RAM], {RAM: obj, HEAP: heap(), **global_heap()}, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

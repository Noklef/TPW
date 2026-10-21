# The counted shared globals' releases of src/game/core/sharedglobals against the originals: the global a counted object (its use count
# at the edges: 0, 1, 2, 3, ~0, ~1; its slot 0 a recorder) or now and then none (a fault in both); the STL lock left out of both
# (counted_shared.py's hooks). The object, the global and the log are compared.
# usage: sharedglobals.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Global7ced48_Release,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from counted_shared import lock, shared_count  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, log_region, vtable  # noqa: E402

# name: (address, the global)
CALLS = {'Global%x_Release' % g: (a, g) for a, g in [
    (0x4f7a10, 0x7ced48), (0x699650, 0xfb1dc0), (0x699770, 0xfb1dbc), (0x699890, 0xfb1dc4), (0x6a5ae0, 0xfb1e54), (0x6a5c00, 0xfb1e30),
    (0x6a5d20, 0xfb1e58), (0x6a5e40, 0xfb1e34), (0x6a5f60, 0xfb1e38), (0x6a6080, 0xfb1e5c), (0x6a61a0, 0xfb1e60), (0x6a62c0, 0xfb1e64),
    (0x6a6360, 0xfb1e3c), (0x6a6400, 0xfb1e44), (0x6a64a0, 0xfb1e4c), (0x6ac0b0, 0xfb1ecc), (0x6ac1d0, 0xfb1eb4), (0x6ac2f0, 0xfb1eb8),
    (0x6ac410, 0xfb1ed0), (0x6ac530, 0xfb1ed4), (0x6ac650, 0xfb1ed8), (0x6ac6f0, 0xfb1ebc), (0x6ac790, 0xfb1ec4)]}
B = Bench([(a, a + 0x96) for a, _g in CALLS.values()], hooks={0x691de0: lock, 0x691ea0: lambda e: None})


def case(r):
    address, where = CALLS[r.deal(OPS)]
    regions = {LOG: log_region(), RAM + 0x1000: vtable({0: 1})}
    regions[where & ~0xf] = bytearray(r.bytes(0x20))
    shared = RAM + 0x200 if r.random() < 0.95 else 0
    regions[where & ~0xf][where & 0xf:(where & 0xf) + 4] = struct.pack('<I', shared)
    regions[RAM + 0x200] = struct.pack('<II', RAM + 0x1000, shared_count(r)) + r.bytes(0x20)
    return Call(address, [], regions, conv='c', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

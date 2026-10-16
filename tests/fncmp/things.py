# The delete queue's release of the thing table (src/game/world/things: 0x518ea0) against the original: slots in use chained newest first, handles
# queued (some of the same slot), the free list head at 0x801bbc; the things are freed through the heap region.
# usage: things.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from thingenv import park  # noqa: E402

B = Bench([(0x518ea0, 0x518f90), (0x5da3c0, 0x5da3c1)])
NODES = RAM + 0x3000


def case(r):
    regions, handles, _a = park(r, count=r.choice([1, 2, 3, 4, 6]), defs=False)
    n = r.choice([0, 1, 2, 3, 4])
    queued = [r.choice(handles) for _ in range(n)]
    nodes = bytearray(r.bytes(0x10 * 5))
    for i, h in enumerate(queued):
        put(nodes, 0xc * i, 'H', h)
        put(nodes, 0xc * i + 4, 'I', NODES + 0xc * (i + 1) if i + 1 < n else 0)
        put(nodes, 0xc * i + 8, 'I', NODES + 0xc * (i - 1) if i else 0)
    regions[0x801f38] = struct.pack('<IIII', r.getrandbits(32), NODES if n else 0, NODES + 0xc * (n - 1) if n else 0, n)
    regions[NODES] = nodes
    regions[0x801bbc] = struct.pack('<I', r.choice([0, 0x7cfb90 + 20 * 3, r.getrandbits(32)]))
    regions[HEAP] = heap()
    return Call(0x518ea0, [], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)

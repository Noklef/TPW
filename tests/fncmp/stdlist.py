# The list<string> and list<dword> members of src/game/core/stdlist against the originals: lists of 0 to 3 nodes around a sentinel, strings of the std::string bench
# (null, shared, frozen, short and long), the new nodes from the heap region.
# usage: stdlist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import stdstring  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Clear': (0x5cde80, 't', 'v', 0x80), 'Push': (0x5cdf00, 't', 'v', 0x70), 'Insert': (0x5d0330, 't', 'v', 0x80), 'Dword': (0x618160, 't', 'v', 0x50)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x45b340, 0x45b9a0), (0x67acb0, 0x67acb1)])
NODES, LIST, X = RAM + 0x3000, RAM, RAM + 0x2000


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    blocks = bytearray(r.bytes(6 * stdstring.SLOT))
    n = r.choice([0, 1, 2, 3])
    size = 0x18 if name != 'Dword' else 0xc
    nodes = bytearray(r.bytes(size * 4))
    at = [NODES + size * i for i in range(n + 1)]  # at[0] is the sentinel, the rest the nodes in order
    for i in range(n + 1):
        put(nodes, size * i, 'II', at[(i + 1) % (n + 1)], at[i - 1])
    for i in range(1, n + 1):
        if name != 'Dword':
            ptr, ln, res = stdstring.string(r, blocks, i)
            put(nodes, size * i + 8, '4I', r.getrandbits(32), ptr, ln, res)
    obj = bytearray(r.bytes(0x20))
    put(obj, 4, 'II', at[0], n)
    regions = {LIST: obj, NODES: nodes, stdstring.BLOCKS: blocks, HEAP: heap()}
    if name == 'Clear':
        return Call(address, [LIST], regions, conv=conv, ret=ret)
    if name == 'Dword':
        regions[X] = r.bytes(0x10)
        return Call(address, [LIST, X], regions, conv=conv, ret=ret)
    x = bytearray(r.bytes(0x20))
    ptr, ln, res = stdstring.string(r, blocks, 5)
    put(x, 0, '4I', r.getrandbits(32), ptr, ln, res)
    regions[X] = x
    if name == 'Push':
        return Call(address, [LIST, X], regions, conv=conv, ret=ret)
    regions[RAM + 0x2800] = r.bytes(0x10)
    return Call(address, [LIST, RAM + 0x2800, r.choice(at), X], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

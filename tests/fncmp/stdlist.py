# The list<string> and list<dword> members of src/game/core/stdlist against the originals: lists of 0 to 3 nodes around a sentinel, strings of the std::string bench
# (null, shared, frozen, short and long), the new nodes from the heap region. A record's destructor (0x5caf20): three such strings and a list of
# counted-buffer nodes (as Pair's). Chars (0x4a8ad0): a list at +0x10 of the owner, given characters (none to 0x7e, at any address: its low byte
# becomes the allocator byte) made into a string and pushed.
# usage: stdlist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import stdstring  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Clear': (0x5cde80, 't', 'v', 0x80), 'Push': (0x5cdf00, 't', 'v', 0x70), 'Insert': (0x5d0330, 't', 'v', 0x80), 'Dword': (0x618160, 't', 'v', 0x50),
         'Pair': (0x5af0e0, 't', 'v', 0xf4), 'Record': (0x5caf20, 't', 'v', 0x110),
         'Chars': (0x4a8ad0, 't', 'v', 0xfc)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x45b340, 0x45b9a0), (0x67acb0, 0x67acb1), (0x4f54e0, 0x4f5560), (0x47db20, 0x47db80),
                                                                (0x5cdf70, 0x5cdfdd), (0x4a9420, 0x4a953c)])
NODES, LIST, X = RAM + 0x3000, RAM, RAM + 0x2000
BUFS = RAM + 0x4000


def pair(r):
    """two lists of counted-buffer nodes (0x18 bytes: next, previous, a dword, the buffer, two dwords; the buffer's use count byte at -1)
    at +0 and +0xc of the object, 0-3 nodes each, counts theirs or not."""
    obj = bytearray(r.bytes(0x20))
    nodes = bytearray(r.bytes(0x18 * 8))
    bufs = bytearray(r.bytes(0x10 * 8))
    for k, base in enumerate((0, 0xc)):
        n = r.choice([0, 1, 2, 3])
        at = [NODES + 0x18 * (4 * k + i) for i in range(n + 1)]
        for i in range(n + 1):
            put(nodes, at[i] - NODES, 'II', at[(i + 1) % (n + 1)], at[i - 1])
        for i in range(1, n + 1):
            b = 4 * k + i
            bufs[0x10 * b + 3] = r.choice([0, 1, 2, 0xfe, 0xff])
            put(nodes, at[i] - NODES + 0xc, 'I', BUFS + 0x10 * b + 4 if r.random() < 0.8 else 0)
        put(obj, base + 4, 'II', at[0], r.choice([n, n, n + 1, 0]))
    return Call(0x5af0e0, [LIST], {LIST: obj, NODES: nodes, BUFS: bufs, HEAP: heap()}, conv='t', ret='v')


def record(r):
    """three std::strings (+0, +0x10, +0x2c; of the std::string bench) and a list of counted-buffer nodes (as pair()'s) at +0x20, 0-3 nodes,
    its count its own or not."""
    obj = bytearray(r.bytes(0x40))
    blocks = bytearray(r.bytes(4 * stdstring.SLOT))
    for k, base in enumerate((0, 0x10, 0x2c)):
        put(obj, base + 4, '3I', *stdstring.string(r, blocks, k))
    nodes = bytearray(r.bytes(0x18 * 4))
    bufs = bytearray(r.bytes(0x10 * 4))
    n = r.choice([0, 1, 2, 3])
    at = [NODES + 0x18 * i for i in range(n + 1)]
    for i in range(n + 1):
        put(nodes, at[i] - NODES, 'II', at[(i + 1) % (n + 1)], at[i - 1])
    for i in range(1, n + 1):
        bufs[0x10 * i + 3] = r.choice([0, 1, 2, 0xfe, 0xff])
        put(nodes, at[i] - NODES + 0xc, 'I', BUFS + 0x10 * i + 4 if r.random() < 0.8 else 0)
    put(obj, 0x24, 'II', at[0], r.choice([n, n, n + 1, 0]))
    regions = {LIST: obj, NODES: nodes, BUFS: bufs, stdstring.BLOCKS: blocks, HEAP: heap()}
    return Call(0x5caf20, [LIST], regions, conv='t', ret='v')


def case(r):
    name = r.deal(OPS)
    if name == 'Pair':
        return pair(r)
    if name == 'Record':
        return record(r)
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
    put(obj, 0x14 if name == 'Chars' else 4, 'II', at[0], n)
    regions = {LIST: obj, NODES: nodes, stdstring.BLOCKS: blocks, HEAP: heap()}
    if name == 'Clear':
        return Call(address, [LIST], regions, conv=conv, ret=ret)
    if name == 'Dword':
        regions[X] = r.bytes(0x10)
        return Call(address, [LIST, X], regions, conv=conv, ret=ret)
    if name == 'Chars':
        k = r.choice([0, 1, 2, r.randrange(0x20), r.randrange(0x7f)])
        off = r.randrange(0x40)
        text = bytearray(r.bytes(0xd0))
        text[off:off + k] = bytes(r.randrange(1, 0x100) for _ in range(k))
        text[off + k] = 0
        regions[X] = text
        return Call(address, [LIST, X + off], regions, conv=conv, ret=ret)
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

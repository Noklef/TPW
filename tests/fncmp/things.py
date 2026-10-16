# The thing table and its delete queue (src/game/world/things: 0x515f30, 0x516130, 0x516160, 0x5161b0, 0x516200, 0x516270, 0x516310, 0x518ea0,
# 0x51ad40, 0x51ad90) against the originals: slots in use chained newest first, handles queued (some of the same slot), the free list head at
# 0x801bbc; blocks come from and go to the heap region.
# usage: things.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...; Init is heavy (the whole 0x2800-slot table) and not in the default)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from thingenv import POOL, SLOTS, THING, park  # noqa: E402

# name: (address, size, convention, returns)
CALLS = {'Rel518': (0x518ea0, 0xf0, 'c', 'v'), 'First': (0x516130, 0x30, 't', 'a'), 'Next': (0x516160, 0x50, 't', 'a'), 'Prev': (0x5161b0, 0x50, 't', 'a'),
         'SlotRel': (0x516200, 0x70, 't', 'v'), 'Add': (0x516270, 0xa0, 't', 'a'), 'QDel': (0x516310, 0x20, 't', 'v'), 'Push': (0x51ad40, 0x50, 't', 'v'),
         'Pop': (0x51ad90, 0x50, 't', 'a'), 'Init': (0x515f30, 0x80, 'c', 'v')}
B = Bench([(a, a + n) for (a, n, _c, _r) in CALLS.values()] + [(0x50b350, 0x50b360), (0x51ad40, 0x51ad90), (0x5da3c0, 0x5da3c1)])
NODES, OUT = RAM + 0x3000, RAM + 0x300


def queue(r, handles, n):
    """A queue of n nodes {handle, next, prev} at NODES; its header words (unknown, head, tail, count)."""
    nodes = bytearray(r.bytes(0x10 * 5))
    for i in range(n):
        put(nodes, 0xc * i, 'H', r.choice(handles))
        put(nodes, 0xc * i + 4, 'I', NODES + 0xc * (i + 1) if i + 1 < n else 0)
        put(nodes, 0xc * i + 8, 'I', NODES + 0xc * (i - 1) if i else 0)
    return nodes, struct.pack('<IIII', r.getrandbits(32), NODES if n else 0, NODES + 0xc * (n - 1) if n else 0, n)


def case(r):
    name = r.choice(OPS)
    address, _n, conv, ret = CALLS[name]
    if name == 'Init':
        slots = bytearray(r.bytes(0x2800 * 20))
        return Call(address, [], {SLOTS: slots, 0x7cf56c: r.bytes(4), 0x801bbc: r.bytes(4)}, conv=conv, ret=ret)
    regions, handles, addrs = park(r, count=r.choice([1, 2, 3, 4, 6]), defs=False)
    regions[HEAP] = heap()
    regions[OUT] = r.bytes(8)
    regions[RAM] = r.bytes(0x40)
    n = r.choice([0, 1, 2, 3, 4])
    if name in ('Rel518', 'QDel', 'Push', 'Pop'):
        nodes, header = queue(r, handles, n)
        regions[NODES] = nodes
        if name == 'Rel518' or name == 'QDel':
            regions[0x801f38] = header
        else:
            regions[RAM] = bytearray(header) + r.bytes(0x30)
    regions[0x801bbc] = struct.pack('<I', r.choice([0, SLOTS + 20 * 3, SLOTS + 20 * 7, r.getrandbits(32)]) if name != 'Add' else r.choice([0, SLOTS + 20 * 3, SLOTS + 20 * 7]))
    k = r.randrange(len(handles))
    if name == 'Rel518':
        return Call(address, [], regions, conv=conv, ret=ret)
    if name == 'First':
        return Call(address, [RAM, OUT], regions, conv=conv, ret=ret)
    if name in ('Next', 'Prev'):
        return Call(address, [RAM, OUT, addrs[k]], regions, conv=conv, ret=ret)
    if name == 'SlotRel':
        return Call(address, [SLOTS + 20 * handles[k]], regions, conv=conv, ret=ret)
    if name == 'Add':
        regions[SLOTS + 20 * 3] = r.bytes(20) if r.random() < 0.5 else regions[SLOTS][20 * 3:20 * 4]
        return Call(address, [RAM, OUT, r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'QDel':
        return Call(address, [RAM, r.choice(handles + [0, 5, 0xffff, 0x12345])], regions, conv=conv, ret=ret)
    if name == 'Push':
        return Call(address, [RAM, r.choice(handles + [0, 5, 0xffff, 0x12345])], regions, conv=conv, ret=ret)
    return Call(address, [RAM, OUT], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(c for c in CALLS if c != 'Init')).split(',') if k]


if __name__ == '__main__':
    B.main(case)

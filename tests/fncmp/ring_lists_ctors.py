# The ring of words, ramp table, lookup table, kind check, 7028xx constructors, list erases and table writers of src/game/core/ring_lists_ctors against
# the originals: any object bytes and arguments (the allocator is the heap region).
# usage: ring_lists_ctors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Ring': (0x4caa70, 't', 'a', 0x70), 'Ramp': (0x55e710, 'c', 'v', 0x70), 'T76': (0x59c320, 's', 'b', 0x70), 'Kind': (0x535600, 't', 'a', 0x70),
         'C1': (0x5bce60, 't', 'a', 0x70), 'C2': (0x5bd2a0, 't', 'a', 0x70), 'C3': (0x5bd6b0, 't', 'a', 0x70), 'E68': (0x5cc700, 't', 'v', 0x70),
         'E74': (0x5cc830, 't', 'v', 0x70), 'Node': (0x5cdf70, 't', 'a', 0x70), 'Re3': (0x5e7840, 't', 'b', 0x70), 'Key': (0x5ed700, 'c', 'v', 0x70)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x5da3c0, 0x5da3c1)])
LIST, BUFS = RAM + 0x1000, RAM + 0x2000


def ring(r, n):
    """A circular list: the sentinel at LIST and n nodes of 0x20 bytes ({next, prev, key, ...}); the node addresses."""
    nodes = [LIST + 0x20 * (i + 1) for i in range(n)]
    order = [LIST] + nodes
    data = bytearray(r.bytes(0x20 * (n + 1)))
    for i, a in enumerate(order):
        put(data, a - LIST, 'II', order[(i + 1) % len(order)], order[i - 1])
        put(data, a - LIST + 8, 'I', r.choice([1, 2, 3]))
    return data, nodes


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x400))
    regions = {RAM: obj, HEAP: heap()}
    if name == 'Ring':
        base = RAM + 0x800
        rec = bytearray(r.bytes(0x100))
        for k in range(4):
            o = 0x40 + 20 * k
            put(rec, o + 8, 'ii', r.choice([0, 1, 2, 3, 5, 6]), r.choice([0, 1, 3, 4, 8]))
            rec[o + 0x10] = r.choice([0, 1, 2])
        regions[base] = rec
        which = r.randrange(4)
        this = base + 0x40 - 20 * 0x18d2
        return Call(address, [this, RAM + 0x100, which, r.choice([0, 1, 2, 3, 4, 5, 6, 8])], regions, conv='t', ret=ret)
    if name == 'Ramp':
        w = r.choice([0, 1, 5, 20, 100, 200, 254, 255, 300, 1000, -5, -100, 0x7fff, 0x12345])
        return Call(address, [RAM + 0x100, r.choice([0, 50, 128, 200, 255, 256, -3, 1000, 0x10010, r.getrandbits(32)]), w & 0xffffffff], regions, conv='c', ret=ret)
    if name == 'T76':  # the table of 56-byte records (ids at +4, mostly the slot's own number), which the game fills at run time
        table = bytearray(r.bytes(56 * 342 + 0x40))
        for i in range(342):
            put(table, 56 * i + 4, 'I', i if r.random() < 0.7 else r.choice([0, 1, 5, 17, r.randrange(342)]))
        regions[0x76e300] = table
        return Call(address, [r.choice([0, 1, 2, 5, 17, 100, 0x15e, 0x15f, 0x160, 0x161, 200, 300, 341])], regions, conv='s', ret=ret)
    if name == 'Kind':
        put(obj, 8, 'I', r.choice([0, 1, 3, 4, 0x15, 2]))
        regions[0x820ab8] = struct.pack('<I', r.choice([0, 0, 1]))
        return Call(address, [RAM, r.choice([0, 1, 3, 4, 0x15, 2, 7])], regions, conv='t', ret=ret)
    if name in ('C1', 'C2', 'C3'):
        return Call(address, [RAM], regions, conv='t', ret=ret)
    if name in ('E68', 'E74'):
        data, nodes = ring(r, r.choice([0, 1, 2, 3, 5]))
        regions[LIST] = data
        put(obj, 0x68 if name == 'E68' else 0x74, 'I', LIST)
        put(obj, 0x6c if name == 'E68' else 0x78, 'I', r.choice([len(nodes), len(nodes), 0, 100]))
        put(obj, 0x7c, 'I', RAM + 0x600 - 8)
        regions[RAM + 0x600] = put(bytearray(16), 8, 'I', r.choice([1, 2, 3, 4]))
        regions[RAM] = obj
        return Call(address, [RAM], regions, conv='t', ret=ret)
    if name == 'Node':
        data, nodes = ring(r, r.choice([1, 2, 3]))
        bufs = bytearray(r.bytes(0x100))
        for i, a in enumerate(nodes):
            use = r.choice([0, 1, 2, 5, 0xfe, 0xff])
            bufs[0x20 * i] = use
            put(data, a - LIST + 0xc, 'I', r.choice([0, BUFS + 0x20 * i + 1, BUFS + 0x20 * i + 1]))
        put(obj, 8, 'I', r.choice([len(nodes), 0, 100]))
        regions.update({LIST: data, BUFS: bufs})
        return Call(address, [RAM, RAM + 0x300, r.choice(nodes)], regions, conv='t', ret=ret)
    if name == 'Re3':
        put(obj, 0xc, 'I', r.choice([0, 0, HEAP + 0x40, RAM + 0x100]))
        return Call(address, [RAM] + [r.choice([0, 1, 2, 3, 4, 8]) for _ in range(3)], regions, conv='t', ret=ret)
    regions[0xf86918] = r.bytes(68 * 4)
    return Call(address, [r.choice([0, 1, 2]), r.choice([0, 1, 5, 0x100]), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='c', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

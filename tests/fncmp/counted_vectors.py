# The tiny constructors, containers and comparisons of src/game/core/counted_vectors against the originals: any object bytes, arguments and
# texts (the allocator is the heap region).
# usage: counted_vectors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Has': (0x506580, 't', 'a', 0x50), 'Ext': (0x50e260, 's', 'a', 0x50), 'B50': (0x5101f0, 't', 'a', 0x50), 'Push': (0x5e2a30, 't', 'a', 0x50),
         'Tex': (0x55f720, 't', 'v', 0x60), 'C80': (0x5b96e0, 't', 'a', 0x60), 'C90': (0x5b9740, 't', 'a', 0x60), 'Cmp': (0x5d1330, 't', 'a', 0x60),
         'V14': (0x4791c0, 't', 'v', 0x70), 'V4': (0x47aba0, 't', 'v', 0x70)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x5da3c0, 0x5da3c1)])
ARR, BUFS, TEXTS = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000
COUNTS = [0, 1, 2, 5, 0x7f, 0xfe, 0xff]


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x400))
    regions = {RAM: obj, HEAP: heap()}
    this = RAM + 0x80
    args = []
    if name == 'Has':
        obj[0x80 + 2] = r.choice([0, 3, 4, 4, 5, 5, 6, 0xff])
        w = r.choice([0, 1, 0x1234, 0xffff, r.getrandbits(16)])
        put(obj, 0x80 + 0x218, 'HH', w if r.random() < 0.5 else r.getrandbits(16), w if r.random() < 0.5 else r.getrandbits(16))
        args = [w | r.choice([0, 0, 0x10000, 0xabcd0000])]
    elif name == 'Ext':
        regions[0x7cf100] = r.bytes(8)
        return Call(address, [RAM + 0x180, RAM + 0x1c0], regions, conv='s', ret=ret)
    elif name == 'Push':
        node = RAM + 0x300
        empty = r.random() < 0.4
        put(obj, 0x80 + 4, 'II', 0 if empty else node, r.getrandbits(32))
        put(obj, 0x80 + 0xc, 'I', r.choice([0, 1, 5, 0xffffffff]))
        regions[node] = r.bytes(0x20)
        args = [r.choice([0, 1, 0x1234, r.getrandbits(32)])]
    elif name == 'Tex':
        regions[0x8798c8] = put(bytearray(4), 0, 'I', r.choice([0, 0, HEAP + 0x40, RAM + 0x100]))
        args = [r.choice([0x40, 0x20, 0x10, 0, 1, 0x30, 0x60, 0x140, 0xffffffff, r.getrandbits(32)])]
    elif name == 'Cmp':
        text = bytearray(r.choice([1, 2, 0x41, 0x42, 0xfe, 0xff]) for _ in range(0x20))
        if r.random() < 0.5:  # the same text, so the lengths decide
            text[0x10:0x18] = text[0:8]
        put(obj, 0x80 + 4, 'II', TEXTS, r.choice([0, 1, 2, 3, 5, 8]))
        put(obj, 0x100 + 4, 'II', TEXTS + 0x10 if r.random() < 0.75 else 0, r.choice([0, 1, 2, 3, 5, 8]))
        regions[TEXTS] = text
        args = [RAM + 0x100]
    elif name in ('V14', 'V4'):
        off = 0x14 if name == 'V14' else 4
        n = r.choice([0, 1, 2, 3])
        data = bytearray(r.bytes(0x100))
        for i in range(n):
            put(data, 0x10 * i + 4, 'I', r.choice([0, BUFS + 3 + 0x20 * i, BUFS + 3 + 0x20 * i]))
        bufs = bytearray(r.bytes(0x100))
        for i in range(n):
            bufs[0x20 * i + 2] = r.choice(COUNTS)
        put(obj, 0x80 + off, 'II', ARR if n or r.random() < 0.5 else 0, ARR + 0x10 * n if n else (ARR if r.random() < 0.5 else 0))
        regions.update({ARR: data, BUFS: bufs})
    return Call(address, [this] + args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

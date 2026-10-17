# The counted buffer releases, the pointer array removal and the list range erase of src/game/core/small6 against the originals:
# buffers with use counts around the "last user" values, entries and nodes of any shape (the allocator is the heap region).
# usage: small6.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Entries': (0x47b5b0, 's', 'v', 0x50), 'Reset': (0x4f23f0, 't', 'v', 0x50), 'Del': (0x47db20, 't', 'a', 0x60),
         'Remove': (0x56b5c0, 't', 'a', 0x60), 'Erase': (0x5ce540, 't', 'a', 0x70)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()])
BUFS, ARR, LIST = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000
COUNTS = [0, 1, 2, 5, 0x7f, 0xfe, 0xff]


def buffers(r, n):
    """n counted buffers (use count byte at -1) in BUFS; their pointers (some 0)."""
    data = bytearray(r.bytes(0x20 * n + 8))
    ptrs = []
    for i in range(n):
        data[0x20 * i + 3 - 1 + 0] = r.choice(COUNTS)
        ptrs.append(BUFS + 0x20 * i + 3 if r.random() < 0.8 else 0)
    return data, ptrs


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x200))
    regions = {RAM: obj, HEAP: heap()}
    args = []
    if name == 'Entries':
        n = r.choice([0, 1, 2, 3])
        data, ptrs = buffers(r, n)
        entries = bytearray(r.bytes(0x10 * 4))
        for i in range(n):
            put(entries, 0x10 * i + 4, 'I', ptrs[i])
        regions.update({BUFS: data, ARR: entries})
        return Call(address, [ARR, ARR + 0x10 * n], regions, conv='s', ret=ret)
    if name in ('Reset', 'Del'):
        data, ptrs = buffers(r, 1)
        put(obj, 4, 'III', ptrs[0], r.getrandbits(32), r.getrandbits(32))
        regions[BUFS] = data
        args = [r.choice([0, 1, 2, 0x100, 0xff00, r.getrandbits(32)])]
    elif name == 'Remove':
        n = r.choice([0, 1, 2, 3, 4])
        items = [r.choice([0, 1, 0x30004000 + 0x10 * i, 0xffffffff]) for i in range(n)]
        arr = bytearray(r.bytes(0x20))
        for i, v in enumerate(items):
            put(arr, 4 * i, 'I', v)
        put(obj, 0x1c, 'i', r.choice([n, n, n, n + 1, 0, -1]))
        put(obj, 8, 'I', ARR)
        put(obj, 4, 'I', RAM + 0x100)
        regions[ARR] = arr
        args = [r.choice(items + [0, 5, 0x30004000]) if items else r.choice([0, 5])]
    elif name == 'Erase':
        n = r.choice([0, 1, 2, 3, 4])
        nodes = [LIST + 0x10 * (i + 1) for i in range(n)]
        ring = [LIST] + nodes  # the sentinel and the nodes, circular
        data = bytearray(r.bytes(0x80))
        for i, a in enumerate(ring):
            put(data, a - LIST, 'II', ring[(i + 1) % len(ring)], ring[i - 1])
        put(obj, 8, 'i', r.choice([n, n, n + 1, 0, -1]))
        regions[LIST] = data
        first, last = r.choice(ring), r.choice(ring)
        args = [RAM + 0x180, first, last]
    return Call(address, [RAM] + args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

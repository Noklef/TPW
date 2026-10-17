# The wrappers and clears over counted buffers and vectors of src/game/core/counted_vectors_clear against the originals: vectors of counted entries,
# lists of counted nodes, any object bytes (the allocator is the heap region).
# usage: counted_vectors_clear.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

CALLS = {'Del': (0x47db80, 't', 'a'), 'Push': (0x4d3850, 't', 'a'), 'Range': (0x47bfe0, 's', 'v'), 'Clear': (0x4f04d0, 't', 'v'),
         'C2': (0x5c9970, 't', 'v'), 'Pair': (0x4e51a0, 't', 'v'), 'PRange': (0x4f36f0, 's', 'v'), 'PVec': (0x4f0480, 't', 'a'),
         'PVecDel': (0x4f7430, 't', 'a'), 'PDel': (0x4f7480, 't', 'a')}
B = Bench([(0x47db80, 0x47dba0), (0x4791c0, 0x479230), (0x4d3850, 0x4d3860), (0x5e2a30, 0x5e2a80), (0x47bfe0, 0x47c010), (0x4f04d0, 0x4f0540),
           (0x4f75e0, 0x4f7620), (0x5c9970, 0x5c99f0), (0x5da3c0, 0x5da3c1), (0x4e51a0, 0x4e520e),
           (0x4f36f0, 0x4f3711), (0x4f0480, 0x4f04d0), (0x4f7430, 0x4f747d), (0x4f7480, 0x4f749e)])
LIST, BUFS, ARR = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000
COUNTS = [0, 1, 2, 5, 0x7f, 0xfe, 0xff]


def vector(r, obj, base, arr, bufs, n, off):
    """A vector of n 16-byte entries (each with a counted buffer at +4) at arr, begin/end/capacity fields at base+off."""
    for i in range(n):
        put(arr, 0x10 * i + 4, 'I', r.choice([0, BUFS + 0x20 * i + 3, BUFS + 0x20 * i + 3]))
        bufs[0x20 * i + 2] = r.choice(COUNTS)
    put(obj, base + off, 'II', ARR if n or r.random() < 0.5 else 0, ARR + 0x10 * n if n else (ARR if r.random() < 0.5 else 0))


def pair(r, data, base, bufs, slot):
    """A 0x24-byte element at data + base: the wide string's counted buffer at +0x14 (block 2 bytes below the pointer), the narrow one's at +4 (1 below)."""
    for off, k in ((0x14, 0), (4, 1)):
        at = 0x20 * (2 * slot + k)
        bufs[at + 1] = r.choice(COUNTS)
        put(data, base + off, 'I', r.choice([0, BUFS + at + 2, BUFS + at + 2]))


def case(r):
    name = r.choice(OPS)
    address, conv, ret = CALLS[name]
    obj = bytearray(r.bytes(0x400))
    regions = {RAM: obj, HEAP: heap()}
    arr, bufs = bytearray(r.bytes(0x100)), bytearray(r.bytes(0x100))
    if name == 'Del':
        vector(r, obj, 0x80, arr, bufs, r.choice([0, 1, 2, 3]), 0x14)
        regions.update({ARR: arr, BUFS: bufs})
        return Call(address, [RAM + 0x80, r.choice([0, 1, 2, 0xffffffff])], regions, conv='t', ret=ret)
    if name == 'Push':
        node = RAM + 0x300
        put(obj, 0x80 + 4, 'II', 0 if r.random() < 0.4 else node, r.getrandbits(32))
        put(obj, 0x80 + 0xc, 'I', r.choice([0, 1, 5]))
        regions[node] = r.bytes(0x20)
        return Call(address, [RAM + 0x80, r.choice([0, 1, 0x1234, r.getrandbits(32)])], regions, conv='t', ret=ret)
    if name == 'Range':
        # up to two 0x24-byte elements, each a vector at +0x14; both share the first n entries (the buffers are released twice, as the original would)
        count = r.choice([0, 1, 2])
        vector(r, obj, 0x80, arr, bufs, r.choice([0, 1, 2]), 0x14)
        if count > 1:
            put(obj, 0x80 + 0x24 + 0x14, 'II', *struct_pair(obj, 0x80 + 0x14))
        regions.update({ARR: arr, BUFS: bufs})
        return Call(address, [RAM + 0x80, RAM + 0x80 + 0x24 * count], regions, conv='s', ret=ret)
    if name == 'Clear':
        n = r.choice([0, 1, 2, 3])
        nodes = [LIST + 0x30 * (i + 1) for i in range(n)]
        order = [LIST] + nodes
        data = bytearray(r.bytes(0x30 * (n + 1)))
        for i, a in enumerate(order):
            put(data, a - LIST, 'II', order[(i + 1) % len(order)], order[i - 1])
            bufs[0x30 * i + 2] = r.choice(COUNTS)
            put(data, a - LIST + 0xc, 'I', r.choice([0, BUFS + 0x30 * i + 3, BUFS + 0x30 * i + 3]))
        put(obj, 0x80 + 4, 'I', LIST)
        put(obj, 0x80 + 8, 'I', r.choice([n, n + 1, 0, 100]))
        regions.update({LIST: data, BUFS: bufs})
        return Call(address, [RAM + 0x80], regions, conv='t', ret=ret)
    if name in ('PRange', 'PVec', 'PVecDel'):  # up to four 0x24-byte elements (each a pair of counted buffers) at ARR
        n = r.choice([0, 1, 2, 3, 4])
        bufs, elements = bytearray(r.bytes(0x400)), bytearray(r.bytes(0x24 * 4))
        for i in range(n):
            pair(r, elements, 0x24 * i, bufs, i)
        regions.update({ARR: elements, BUFS: bufs})
        if name == 'PRange':
            return Call(address, [ARR, ARR + 0x24 * n], regions, conv='s', ret=ret)
        begin = ARR if n or r.random() < 0.5 else 0  # an empty vector: begin == end, 0 or not
        put(obj, 0x80 + 4, 'II', begin, begin + 0x24 * n)
        args = [RAM + 0x80] + ([r.choice([0, 1, 2, 3, 0xffffffff])] if name == 'PVecDel' else [])
        return Call(address, args, regions, conv='t', ret=ret)
    if name == 'PDel':
        bufs = bytearray(r.bytes(0x100))
        pair(r, obj, 0x80, bufs, 0)
        regions[BUFS] = bufs
        return Call(address, [RAM + 0x80, r.choice([0, 1, 2, 3, 0xffffffff])], regions, conv='t', ret=ret)
    if name == 'Pair':  # the wide string's buffer at +0x14 (block 2 bytes below the pointer), the narrow one's at +4 (1 byte below)
        for off, k, below in ((0x14, 0, 2), (4, 1, 1)):
            bufs[0x20 * k + 2 - 1] = r.choice(COUNTS)
            put(obj, 0x80 + off, 'I', r.choice([0, BUFS + 0x20 * k + 2, BUFS + 0x20 * k + 2]))
        regions[BUFS] = bufs
        return Call(address, [RAM + 0x80], regions, conv='t', ret=ret)
    # C2: the block at +0x20 and the two counted buffers (count byte first: the block is freed 1 byte below the pointer)
    put(obj, 0x80 + 0x20, 'I', r.choice([0, HEAP + 0x40, RAM + 0x200]))
    for off, k in ((0x14, 0), (4, 1)):
        bufs[0x20 * k] = r.choice(COUNTS)
        put(obj, 0x80 + off, 'I', r.choice([0, BUFS + 0x20 * k + 1, BUFS + 0x20 * k + 1]))
    regions[BUFS] = bufs
    return Call(address, [RAM + 0x80], regions, conv='t', ret=ret)


def struct_pair(obj, off):
    import struct
    return struct.unpack_from('<II', obj, off)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

# The tiny functions of src/game/core/list_removes (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: list_removes.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)
FUNCS = [
    (0x5d8a80, 0x5d8b00, 't', 1, 'v', [], ()),
    (0x5da180, 0x5da200, 't', 1, 'v', [], ()),
    (0x5da450, 0x5da4d0, 't', 1, 'v', [], ()),
    (0x5dd4b0, 0x5dd530, 't', 1, 'v', [], ()),
    (0x5e15c0, 0x5e1640, 't', 1, 'v', [], ()),
    (0x5e4250, 0x5e42d0, 't', 1, 'v', [], ()),
    (0x5f42f0, 0x5f4365, 't', 1, 'v', [], ()),
]
EXTRA = []
LISTY = ()  # linked list destructors: a real chain (next at +4) is built for them
B = Bench([(a, e) for a, e, *_ in FUNCS] + EXTRA)


def setup_5d8a80(r, obj, regions):
    LIST = RAM + 0x1000
    count = r.choice([0, 1, 2, 3, 5])
    nodes = bytearray(r.bytes(0x100))
    values = [r.choice([1, 2, 3, 4, 5, 6, 7, 0x1000]) for _ in range(count)]
    for i in range(count):  # nodes 0x10 apart: the value, the next, the previous
        put(nodes, 0x10 * i, 'III', values[i], LIST + 0x100 + 0x10 * (i + 1) if i + 1 < count else 0, LIST + 0x100 + 0x10 * (i - 1) if i else 0)
    head = LIST + 0x100 if count else 0
    tail = LIST + 0x100 + 0x10 * (count - 1) if count else 0
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'III', head, tail, count)
    regions[LIST] = lst
    regions[LIST + 0x100] = nodes
    put(obj, 0x80 + 0xc, 'I', LIST)  # the object is at RAM + 0x80
    regions[HEAP] = heap(0x1000)
    return [r.choice(values + [99, 0]) if values else r.choice([0, 1])]


def setup_5da180(r, obj, regions):
    LIST = RAM + 0x1000
    count = r.choice([0, 1, 2, 3, 5])
    nodes = bytearray(r.bytes(0x100))
    values = [r.choice([1, 2, 3, 4, 5, 6, 7, 0x1000]) for _ in range(count)]
    for i in range(count):  # nodes 0x10 apart: the value, the next, the previous
        put(nodes, 0x10 * i, 'III', values[i], LIST + 0x100 + 0x10 * (i + 1) if i + 1 < count else 0, LIST + 0x100 + 0x10 * (i - 1) if i else 0)
    head = LIST + 0x100 if count else 0
    tail = LIST + 0x100 + 0x10 * (count - 1) if count else 0
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'III', head, tail, count)
    regions[LIST] = lst
    regions[LIST + 0x100] = nodes
    put(obj, 0x80 + 0x58, 'I', LIST)  # the object is at RAM + 0x80
    regions[HEAP] = heap(0x1000)
    return [r.choice(values + [99, 0]) if values else r.choice([0, 1])]


def setup_5da450(r, obj, regions):
    LIST = RAM + 0x1000
    count = r.choice([0, 1, 2, 3, 5])
    nodes = bytearray(r.bytes(0x100))
    values = [r.choice([1, 2, 3, 4, 5, 6, 7, 0x1000]) for _ in range(count)]
    for i in range(count):  # nodes 0x10 apart: the value, the next, the previous
        put(nodes, 0x10 * i, 'III', values[i], LIST + 0x100 + 0x10 * (i + 1) if i + 1 < count else 0, LIST + 0x100 + 0x10 * (i - 1) if i else 0)
    head = LIST + 0x100 if count else 0
    tail = LIST + 0x100 + 0x10 * (count - 1) if count else 0
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'III', head, tail, count)
    regions[LIST] = lst
    regions[LIST + 0x100] = nodes
    put(obj, 0x80 + 0x4, 'I', LIST)  # the object is at RAM + 0x80
    regions[HEAP] = heap(0x1000)
    return [r.choice(values + [99, 0]) if values else r.choice([0, 1])]


def setup_5dd4b0(r, obj, regions):
    LIST = RAM + 0x1000
    count = r.choice([0, 1, 2, 3, 5])
    nodes = bytearray(r.bytes(0x100))
    values = [r.choice([1, 2, 3, 4, 5, 6, 7, 0x1000]) for _ in range(count)]
    for i in range(count):  # nodes 0x10 apart: the value, the next, the previous
        put(nodes, 0x10 * i, 'III', values[i], LIST + 0x100 + 0x10 * (i + 1) if i + 1 < count else 0, LIST + 0x100 + 0x10 * (i - 1) if i else 0)
    head = LIST + 0x100 if count else 0
    tail = LIST + 0x100 + 0x10 * (count - 1) if count else 0
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'III', head, tail, count)
    regions[LIST] = lst
    regions[LIST + 0x100] = nodes
    put(obj, 0x80 + 0x6c, 'I', LIST)  # the object is at RAM + 0x80
    regions[HEAP] = heap(0x1000)
    return [r.choice(values + [99, 0]) if values else r.choice([0, 1])]


def setup_5e15c0(r, obj, regions):
    LIST = RAM + 0x1000
    count = r.choice([0, 1, 2, 3, 5])
    nodes = bytearray(r.bytes(0x100))
    values = [r.choice([1, 2, 3, 4, 5, 6, 7, 0x1000]) for _ in range(count)]
    for i in range(count):  # nodes 0x10 apart: the value, the next, the previous
        put(nodes, 0x10 * i, 'III', values[i], LIST + 0x100 + 0x10 * (i + 1) if i + 1 < count else 0, LIST + 0x100 + 0x10 * (i - 1) if i else 0)
    head = LIST + 0x100 if count else 0
    tail = LIST + 0x100 + 0x10 * (count - 1) if count else 0
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'III', head, tail, count)
    regions[LIST] = lst
    regions[LIST + 0x100] = nodes
    put(obj, 0x80 + 0x8, 'I', LIST)  # the object is at RAM + 0x80
    regions[HEAP] = heap(0x1000)
    return [r.choice(values + [99, 0]) if values else r.choice([0, 1])]


def setup_5e4250(r, obj, regions):
    LIST = RAM + 0x1000
    count = r.choice([0, 1, 2, 3, 5])
    nodes = bytearray(r.bytes(0x100))
    values = [r.choice([1, 2, 3, 4, 5, 6, 7, 0x1000]) for _ in range(count)]
    for i in range(count):  # nodes 0x10 apart: the value, the next, the previous
        put(nodes, 0x10 * i, 'III', values[i], LIST + 0x100 + 0x10 * (i + 1) if i + 1 < count else 0, LIST + 0x100 + 0x10 * (i - 1) if i else 0)
    head = LIST + 0x100 if count else 0
    tail = LIST + 0x100 + 0x10 * (count - 1) if count else 0
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'III', head, tail, count)
    regions[LIST] = lst
    regions[LIST + 0x100] = nodes
    put(obj, 0x80 + 0x18, 'I', LIST)  # the object is at RAM + 0x80
    regions[HEAP] = heap(0x1000)
    return [r.choice(values + [99, 0]) if values else r.choice([0, 1])]


def setup_5f42f0(r, obj, regions):
    out = setup_5e4250(r, obj, regions)   # (the list's pointer at +0x18 too: harmless)
    regions[RAM + 0x80 + 0x20080] = struct.pack('<I', RAM + 0x1000)   # the list at +0x20080, past the object's bytes
    return out


SETUP = {0x5f42f0: setup_5f42f0, 0x5d8a80: setup_5d8a80, 0x5da180: setup_5da180, 0x5da450: setup_5da450, 0x5dd4b0: setup_5dd4b0, 0x5e15c0: setup_5e15c0, 0x5e4250: setup_5e4250}


def case(r):
    address, _end, conv, n, ret, globs, ptr = r.choice(FUNCS)
    obj = bytearray(r.bytes(0x400))
    for k in range(0, 0x400, 4):  # many dwords point into the object, so a chain of pointers can be followed
        if r.random() < 0.5:
            put(obj, k, 'I', RAM + 0x100 + 4 * r.randrange(0x40))
    regions = {RAM: obj}
    regions[HEAP] = heap(0x1000)  # the game heap the code frees into
    override = SETUP[address](r, obj, regions) if address in SETUP else None
    for g in globs:
        regions[g & ~0xf] = r.bytes(0x20)
    args = [r.choice([0, 1, 0xff, 0x7fffffff, 0xffffffff, r.getrandbits(32)]) for _ in range(n)]
    for i in ptr:
        args[i] = RAM + 0x200 + 0x40 * min(i, 3) + r.choice([0, 4, 0x20])
    if override is not None:
        args = list(override)
    if conv == 't':
        args = [RAM + 0x80] + args
    return Call(address, args, regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)

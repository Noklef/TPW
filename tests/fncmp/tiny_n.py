# The tiny functions of src/game/core/tiny_n (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: tiny_n.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)
FUNCS = [
    (0x59c240, 0x59c2b0, 's', 1, 'a', [], ()),
    (0x59c2b0, 0x59c320, 's', 1, 'a', [], ()),
    (0x59c490, 0x59c500, 's', 1, 'a', [], ()),
    (0x59c500, 0x59c570, 's', 1, 'a', [], ()),
]
EXTRA = [(6136768, 6136769)]
LISTY = ()  # linked list destructors: a real chain (next at +4) is built for them
B = Bench([(a, e) for a, e, *_ in FUNCS] + EXTRA)


def setup_59c240(r, obj, regions):
    table = bytearray(r.bytes(0x5000))
    for i in range(0x15e):  # the table: most records hold their own index as the id
        put(table, 56 * i + 4, 'I', i if r.random() < 0.7 else r.choice([0, 1, 5, 0x10, 0x15d, 0x150, r.randrange(0, 0x15e)]))
    x = r.choice([0, 1, 2, 5, 0x10, 0x14f, 0x150, 0x15d, 0x15e, 0x15f, 0x160, 0x161, r.randrange(0, 0x15e), r.randrange(0, 0x15e), 0x400])
    if x in (0x15f, 0x160) and r.random() < 0.7:  # the special ids are never looked for, even when the table holds them
        put(table, 56 * x + 4, 'I', x)
        put(table, 56 * r.choice([0x15d, 0, 5]) + 4, 'I', x)
    if r.random() < 0.5 and x < 0x15e:  # the record by index does not have the id; another one, perhaps the last, does
        put(table, 56 * x + 4, 'I', (x + 1) & 0xffff)
        put(table, 56 * r.choice([0x15d, 0x15c, 0, 1, r.randrange(0, 0x15e)]) + 4, 'I', x)
    regions[0x76e300] = table
    return [x]


def setup_59c2b0(r, obj, regions):
    table = bytearray(r.bytes(0x5000))
    for i in range(0x15e):  # the table: most records hold their own index as the id
        put(table, 56 * i + 4, 'I', i if r.random() < 0.7 else r.choice([0, 1, 5, 0x10, 0x15d, 0x150, r.randrange(0, 0x15e)]))
    x = r.choice([0, 1, 2, 5, 0x10, 0x14f, 0x150, 0x15d, 0x15e, 0x15f, 0x160, 0x161, r.randrange(0, 0x15e), r.randrange(0, 0x15e), 0x400])
    if x in (0x15f, 0x160) and r.random() < 0.7:  # the special ids are never looked for, even when the table holds them
        put(table, 56 * x + 4, 'I', x)
        put(table, 56 * r.choice([0x15d, 0, 5]) + 4, 'I', x)
    if r.random() < 0.5 and x < 0x15e:  # the record by index does not have the id; another one, perhaps the last, does
        put(table, 56 * x + 4, 'I', (x + 1) & 0xffff)
        put(table, 56 * r.choice([0x15d, 0x15c, 0, 1, r.randrange(0, 0x15e)]) + 4, 'I', x)
    regions[0x76e300] = table
    return [x]


def setup_59c490(r, obj, regions):
    table = bytearray(r.bytes(0x5000))
    for i in range(0x15e):  # the table: most records hold their own index as the id
        put(table, 56 * i + 4, 'I', i if r.random() < 0.7 else r.choice([0, 1, 5, 0x10, 0x15d, 0x150, r.randrange(0, 0x15e)]))
    x = r.choice([0, 1, 2, 5, 0x10, 0x14f, 0x150, 0x15d, 0x15e, 0x15f, 0x160, 0x161, r.randrange(0, 0x15e), r.randrange(0, 0x15e), 0x400])
    if x in (0x15f, 0x160) and r.random() < 0.7:  # the special ids are never looked for, even when the table holds them
        put(table, 56 * x + 4, 'I', x)
        put(table, 56 * r.choice([0x15d, 0, 5]) + 4, 'I', x)
    if r.random() < 0.5 and x < 0x15e:  # the record by index does not have the id; another one, perhaps the last, does
        put(table, 56 * x + 4, 'I', (x + 1) & 0xffff)
        put(table, 56 * r.choice([0x15d, 0x15c, 0, 1, r.randrange(0, 0x15e)]) + 4, 'I', x)
    regions[0x76e300] = table
    return [x]


def setup_59c500(r, obj, regions):
    table = bytearray(r.bytes(0x5000))
    for i in range(0x15e):  # the table: most records hold their own index as the id
        put(table, 56 * i + 4, 'I', i if r.random() < 0.7 else r.choice([0, 1, 5, 0x10, 0x15d, 0x150, r.randrange(0, 0x15e)]))
    x = r.choice([0, 1, 2, 5, 0x10, 0x14f, 0x150, 0x15d, 0x15e, 0x15f, 0x160, 0x161, r.randrange(0, 0x15e), r.randrange(0, 0x15e), 0x400])
    if x in (0x15f, 0x160) and r.random() < 0.7:  # the special ids are never looked for, even when the table holds them
        put(table, 56 * x + 4, 'I', x)
        put(table, 56 * r.choice([0x15d, 0, 5]) + 4, 'I', x)
    if r.random() < 0.5 and x < 0x15e:  # the record by index does not have the id; another one, perhaps the last, does
        put(table, 56 * x + 4, 'I', (x + 1) & 0xffff)
        put(table, 56 * r.choice([0x15d, 0x15c, 0, 1, r.randrange(0, 0x15e)]) + 4, 'I', x)
    regions[0x76e300] = table
    return [x]


SETUP = {0x59c240: setup_59c240, 0x59c2b0: setup_59c2b0, 0x59c490: setup_59c490, 0x59c500: setup_59c500}


def case(r):
    address, _end, conv, n, ret, globs, ptr = r.choice(FUNCS)
    obj = bytearray(r.bytes(0x400))
    for k in range(0, 0x400, 4):  # many dwords point into the object, so a chain of pointers can be followed
        if r.random() < 0.5:
            put(obj, k, 'I', RAM + 0x100 + 4 * r.randrange(0x40))
    regions = {RAM: obj}
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

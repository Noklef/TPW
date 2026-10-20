# The tiny functions of src/game/core/world_table_getters (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: world_table_getters.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap, put, off_code  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)
FUNCS = [
    (0x519450, 0x519490, 't', 0, 'a', [], ()),
    (0x519490, 0x5194d0, 't', 0, 'a', [], ()),
    (0x5194d0, 0x519510, 't', 0, 'a', [], ()),
    (0x519510, 0x519550, 't', 0, 'a', [], ()),
    (0x519550, 0x519590, 't', 0, 'a', [], ()),
    (0x519590, 0x5195d0, 't', 0, 'a', [], ()),
    (0x5195d0, 0x519610, 't', 0, 'a', [], ()),
    (0x519610, 0x519650, 't', 0, 'a', [], ()),
]
EXTRA = [(6136768, 6136769)]
LISTY = ()  # linked list destructors: a real chain (next at +4) is built for them
B = Bench([(a, e) for a, e, *_ in FUNCS] + EXTRA)


def setup_519450(r, obj, regions):
    base = RAM + 0x80 + 0x1da718  # the field is far out in the game object: give it a window
    window = bytearray(r.bytes(0x20))
    put(window, base & 0xf, 'H', r.choice([0, 1, 2, 3, 5, 7, 8]))
    regions[base & ~0xf] = window
    regions[0x7cfb90] = r.bytes(0xb0)  # the table, 20 bytes an entry


def setup_519490(r, obj, regions):
    base = RAM + 0x80 + 0x1da71a  # the field is far out in the game object: give it a window
    window = bytearray(r.bytes(0x20))
    put(window, base & 0xf, 'H', r.choice([0, 1, 2, 3, 5, 7, 8]))
    regions[base & ~0xf] = window
    regions[0x7cfb90] = r.bytes(0xb0)  # the table, 20 bytes an entry


def setup_5194d0(r, obj, regions):
    base = RAM + 0x80 + 0x1da71c  # the field is far out in the game object: give it a window
    window = bytearray(r.bytes(0x20))
    put(window, base & 0xf, 'H', r.choice([0, 1, 2, 3, 5, 7, 8]))
    regions[base & ~0xf] = window
    regions[0x7cfb90] = r.bytes(0xb0)  # the table, 20 bytes an entry


def setup_519510(r, obj, regions):
    base = RAM + 0x80 + 0x1da720  # the field is far out in the game object: give it a window
    window = bytearray(r.bytes(0x20))
    put(window, base & 0xf, 'H', r.choice([0, 1, 2, 3, 5, 7, 8]))
    regions[base & ~0xf] = window
    regions[0x7cfb90] = r.bytes(0xb0)  # the table, 20 bytes an entry


def setup_519550(r, obj, regions):
    base = RAM + 0x80 + 0x1da722  # the field is far out in the game object: give it a window
    window = bytearray(r.bytes(0x20))
    put(window, base & 0xf, 'H', r.choice([0, 1, 2, 3, 5, 7, 8]))
    regions[base & ~0xf] = window
    regions[0x7cfb90] = r.bytes(0xb0)  # the table, 20 bytes an entry


def setup_519590(r, obj, regions):
    base = RAM + 0x80 + 0x1da724  # the field is far out in the game object: give it a window
    window = bytearray(r.bytes(0x20))
    put(window, base & 0xf, 'H', r.choice([0, 1, 2, 3, 5, 7, 8]))
    regions[base & ~0xf] = window
    regions[0x7cfb90] = r.bytes(0xb0)  # the table, 20 bytes an entry


def setup_5195d0(r, obj, regions):
    base = RAM + 0x80 + 0x1da726  # the field is far out in the game object: give it a window
    window = bytearray(r.bytes(0x20))
    put(window, base & 0xf, 'H', r.choice([0, 1, 2, 3, 5, 7, 8]))
    regions[base & ~0xf] = window
    regions[0x7cfb90] = r.bytes(0xb0)  # the table, 20 bytes an entry


def setup_519610(r, obj, regions):
    base = RAM + 0x80 + 0x1da736  # the field is far out in the game object: give it a window
    window = bytearray(r.bytes(0x20))
    put(window, base & 0xf, 'H', r.choice([0, 1, 2, 3, 5, 7, 8]))
    regions[base & ~0xf] = window
    regions[0x7cfb90] = r.bytes(0xb0)  # the table, 20 bytes an entry


SETUP = {0x519450: setup_519450, 0x519490: setup_519490, 0x5194d0: setup_5194d0, 0x519510: setup_519510, 0x519550: setup_519550, 0x519590: setup_519590, 0x5195d0: setup_5195d0, 0x519610: setup_519610}


def case(r):
    address, _end, conv, n, ret, globs, ptr = r.choice(FUNCS)
    obj = bytearray(r.bytes(0x400))
    for k in range(0, 0x400, 4):  # many dwords point into the object, so a chain of pointers can be followed
        if r.random() < 0.5:
            put(obj, k, 'I', RAM + 0x100 + 4 * r.randrange(0x40))
    off_code(obj)   # (a random dword in the exe's code would read the driver's code natively)
    regions = {RAM: obj}
    if address in SETUP:
        SETUP[address](r, obj, regions)
    for g in globs:
        regions[g & ~0xf] = r.bytes(0x20)
    args = [r.choice([0, 1, 0xff, 0x7fffffff, 0xffffffff, r.getrandbits(32)]) for _ in range(n)]
    for i in ptr:
        args[i] = RAM + 0x200 + 0x40 * min(i, 3) + r.choice([0, 4, 0x20])
    if conv == 't':
        args = [RAM + 0x80] + args
    return Call(address, args, regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)

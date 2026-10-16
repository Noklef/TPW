# The tiny getters, setters and stubs of src/game/core/accessors against the originals: any object
# bytes, any global values, any arguments.
# usage: accessors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Get24,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, convention, stack arguments, returns, the global it touches)
CALLS = {
    'Get24': (0x423720, 't', 0, 'a', None), 'Get2c': (0x423730, 't', 0, 'a', None), 'Get30': (0x4030c0, 't', 0, 'a', None),
    'SetFirst': (0x423b50, 't', 1, 'a', None), 'Get188': (0x4e1a10, 't', 0, 'a', None), 'Set188': (0x4e1a20, 't', 1, 'a', None),
    'Get194': (0x4e1a00, 't', 0, 'a', None), 'Set194': (0x4e2770, 't', 1, 'a', None), 'Byte18c': (0x4e2180, 't', 0, 'b', None),
    'Inc2b8': (0x4e19f0, 't', 0, 'v', None), 'V6fd818': (0x4df280, 't', 0, 'v', None), 'SetByte7': (0x47ebb0, 't', 2, 'b', None),
    'Ret16': (0x477190, 's', 4, 'v', None), 'Ret4b': (0x4f7c50, 's', 1, 'v', None),
    'G7ca1cc': (0x498a70, 'c', 0, 'a', 0x7ca1cc), 'G7cb2e8': (0x4a2a90, 'c', 0, 'a', 0x7cb2e8),
    'G7c2650': (0x48c920, 'c', 0, 'a', 0x7c2650), 'G7c2654': (0x48c930, 'c', 0, 'a', 0x7c2654),
    'S7c2654': (0x48c940, 'c', 1, 'a', 0x7c2654), 'S7c2650': (0x48c950, 'c', 1, 'a', 0x7c2650),
    'G8186ac': (0x527930, 'c', 0, 'a', 0x8186ac), 'S816d48': (0x527940, 'c', 1, 'a', 0x816d48),
    'G816d48': (0x527950, 'c', 0, 'a', 0x816d48), 'G80239c': (0x4e1910, 'c', 0, 'a', 0x80239c),
    'M802ba8': (0x51b3c0, 'c', 1, 'a', 0x802ba8), 'S802bc0': (0x51b3d0, 'c', 0, 'v', 0x802bc0),
    'C871ee0': (0x53c8c0, 'c', 0, 'v', 0x871ee0), 'B7c24d0': (0x486b80, 'c', 0, 'b', 0x7c24d0),
    'Table': (0x4861d0, 'c', 1, 'a', None), 'Inc': (0x4d8c50, 'c', 0, 'a', 0x7cdb98), 'Nibble': (0x4d8c00, 'c', 1, 'a', None),
    'Log2': (0x4d8c20, 'c', 2, 'a', None), 'MeshReady': (0x42a010, 't', 0, 'a', 0x879dec),
    'Kind2': (0x42a050, 't', 0, 'a', None), 'Load': (0x48c960, 'c', 0, 'a', 0x7c2650),
    'C492d80': (0x492e40, 't', 0, 'a', None), 'Select': (0x4a0810, 'c', 2, 'a', 0x7cb290)}
SIZES = {0x4d8c00: 0x20, 0x4d8c20: 0x30, 0x42a010: 0x20, 0x42a050: 0x20, 0x48c960: 0x20, 0x492e40: 0x20, 0x4a0810: 0x30}
B = Bench([(a, a + SIZES.get(a, 0x10)) for (a, *_rest) in CALLS.values()])


def case(r):
    address, conv, n, ret, glob = CALLS[r.choice(OPS)]
    regions = {RAM: r.bytes(0x400)}
    if glob:
        regions[glob & ~0xf] = r.bytes(0x20)
    for extra in (0x7b05a8, 0x7c264c, 0x7cb28c, 0x7cb2a0):
        regions.setdefault(extra & ~0xf, r.bytes(0x20))
    args = [r.choice([0, 1, 0x3ff, 0x400, 0x7fffffff, 0xffffffff, r.getrandbits(32)]) for _ in range(n)]
    if address == 0x4861d0:
        args = [r.randrange(0, 64)]
    elif address == 0x4d8c00:
        args = [r.choice([0, 1, 0xf, 0x10, 0x11, 0xff, 0x100, 0x1230, 0x12340, r.getrandbits(32)])]
    elif address == 0x4d8c20:
        args = [r.choice([0, 1, 2, 3, 4, 7, 8, 0x80, 0xff, 0x1ff, r.getrandbits(32)]), r.choice([0, 1, 0x80, 0xff, r.getrandbits(32)])]
    elif address == 0x4a0810:
        args = [r.choice([0, 1, 2, 3, 4, 0xffffffff, 100]), r.getrandbits(32)]
    if conv == 't':
        args = [RAM + 0x60 + r.choice([0, 0x10])] + args
    return Call(address, args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

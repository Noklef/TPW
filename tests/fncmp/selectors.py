# The tiny getters and selectors of src/game/core/selectors against the originals: any object bytes (with the
# interesting values forced into the fields they test), any global values, a link to a node.
# usage: selectors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Shr2,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {
    'Shr2': (0x41a980, 't', 'a', 6), 'Bit24': (0x56dd50, 't', 'a', 8), 'Word': (0x516c00, 'c', 'b', 0x10),
    'S16': (0x6c6ff0, 't', 'a', 0x10), 'S2': (0x6c7000, 't', 'a', 0x10), 'S45': (0x6b0600, 't', 'a', 0x10),
    'S143': (0x6b0610, 't', 'a', 0x10), 'P7cc': (0x4bae30, 'c', 'a', 0x10), 'Inc30': (0x6b9da0, 't', 'a', 8),
    'IncM': (0x591a60, 't', 'a', 8), 'Mul': (0x4c9ed0, 'c', 'a', 0x10), 'Bits': (0x6b9260, 't', 'a', 0x10),
    'Link': (0x409ce0, 's', 'a', 0x10), 'Inst': (0x633b70, 't', 'a', 0x10), 'Wnd': (0x633b80, 't', 'a', 0x10),
    'D6fd': (0x4f0240, 't', 'a', 0x10), 'D708': (0x67d340, 't', 'a', 0x10)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x5f86a0, 0x5f86a6), (0x5f86c0, 0x5f86c6)])
VALUES = [0, 1, 2, 3, 0x40, 0x42, 0xff, 0xffffffff, 0x80000000, 0x7fffffff, 0x1000000, 0xfeffffff]


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    for off in (0, 4, 8, 0x14, 0x1c, 0x20, 0x28, 0x30, 0x54, 0x5c, 0x70):
        if r.random() < 0.7:
            struct.pack_into('<I', obj, off, r.choice(VALUES))
    regions = {RAM: obj}
    for g in (0x7cf4f4, 0x7cc53c, 0x7852f8, 0x78548c, 0xfa760c, 0xfa7610):
        regions[g] = struct.pack('<I', r.choice(VALUES))
    if name == 'Link':
        nodes = bytearray(r.bytes(0x20))
        regions[RAM + 0x200] = nodes
        regions[RAM + 0x300] = struct.pack('<I', RAM + 0x200)
        return Call(address, [RAM + 0x300], regions, conv=conv, ret=ret)
    # IncM reads and writes 0x4c below its object
    this = RAM + (0x60 if name == 'IncM' else 0)
    return Call(address, [] if conv == 'c' else [this], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

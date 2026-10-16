# The dynamic face queue writers of src/game/render/dynfaces (0x53c8d0, 0x53c9c0) against the originals: counts of faces from none to full (the 0x806 limit too), tags
# including 1 with the flag byte either way, the square's arguments and the polygon's four corners / uvs / colours.
# usage: dynfaces.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Square,Poly)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

CALLS = {'Square': 0x53c8d0, 'Poly': 0x53c9c0}
B = Bench([(0x53c8d0, 0x53c9c0), (0x53c9c0, 0x53cab0), (0x5da3c0, 0x5da3c1)])


def case(r):
    name = r.choice(OPS)
    n = r.choice([0, 1, 2, 5, 0x805, 0x806, 0x807])
    base = 0x820b00 + 0xa0 * min(n, 6)
    regions = {0x820b00: r.bytes(0xa0 * 9), 0x872168: r.bytes(0x40), 0x871ee0: struct.pack('<I', min(n, 6) if n < 0x806 else n), 0x763c98: bytes([r.choice([0, 1])]) + bytes(3),
               0x874fc4: struct.pack('<I', r.choice([0, 0, 1, 5])), RAM: r.bytes(0x100), RAM + 0x100: r.bytes(0x100), RAM + 0x200: r.bytes(0x100)}
    tag = r.choice([0, 1, 1, 2, 0x1f, 0x20, 0xff, r.getrandbits(32)])
    if name == 'Square':
        w = lambda: r.getrandbits(32)  # noqa: E731
        return Call(CALLS[name], [w(), w(), w(), w(), tag, w(), w(), w(), w(), w(), w()], regions, ret='v')
    return Call(CALLS[name], [RAM, tag, RAM + 0x100, RAM + 0x200, r.getrandbits(32)], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

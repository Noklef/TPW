# The dynamic face queue writers and the tag sort of src/game/render/dynfaces (0x53c8d0, 0x53c9c0, 0x53c1f0) against the originals: counts of faces from none to full (the 0x806 limit too), tags
# including 1 with the flag byte either way, the square's arguments and the polygon's four corners / uvs / colours; tag words of few or all 32 tags to sort,
# from none (which the original's unsigned compares send to the partition) to 300 (runs of all sizes).
# usage: dynfaces.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Square,Poly,Sort)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

CALLS = {'Square': 0x53c8d0, 'Poly': 0x53c9c0, 'Sort': 0x53c1f0}
B = Bench([(0x53c8d0, 0x53c9c0), (0x53c9c0, 0x53cab0), (0x5da3c0, 0x5da3c1), (0x53c1f0, 0x53c3e4)])


def case(r):
    name = r.choice(OPS)
    if name == 'Sort':
        n = r.choice([0, 1, 2, 3, 6, 7, 8, 9, 15, 40, 300]) if r.random() < 0.6 else r.randrange(0, 120)
        kinds = r.choice([1, 2, 3, 5, 32])
        words = [r.getrandbits(27) << 5 | (r.randrange(kinds) if kinds < 32 else r.getrandbits(5)) for _ in range(n + 2)]
        if r.random() < 0.15:  # already sorted or reversed
            words[1:n + 1] = sorted(words[1:n + 1], key=lambda w: w & 0x1f, reverse=r.random() < 0.5)
        return Call(CALLS[name], [RAM + 0x1000, n], {RAM + 0x1000: struct.pack('<%dI' % len(words), *words)}, ret='v')
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

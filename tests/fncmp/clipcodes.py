# The clip codes of a run of points (src/game/render/clipcodes: 0x561bc0, and 0x570d60 that passes its ECX and its third argument on) against the original: 1-9 points (0x10 to 0x30 bytes apart) of floats of either
# sign whose magnitudes tie, lead or trail w's (or random patterns), a random 128-byte code table at 0x879970, masks of all bits or random, and the
# caller's ECX (whose high half the result's top bytes keep) random.
# usage: clipcodes.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x561bc0, 0x561c3c), (0x570d60, 0x570d8a), (0x5da3c0, 0x5da3c1)])
OUT, CODES, POINTS = RAM + 0x100, RAM + 0x200, RAM + 0x1000


def coord(r, w):
    if r.random() < 0.1:
        return r.getrandbits(32)
    m = r.choice([abs(w), abs(w) * 0.5, abs(w) * 2, 0.0, 1.0, r.uniform(0, 100)])
    return struct.unpack('<I', struct.pack('<f', m if r.random() < 0.5 else -m))[0]


def case(r):
    count = r.choice([1, 1, 2, 3, 4, 9, r.randrange(1, 10)])
    stride = r.choice([0x10, 0x20, 0x20, 0x30])
    points = bytearray(r.bytes(stride * count + 0x10))
    for k in range(count):
        w = r.choice([1.0, 2.5, 0.0, 100.0, r.uniform(-50, 50)])
        wbits = struct.unpack('<I', struct.pack('<f', w if r.random() < 0.8 else -w))[0]
        struct.pack_into('<4I', points, stride * k, coord(r, w), coord(r, w), coord(r, w), wbits if r.random() < 0.95 else r.getrandbits(32))
    regions = {0x879970: r.bytes(0x80), OUT: r.bytes(8), CODES: r.bytes(0x10), POINTS: points}
    mask = r.choice([0xff, 0xff, 0x3f, r.getrandbits(32)])
    return Call(r.deal([0x561bc0, 0x570d60]), [r.getrandbits(32), r.getrandbits(32), OUT, CODES, POINTS, count, stride, mask], regions, conv='f', ret='a')


if __name__ == '__main__':
    B.main(case)

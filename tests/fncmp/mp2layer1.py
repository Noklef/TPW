# The MP2 decoder's layer I frame decoder (src/game/core/mp2layer1) against the original (0x6ce5a0): a decoder of 1 or 2 channels (now
# and then 0) with a frame of random bits read from a random position (whole bytes for the fixed-point allocations), the synthesis's
# buffers random, float or fixed point (0xfb2704), half rate (0xfb2708) now and then, the output moving on +0x2450 samples a granule (none
# with no advance now and then); the factor tables, windows and the rest as tests/fncmp/mp2layer2.py's frame cases (its tables()), the
# fixed-point transform (0x6d62c0) fncmp's stand-in.
# usage: mp2layer1.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mp2layer2  # noqa: E402  (the tables)
from fncmp import RAM, Bench, Call, _mp2_transform  # noqa: E402

B = Bench([(0x6ce5a0, 0x6d17a4), (0x6c9270, 0x6c957d), (0x6c9580, 0x6ca574), (0x6ca580, 0x6cc735)], hooks={0x6d62c0: _mp2_transform})
DECODER, FRAME, PCM = RAM, RAM + 0x3000, RAM + 0x6000


def case(r):
    channels = r.choice([1, 2, 2]) if r.random() < 0.97 else 0
    fixed = r.random() < 0.4
    decoder = bytearray(r.bytes(0x2460))
    if fixed:
        decoder[4:0x1204] = b''.join(struct.pack('<h', int(r.gauss(0, 2000))) for _ in range(0x900))
    else:
        decoder[4:0x1204] = b''.join(struct.pack('<I', r.f(r.choice([0.3, 1.0]))) for _ in range(0x480))
    out = 0 if r.random() < 0.05 else PCM
    advance = 0 if not out else r.choice([32 * max(channels, 1), 32, 64, r.randrange(0, 80)])
    struct.pack_into('<2I', decoder, 0x2428, FRAME, r.randrange(0, 64))
    struct.pack_into('<i', decoder, 0x244c, channels)
    struct.pack_into('<I', decoder, 0x2450, advance)
    regions = {DECODER: decoder, FRAME: r.bytes(0x2000), PCM: r.bytes(12 * 2 * 80 + 0x100), mp2layer2.GROUPS: mp2layer2.tables(r),
               mp2layer2.FIXED_POINT: struct.pack('<I', r.choice([1, r.getrandbits(32)]) if fixed else 0),
               mp2layer2.HALF: struct.pack('<I', r.choice([0, 0, 0, 1]))}
    return Call(0x6ce5a0, [DECODER, out], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

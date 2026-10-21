# The MP2 decoder's polyphase synthesis (src/game/core/mp2synth) against the originals: the 32-point DCT (0x6c9580) over 32 subband samples of
# a decoder's range, now and then all near the float limits (sums that overflow in a register but not after the next scaling, products below the
# normal range that a store makes denormal) or with NaNs, infinities and random bits; the two output buffers start as random bits.
# usage: mp2synth.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

CALLS = {'dct64': 0x6c9580}
B = Bench([(0x6c9580, 0x6ca574)])
OUT0, OUT1, IN = RAM, RAM + 0x1000, RAM + 0x2000
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def sample(r, kind):
    if kind == 'big':     # near FLT_MAX: a sum of two overflows the float range, kept in a register
        return fb(r.choice([-1, 1]) * r.uniform(0.5, 1.0) * 3.4e38)
    if kind == 'tiny':    # near FLT_MIN: products and differences fall below the normal range
        return fb(r.choice([-1, 1]) * r.uniform(0, 1) * 10.0 ** r.randrange(-40, -35))
    return r.f(r.choice([1.0, 1.0, 32768.0]))


def case(r):
    name = r.deal(OPS)
    kind = r.choice(['plain', 'plain', 'plain', 'big', 'tiny', 'mixed'])
    data = bytearray(0x80)
    for i in range(32):
        k = r.choice(['plain', 'big', 'tiny']) if kind == 'mixed' else kind
        struct.pack_into('<I', data, 4 * i, sample(r, k))
    regions = {OUT0: r.bytes(0x408), OUT1: r.bytes(0x3c8), IN: data}
    return Call(CALLS[name], [OUT0, OUT1, IN], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)

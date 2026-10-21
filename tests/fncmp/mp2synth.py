# The MP2 decoder's polyphase synthesis (src/game/core/mp2synth) against the originals: the 32-point DCT (0x6c9580) over 32 subband samples of
# a decoder's range, now and then all near the float limits (sums that overflow in a register but not after the next scaling, products below the
# normal range that a store makes denormal) or with NaNs, infinities and random bits; the two output buffers start as random bits. The
# synthesis filter (0x6c9270) over a decoder of 0 to 3 channels (mostly 1 or 2) at any ring position, its buffers and the window (0x70bd48)
# random floats of a decoded stream's range (now and then large enough to clip or overflow the fistp), half rate (0xfb2708) now and then, the
# output none now and then. The fixed-point filter (0x6ca580) the same over halfword buffers and windows (0x70c5c8-0x70ce08) of full range
# (sums that wrap and saturate) or small, through its transform (0x6d62c0). The transform alone over 32 halfwords of full range or
# small, its two outputs apart or overlapping (row 8 goes to both).
# usage: mp2synth.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

CALLS = {'dct64': 0x6c9580, 'synth': 0x6c9270, 'fixed': 0x6ca580, 'transform': 0x6d62c0}
B = Bench([(0x6c9580, 0x6ca574), (0x6c9270, 0x6c957d), (0x6ca580, 0x6cc735), (0x6d62c0, 0x6d6c4c)])
OUT0, OUT1, IN = RAM, RAM + 0x1000, RAM + 0x2000
DECODER, PCM = RAM + 0x4000, RAM + 0x3000
WINDOW, HALF = 0x70bd48, 0xfb2708
WINDOW_FIXED = 0x70c5a8                     # (to 0x70ce08: the mirrored window from 0x70c5c8, the first from 0x70c9c8)
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def sample(r, kind):
    if kind == 'big':     # near FLT_MAX: a sum of two overflows the float range, kept in a register
        return fb(r.choice([-1, 1]) * r.uniform(0.5, 1.0) * 3.4e38)
    if kind == 'tiny':    # near FLT_MIN: products and differences fall below the normal range
        return fb(r.choice([-1, 1]) * r.uniform(0, 1) * 10.0 ** r.randrange(-40, -35))
    return r.f(r.choice([1.0, 1.0, 32768.0]))


def floats(r, n, scale):
    return b''.join(struct.pack('<I', r.f(scale)) for _ in range(n))


def case_synth(r):
    channels = r.choice([1, 2, 2, 2]) if r.random() < 0.95 else r.choice([0, 3])
    loud = r.random() < 0.15
    decoder = bytearray(r.bytes(0x2460))
    decoder[4:0x1204] = floats(r, 0x480, 30000.0 if loud else r.choice([0.3, 1.0]))
    decoder[0x243d] = r.getrandbits(8)
    struct.pack_into('<i', decoder, 0x244c, channels)
    window = floats(r, 0x300, r.choice([1.0, 30000.0]) if loud else r.choice([0.5, 1.0]))
    fraction = floats(r, 32 * 3, r.choice([0.1, 1.0, 30000.0]))
    out = 0 if r.random() < 0.05 else PCM
    regions = {DECODER: decoder, PCM: r.bytes(0x200), IN: fraction, WINDOW - 0x100: window, HALF: struct.pack('<I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))}
    return Call(CALLS['synth'], [DECODER, out, IN], regions, conv='t', ret='v')


def halfwords(r, n, big):
    return b''.join(struct.pack('<h', r.randrange(-0x8000, 0x8000) if big else int(r.gauss(0, r.choice([300, 3000])))) for _ in range(n))


def case_fixed(r):
    channels = r.choice([1, 2, 2, 2]) if r.random() < 0.95 else r.choice([0, 3])
    big = r.random() < 0.2
    decoder = bytearray(r.bytes(0x2460))
    decoder[4:0x1204] = halfwords(r, 0x900, big or r.random() < 0.1)
    decoder[0x243d] = r.getrandbits(8)
    struct.pack_into('<i', decoder, 0x244c, channels)
    out = 0 if r.random() < 0.05 else PCM
    regions = {DECODER: decoder, PCM: r.bytes(0x200), IN: r.bytes(0x100), WINDOW_FIXED: halfwords(r, 0x430, big or r.random() < 0.1),
               HALF: struct.pack('<I', r.choice([0, 0, 0, 1, r.getrandbits(32)]))}
    return Call(CALLS['fixed'], [DECODER, out, IN], regions, conv='t', ret='v')


def case_transform(r):
    data = halfwords(r, 32, r.random() < 0.4)
    if r.random() < 0.2:   # out1 inside out0's span or just past it
        return Call(CALLS['transform'], [OUT0, OUT0 + 2 * r.randrange(0, 0x120), IN], {OUT0: r.bytes(0x440), IN: data}, conv='c', ret='v')
    return Call(CALLS['transform'], [OUT0, OUT1, IN], {OUT0: r.bytes(0x202), OUT1: r.bytes(0x1e2), IN: data}, conv='c', ret='v')


def case(r):
    name = r.deal(OPS)
    if name == 'synth':
        return case_synth(r)
    if name == 'fixed':
        return case_fixed(r)
    if name == 'transform':
        return case_transform(r)
    kind = r.choice(['plain', 'plain', 'plain', 'big', 'tiny', 'mixed'])
    data = bytearray(0x80)
    for i in range(32):
        k = r.choice(['plain', 'big', 'tiny']) if kind == 'mixed' else kind
        struct.pack_into('<I', data, 4 * i, sample(r, k))
    regions = {OUT0: r.bytes(0x408), OUT1: r.bytes(0x3c8), IN: data}
    return Call(CALLS[name], [OUT0, OUT1, IN], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)

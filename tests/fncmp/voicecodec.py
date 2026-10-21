# The voice codec (src/game/core/voicecodec) against the originals: its bit readers over a state {pointer, bit buffer, count} (the count mostly 8
# to 32, now and then any; n mostly 0 to 8, now and then up to 31 and wild past the mask table, into the data after it) over random bytes, the
# buffered ones over a reader whose 0x2000-byte buffer ends where the pointer stands now and then (the stream's slot 1 a recorder filling the
# buffer's first bytes); the channel set-up over random channel bytes with floats (now and then special; signalling NaNs made quiet) for the
# scale and factor. A subframe's excitation (coded or not) and the frame headers (from memory, or the refilled buffer) through the game's
# readers (their vtables 0x70adb8 and 0x70ae28) over random bits, the strides positive (0 would loop for ever). The impulse response of
# 12 coefficients mostly within -1..1, and the synthesis filter over blocks of 12 samples (the history and samples random finite floats);
# a channel's frame and a frame's channels (a memory reader over random bits, the channels' tables and past random finite floats).
# usage: voicecodec.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, fill, log_region, vtable  # noqa: E402

B = Bench([(0x6d4ee0, 0x6d4f2a), (0x6d4fa0, 0x6d4fdc), (0x6d4f30, 0x6d4f99), (0x6d4fe0, 0x6d503a), (0x6d5200, 0x6d5254),
           (0x6d5040, 0x6d51ed), (0x6d5260, 0x6d5341), (0x6d5350, 0x6d5443), (0x6d61c0, 0x6d6285),
           (0x6d57e0, 0x6d61ba), (0x6d5450, 0x6d57d2), (0x6d6290, 0x6d62bd)])
STATE, DATA, CHANNEL, STREAM, READER = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x900, RAM + 0x1000
MEMORY, BITS, FRAME, OUT = RAM + 0x3100, RAM + 0x3200, RAM + 0x4000, RAM + 0x3a00   # a memory reader, its data, a decoder's state
MEMORY_VTABLE, BUFFERED_VTABLE = 0x70adb8, 0x70ae28   # (the driver resolves them to the readers' replacements)
CALLS = {'read': 0x6d4ee0, 'skip': 0x6d4fa0, 'bread': 0x6d4f30, 'bskip': 0x6d4fe0, 'init': 0x6d5200, 'excite': 0x6d5040, 'header': 0x6d5260,
         'bheader': 0x6d5350, 'impulse': 0x6d61c0, 'synth': 0x6d57e0, 'decode': 0x6d5450,
         'frame': 0x6d6290}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def bits(r):
    n = r.choice([r.randrange(0, 9)] * 6 + [r.randrange(0, 32)])
    if r.wild and r.random() < 0.05:
        n = r.randrange(0, 0x100)
    return n


def state(r, pointer):
    count = r.choice([r.randrange(8, 33)] * 6 + [r.randrange(0, 8), r.getrandbits(32), r.randrange(-8, 0) & 0xffffffff])
    return struct.pack('<3I', pointer, r.getrandbits(32), count) + r.bytes(4)


def quiet(x):
    """A signalling NaN made quiet: x87emu loads one quiet before the multiply, where the x87 then answers with the other (quiet) NaN."""
    return x | 0x400000 if x & 0x7f800000 == 0x7f800000 and x & 0x7fffff else x


def finite(r, n, scale):
    """n finite floats in -scale..scale (x87emu's NaNs of equal significands are not the x87's: no NaNs in, only its default one made)."""
    return b''.join(struct.pack('<f', r.uniform(-scale, scale)) for _ in range(n))


def case(r):
    name = r.deal(OPS)
    if name == 'init':
        channel = r.bytes(0x680)
        args = [r.getrandbits(32), r.getrandbits(32), CHANNEL, r.getrandbits(32), quiet(r.f(r.choice([1.0, 100.0]))), quiet(r.f(r.choice([1.0, 1.1])))]
        return Call(0x6d5200, args, {CHANNEL: channel}, conv='t', ret='v')
    if name == 'excite':
        regions = {MEMORY: struct.pack('<I', MEMORY_VTABLE), STATE: state(r, BITS + r.randrange(0, 0x40)), BITS: r.bytes(0x800), OUT: r.bytes(0x1c0)}
        stride = r.choice([1, 1, 2, 3, 4, 4, r.randrange(1, 0x70)])
        coded = r.choice([0, 1, 1, r.getrandbits(32)])
        return Call(0x6d5040, [MEMORY, STATE, coded, OUT, stride], regions, conv='t', ret='v')
    if name == 'impulse':
        scale = r.choice([1.0, 1.0, 0.5, 4.0])
        k = finite(r, 12, scale)
        return Call(0x6d61c0, [CHANNEL, OUT], {CHANNEL: k, OUT: r.bytes(0x30)}, conv='c', ret='v')
    if name in ('decode', 'frame'):
        decoder = bytearray(state(r, BITS + r.randrange(0, 0x40)))
        decoder[0xc:0x14] = struct.pack('<2I', r.choice([0, 1, r.getrandbits(32)]), r.choice([0, 1, r.getrandbits(32)]))
        for _ in range(2):
            decoder += struct.pack('<I', r.choice([r.randrange(0, 0x48), r.getrandbits(32)])) + finite(r, 64, r.choice([1.0, 100.0]))
            decoder += finite(r, 12, r.choice([1.0, 0.5])) + finite(r, 0x300, r.choice([1.0, 100.0, 30000.0]))
        regions = {MEMORY: struct.pack('<I', MEMORY_VTABLE), BITS: r.bytes(0x800), FRAME: decoder}
        if name == 'frame':
            return Call(0x6d6290, [MEMORY, FRAME], regions, conv='t', ret='v')
        return Call(0x6d5450, [MEMORY, FRAME, FRAME + r.choice([0x14, 0xd48])], regions, conv='t', ret='v')
    if name == 'synth':
        channel = r.bytes(0x104) + finite(r, 12, r.choice([1.0, 1.0, 0.5])) + finite(r, 0x300, r.choice([1.0, 100.0, 30000.0]))
        first = r.choice([0, 12, 24, 36, r.randrange(0, 0x30)])
        blocks = r.choice([1, 1, 2, (0x1b0 - first) // 12, r.randrange(-2, 1)])
        return Call(0x6d57e0, [FRAME, first, blocks & 0xffffffff], {FRAME: channel}, conv='c', ret='v')
    if name == 'header':
        regions = {MEMORY: struct.pack('<I', MEMORY_VTABLE), BITS: r.bytes(0x40), FRAME: r.bytes(0x1400)}
        return Call(0x6d5260, [MEMORY, BITS, FRAME, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='v')
    if name in ('read', 'skip'):
        regions = {STATE: state(r, DATA + r.randrange(0, 0x80)), DATA: r.bytes(0x100)}
        return Call(CALLS[name], [r.getrandbits(32), STATE, bits(r)], regions, conv='t', ret='a')
    log = log_region()
    reader = r.bytes(0x2010)
    struct.pack_into('<I', reader, 0x2008, STREAM)
    at = r.choice([0x1fff] * 3 + [0x1ffe, r.randrange(0, 0x2000)])
    if name == 'bheader':
        struct.pack_into('<I', reader, 0, BUFFERED_VTABLE)
        fill(log, 1, 0, r.bytes(0x70))
        regions = {READER: reader, STREAM: struct.pack('<I', STREAM + 4) + vtable({1: 2}), LOG: log, FRAME: r.bytes(0x1400)}
        return Call(0x6d5350, [READER, FRAME, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='v')
    fill(log, 1, 0, r.bytes(r.randrange(1, 0x70)))
    regions = {STATE: state(r, READER + 8 + at), READER: reader, STREAM: struct.pack('<I', STREAM + 4) + vtable({1: 2}), LOG: log}
    return Call(CALLS[name], [READER, STATE, bits(r)], regions, conv='t', ret='a' if name == 'bread' else 'v')


if __name__ == '__main__':
    B.main(case)

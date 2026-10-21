# The sound library's stereo ADPCM decoder of the IMA kind (src/game/core/imaadpcm: 0x672cb3) against the original, over the exe's
# own step and index tables (0x77c6c0, 0x77dd00): the count 1-0x40 (at least one: the listing tests after the first), the data random,
# the channels' samples small, at the 16-bit edges or any, their indices mostly whole steps 0-88 (else any byte offset 0-0x1600), the
# state globals 0x77dd20-0x77dd2c random before; the out buffer random.
# Mono (0x672c10, cdecl (start, data, out, count, sample, index)): the start nibble 0-3 mostly (its parity picks the first byte's
# half), now and then any; the count 1-0x80 mostly, else 0 or -1 (one nibble all the same); the sample dword's lower half as Stereo's
# (the upper random), the index as Stereo's.
# usage: imaadpcm.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Stereo,Mono)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'Stereo': 0x672cb3, 'Mono': 0x672c10}
B = Bench([(0x672cb3, 0x672e26), (0x672c10, 0x672cb3)])
DATA, OUT, SAMPLES, INDICES = RAM, RAM + 0x100, RAM + 0x300, RAM + 0x304


def sample(r):
    return r.choice([0, 1, -1, 0x7fff, -0x8000, r.randrange(-0x100, 0x100), r.randrange(-0x8000, 0x8000)]) & 0xffff


def index(r):
    return 64 * r.randrange(89) if r.random() < 0.85 else r.randrange(0x1601)


def case(r):
    if r.deal(OPS) == 'Mono':
        start = r.choice([0, 0, 1, 1, 2, 3, r.getrandbits(32)])
        count = r.choice([1, 2, 3, 0x10, 0x40, 0x80, r.randrange(1, 0x81), 0, -1]) & 0xffffffff
        regions = {DATA: r.bytes(0x48), OUT: r.bytes(0x104),
                   SAMPLES: struct.pack('<2I', sample(r) | r.getrandbits(16) << 16, index(r))}
        return Call(CALLS['Mono'], [start, DATA, OUT, count, SAMPLES, INDICES], regions, ret='v')
    count = r.choice([1, 2, 3, 0x10, 0x40, r.randrange(1, 0x41)])
    regions = {DATA: r.bytes(0x40), OUT: r.bytes(0x104),
               SAMPLES: struct.pack('<2I', sample(r) | sample(r) << 16, index(r) | index(r) << 16), 0x77dd20: r.bytes(0x10)}
    return Call(CALLS['Stereo'], [DATA, OUT, count, SAMPLES, INDICES], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

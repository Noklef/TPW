# The wave format holder's build (src/game/core/waveformat, 0x6d3a80) against the original: the holder built already (its tag +4 set) now
# and then, else from a sound format record (+0x18: its rate word +0x18, bits byte +0x1a, stereo bit 0 of +0x1b; the game's or any) or,
# without one, from its own +0 / +1 / +2.
# usage: waveformat.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x6d3a80, 0x6d3b04)])
HOLDER, RECORD = RAM, RAM + 0x100


def case(r):
    holder = bytearray(r.bytes(0x20))
    struct.pack_into('<H', holder, 4, r.choice([0, 0, 0, 0, 1, r.getrandbits(16)]))
    struct.pack_into('<I', holder, 0x18, r.choice([0, RECORD, RECORD]))
    holder[0] = r.choice([1, 2, r.getrandbits(8)])
    holder[1] = r.choice([8, 16, r.getrandbits(8)])
    struct.pack_into('<H', holder, 2, r.choice([11025, 22050, 44100, r.getrandbits(16)]))
    record = bytearray(r.bytes(0x20))
    struct.pack_into('<H', record, 0x18, r.choice([11025, 22050, 44100, r.getrandbits(16)]))
    record[0x1a] = r.choice([8, 16, r.getrandbits(8)])
    record[0x1b] = r.choice([0, 1, r.getrandbits(8)])
    return Call(0x6d3a80, [HOLDER], {HOLDER: holder, RECORD: record}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

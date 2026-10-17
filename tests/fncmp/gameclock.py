# The game clock's save and load records (src/game/core/gameclock: 0x4030f0-0x4031b0, 0x402e00,
# 0x402e30) against the originals, through a record stream with room for the bytes or not; the double at +0x20 (0x403390).
# usage: gameclock.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, record_stream  # noqa: E402

B = Bench([(0x402e00, 0x402e25), (0x402e30, 0x402e55), (0x4030f0, 0x403116), (0x403120, 0x403146), (0x403180, 0x4031a6), (0x4031b0, 0x4031d6),
           (0x5f5e20, 0x5f5e50), (0x5f5e50, 0x5f5e80), (0x403390, 0x403394)])
CLOCK, OUT, SINK = RAM, RAM + 0x1000, RAM + 0x2000


def case(r):
    if r.random() < 0.15:  # the double at +0x20: 1.0, 0, small, negative, junk
        clock = bytearray(r.bytes(0x40))
        struct.pack_into('<d', clock, 0x20, r.choice([1.0, 0.0, 0.5, -2.25, 1e-310, 3.0e38]))
        if r.random() < 0.3:
            clock[0x20:0x28] = r.bytes(8)
        return Call(0x403390, [CLOCK], {CLOCK: clock}, conv='t', ret='d')
    at = r.choice([0x4030f0, 0x403120, 0x403180, 0x4031b0, 0x402e00, 0x402e30])
    left = r.choice([4, 4, 8, 0x100, r.randrange(8)])
    return Call(at, [CLOCK, OUT], {CLOCK: r.bytes(0x100), OUT: record_stream(OUT, SINK, left), SINK: r.bytes(0x100)},
                conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

# The game-time stamp's serializer (src/game/core/timesig: 0x41a860) against the original: saves and loads through a record stream with room
# for the 4 bytes or not, versions in and out of 0..1; the stamp and the sink compared after the call.
# usage: timesig.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, record_stream  # noqa: E402

B = Bench([(0x41a860, 0x41a955), (0x416e10, 0x416e15), (0x5f5e20, 0x5f5e80), (0x5da3c0, 0x5da3c1)])
STAMP, STREAM, SINK = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    left = r.choice([0x100, 0x100, 0x100, 4, 0, 1, 2, 3, 5])
    regions = {STAMP: r.bytes(0x10), STREAM: record_stream(STREAM, SINK, left), SINK: r.bytes(0x100)}
    save = r.choice([0, 1, 1, r.getrandbits(32)])
    version = r.choice([0, 1, 0, 1, 0, 1, -1, 2, r.getrandbits(32)])
    return Call(0x41a860, [STAMP, STREAM, save, version, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

# The save game's field announcer and field readers (src/game/core/savegame: 0x416e10; 0x424c20, 0x4d37e0, 0x4fae10, 0x510240, 0x5134d0) against the
# originals. The announcer: random sizes, names and load flags; it returns 0 and pops its three arguments. The readers (1, 2, 4, 8 and 12 bytes): a
# record stream with room for the field or not, announced or quiet; 0 when it read the whole field, else 1.
# usage: savegame.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, record_stream  # noqa: E402

B = Bench([(0x416e10, 0x416e15), (0x424c20, 0x424c8e), (0x4d37e0, 0x4d384e), (0x4fae10, 0x4fae7e), (0x510240, 0x5102ae), (0x5134d0, 0x51353e), (0x5f5e20, 0x5f5e50),
           (0x5da3c0, 0x5da3c1)])
READERS = {0x424c20: 1, 0x4d37e0: 2, 0x4fae10: 4, 0x510240: 8, 0x5134d0: 12}
STREAM, SINK, OUT = RAM, RAM + 0x1000, RAM + 0x2000


def announce(r, at):
    return Call(at, [r.getrandbits(32), r.getrandbits(32), r.choice([0, 1])], {}, conv='s', ret='a')


def read(r, at):
    size = READERS[at]
    left = r.choice([size, size, size + 1, 0x100, 0, r.randrange(size)])
    regions = {STREAM: record_stream(STREAM, SINK, left), SINK: r.bytes(0x100), OUT: r.bytes(0x20)}
    return Call(at, [STREAM, OUT, r.getrandbits(32), r.choice([0, 0, 1, r.getrandbits(32)])], regions, ret='a')


def case(r):
    at = r.deal([0x416e10] + list(READERS))
    return (announce if at == 0x416e10 else read)(r, at)


if __name__ == '__main__':
    B.main(case)

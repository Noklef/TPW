# The game clock's save and load records (src/game/core/gameclock: 0x4030f0-0x4031b0) against the
# originals, through a record stream with room for the four bytes or not.
# usage: gameclock.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, record_stream  # noqa: E402

B = Bench([(0x4030f0, 0x403116), (0x403120, 0x403146), (0x403180, 0x4031a6), (0x4031b0, 0x4031d6),
           (0x5f5e20, 0x5f5e50), (0x5f5e50, 0x5f5e80)])
CLOCK, OUT, SINK = RAM, RAM + 0x1000, RAM + 0x2000


def case(r):
    at = r.choice([0x4030f0, 0x403120, 0x403180, 0x4031b0])
    left = r.choice([4, 4, 0x100, r.randrange(4)])
    return Call(at, [CLOCK, OUT], {CLOCK: r.bytes(0x100), OUT: record_stream(OUT, SINK, left), SINK: r.bytes(0x100)},
                conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

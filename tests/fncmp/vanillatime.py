# The VANT module's save and load records (src/game/core/vanillatime: 0x403220, 0x403260) against the originals: the game clock as thunks_static.py's timer cases have it
# (timeGetTime a recorder, fncmp.COUNTER for the performance counter), a record stream with room for the 4 bytes or not (a short read leaves the stream pointer's own
# bytes in the slot it reads into).
# usage: vanillatime.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, record_stream  # noqa: E402
from thunks_static import clock  # noqa: E402

B = Bench([(0x403220, 0x4032b0), (0x5f5e20, 0x5f5e80), (0x5f5e80, 0x5f5f20), (0x5f5f10, 0x5f5f90), (0x5f5f90, 0x5f5fc0)])
TIME, OUT, SINK = RAM, RAM + 0x1000, RAM + 0x2000


def case(r):
    at = r.choice([0x403220, 0x403260])
    left = r.choice([4, 4, 8, 0x100, r.randrange(8)])
    regions = {TIME: r.bytes(0x10), OUT: record_stream(OUT, SINK, left), SINK: r.bytes(0x100)}
    clock(r, regions, True)
    return Call(at, [TIME, OUT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

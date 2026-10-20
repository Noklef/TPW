# The park analyser's constructor (0x4c58c0 ParkAnalyser_Ctor, src/game/world/analyser) against the original, with the callees run: the thing table as
# thing_ctors.py builds it for a thing constructor, the message centre made or not (thing_ctors.centre; ParkAnalyser_InitCells adds the analyser),
# and the world's clock for the six funny-time stamps (funnytime.py's imports and ticks; the clock's start and rate any).
# usage: analyserctor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import funnytime  # noqa: E402
import thing_ctors  # noqa: E402
import wctimage  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench(wctimage.merged([(0x4c58c0, 0x4c5d10), (0x4c5d10, 0x4c5d70), (0x4cdb60, 0x4cdb80), (0x4ce9e0, 0x4cea00), (0x4c7a90, 0x4c7b62),
                           (0x4d91f0, 0x4d9290), (0x50afe0, 0x50b090), (0x50b350, 0x50b370), (0x50ba80, 0x50bac0), (0x50f1f0, 0x50f200),
                           (0x50ffe0, 0x5100f0), (0x516270, 0x516320), (0x5d1750, 0x5d1751), (0x5da3c0, 0x5da3c1)]   # (thing_ctors.py's for the thing)
                          + funnytime.RANGES + thing_ctors.CENTRE_RANGES), hooks=thing_ctors.CENTRE_HOOKS)
A = RAM + 0x100000   # the analyser (0x21c4c bytes)
SIZE = 0x21c4c


def case(r):
    regions = thing_ctors.thing_regions(r)
    thing_ctors.centre(r, regions)
    funnytime.clock_regions(r, regions)
    clock = bytearray(r.bytes(0x40))
    put(clock, 0x1c, 'I', funnytime.u32(r))
    if r.random() < 0.3:
        put(clock, 0, 'II', r.choice([0, 0xffffffff]), r.choice([0, 0x01bf53eb, 0x7fffffff]))
    regions[funnytime.WORLD + 0x2a0] = clock
    regions[A] = r.bytes(SIZE)
    for mine in (A, funnytime.WORLD + 0x2a0):   # (clear of the others)
        assert not any(a < mine + len(regions[mine]) and mine < a + len(v) for a, v in regions.items() if a != mine)
    return Call(0x4c58c0, [A], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

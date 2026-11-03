# The sweeps of the map's cells (src/game/world/mapsweep: MapSweep_Reset 0x4d8330, MapSweep_Place 0x4d83a0) against the originals. Recorders (driver.cpp's kMapSweepEnv): 0x536780 10, 0x5366e0 11, MapCell_PlaceRide 12, the calls past the log's room hashed. Every sweep 0x8000 calls, so few cases.
# usage: mapsweep.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_fold, log_region  # noqa: E402

B = Bench([(0x4d8330, 0x4d8392), (0x4d83a0, 0x4d8402)], hooks={0x536780: _recorder(10, 0), 0x5366e0: _recorder(11, 0), 0x53b1a0: _recorder(12, 0)})


def case(r):
    log = log_region()
    log_fold(log)
    return Call(r.choice([0x4d8330, 0x4d83a0]), [r.randrange(0x1000, 0x10000000) & ~3], {LOG: log}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

# The dome's centre set (src/game/terrain/landscape: 0x585690 Landscape_SetDomeCentre, 0x5856c0 Landscape_ResetDomeCentre) against the original:
# the three dwords at +0x1f64, Landscape_BuildDome a recorder (k 5) whose answer each returns.
# usage: landscapeset.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

BUILD = 5
B = Bench([(0x585690, 0x5856b6), (0x5856c0, 0x5856dd)], hooks={0x585720: _recorder(BUILD, 0)})
SELF = RAM


def case(r):
    log = log_region()
    set_return(log, BUILD, r.getrandbits(32))
    regions = {LOG: log, SELF: r.bytes(0x1f80)}
    if r.randrange(2):
        return Call(0x585690, [SELF, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    return Call(0x5856c0, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

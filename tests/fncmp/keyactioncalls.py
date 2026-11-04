# The key actions that only call on (src/game/core/keyactions: 0x40bf70, 0x40c3a0, 0x40c3f0, 0x40c410, 0x40c800, 0x424c10) against the
# originals, what they call being recorders; each returns 1.
# usage: keyactioncalls.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402

CALLS = {'Pause': (0x40bf70, 0x40bf82), 'Hide': (0x40c3a0, 0x40c3b0), 'OpenNew': (0x40c3f0, 0x40c405), 'OpenLast': (0x40c410, 0x40c425),
         'Post': (0x40c800, 0x40c818), 'Clear': (0x424c10, 0x424c20)}
B = Bench(list(CALLS.values()), hooks={0x409350: _recorder(1, 1), 0x4819e0: _recorder(2, 0), 0x519ef0: _recorder(3, 2), 0x5194d0: _recorder(4, 0),
                                       0x59ab50: _recorder(5, 0)})
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    log = log_region()
    set_return(log, 4, r.getrandbits(32))
    regions = {LOG: log, 0x7cf83c: r.bytes(4), 0x78d96c: r.bytes(4)}
    return Call(CALLS[r.deal(OPS)][0], [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

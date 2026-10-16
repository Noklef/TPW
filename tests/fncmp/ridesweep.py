# The sweeps over the things of src/game/world/ridesweep (0x482040-0x482400) against the originals: a park of things (thingenv.py) whose objects have definitions of every kind,
# definition words and upgrade types, and flag bytes; the callback is the driver's logging stub (the objects it is called with end up in the log).
# usage: ridesweep.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, CDECL_LOG, Call, LOG, log_region  # noqa: E402
from thingenv import park  # noqa: E402

# name: (address, with a kind argument)
CALLS = {'Kind': (0x482040, True), 'Word': (0x482100, False), 'Up': (0x4821b0, False), 'F1': (0x482270, False), 'F0': (0x482340, False), 'F2': (0x482400, False)}
B = Bench([(a, a + 0xd0) for (a, _k) in CALLS.values()] + [(0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360), (0x412e90, 0x412eea), (0x4dd4e0, 0x4dd500),
          (0x4e2710, 0x4e273a), (0x5da3c0, 0x5da3c1)])


def case(r):
    name = r.choice(OPS)
    address, with_kind = CALLS[name]
    regions, handles, addrs = park(r, count=r.choice([0, 1, 2, 3, 4, 6]), defs=True)
    regions[LOG] = log_region()
    args = ([r.choice([0, 1, 3, 3, 5, 0x7fffffff])] if with_kind else []) + [CDECL_LOG]
    return Call(address, args, regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

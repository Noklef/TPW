# Objects_MeanQueuePosition (src/game/world/objectmsg, 0x4cca40) against the original: the park's things (thingenv.py) of kinds 3 mostly and +0x32 any, World_FirstThing / World_NextThing real, Object_QueuePositionOfNone recorder 10 (objectqueue.py has it; one return
# for all: small, or large so that the sum wraps and the quotient rounds).
# usage: objectsmeanqueue.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402
from thingenv import park  # noqa: E402

B = Bench([(0x4cca40, 0x4ccafe), (0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1)], hooks={0x4ddf40: _recorder(10, 0)})


def case(r):
    log = log_region()
    set_return(log, 10, r.choice([0, 1, 2, 5, 7, 100, 0xffffffff, 0x7fffffff, 0x80000000, 0x55555555, r.getrandbits(32), r.randrange(1000)]))
    regions = park(r, kinds=(3, 3, 3, 3, 1, 2, 0xff, 0x103 & 0xff), defs=False)[0]
    regions[LOG] = log
    return Call(0x4cca40, [], regions, ret='d')


if __name__ == '__main__':
    B.main(case)

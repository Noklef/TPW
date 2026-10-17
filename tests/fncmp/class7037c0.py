# The object factory of src/game/core/misc (0x617ff0 Class7037c0_Create) against the original: an out pair of words and a null or live object pointer; the heap hands out the block,
# so a failed allocation is not covered.
# usage: class7037c0.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap  # noqa: E402

B = Bench([(0x617ff0, 0x6180b0), (0x6338b0, 0x6338d0), (0x67acb0, 0x67acb1)])
OUT, SLOT = RAM + 0x2000, RAM + 0x2100


def case(r):
    regions = {HEAP: heap(), OUT: r.bytes(8), SLOT: r.bytes(8)}
    return Call(0x617ff0, [OUT, r.choice([SLOT, SLOT, 0])], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)

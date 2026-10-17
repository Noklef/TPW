# The object factories of src/game/core/misc (0x617ff0 Class7037c0_Create, 0x617970 Class703750_Create) against the originals: an out pair of words and a null or live object pointer; the heap
# hands out the block, so a failed allocation is not covered.
# usage: class_factories.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=C7037c0,C703750)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap  # noqa: E402

CALLS = {'C7037c0': 0x617ff0, 'C703750': 0x617970}
B = Bench([(0x617ff0, 0x6180b0), (0x617970, 0x617a40), (0x6338b0, 0x6338d0), (0x67acb0, 0x67acb1)])
OUT, SLOT = RAM + 0x2000, RAM + 0x2100


def case(r):
    regions = {HEAP: heap(), OUT: r.bytes(8), SLOT: r.bytes(8)}
    return Call(CALLS[r.choice(OPS)], [OUT, r.choice([SLOT, SLOT, 0])], regions, conv='c', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

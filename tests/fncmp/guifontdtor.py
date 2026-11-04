# The GUI font's destructors (src/game/ui/guifont: 0x657d6c, 0x658460) against the originals: the block at +0xc or none, the flags' bit 0; the frees seen in the heap region.
# usage: guifontdtor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, heap, put  # noqa: E402

B = Bench([(0x657d6c, 0x657da0), (0x658460, 0x658490)])


def case(r):
    regions = {HEAP: heap(0x40), RAM: put(bytearray(r.bytes(0x20)), 0xc, 'I', r.deal([0, RAM + 0x10, r.getrandbits(32)]))}
    if r.random() < 0.3:
        return Call(0x657d6c, [RAM], regions, conv='t', ret='v')
    return Call(0x658460, [RAM, r.deal([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

# Lnd_Clear (0x55f630, src/game/terrain/land) against the original: an object of random bytes whose arrays hold blocks (or not) for Array3D_Free to hand back to the heap; the
# result is the object, the heap's record of the frees and the tile sizes.
# usage: land_init.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x55f630, 0x55f720), (0x5d3860, 0x5d38a0), (0x67acb0, 0x67acb1)])


def case(r):
    obj = bytearray(r.bytes(0x400))
    for offset in range(0x14, 0x3b0, 4):
        if r.random() < 0.3:
            put(obj, offset, 'I', 0)
    return Call(0x55f630, [RAM], {RAM: obj, HEAP: heap()}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

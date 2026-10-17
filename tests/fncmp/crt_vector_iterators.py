# The compiler runtime's array constructor and destructor iterators (src/game/core/crt_vector_iterators: 0x67acf0, 0x67afa0) against the originals, calling the driver's stand-in
# (STUBS + 0x40) that stores a vtable pointer at the element over arrays of different element sizes and counts (zero, negative).
# usage: crt_vector_iterators.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Dtor)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, STUBS  # noqa: E402

B = Bench([(0x67acf0, 0x67ad90), (0x67afa0, 0x67b060)])
ARR = RAM + 0x100


def case(r):
    name = r.choice(OPS)
    size = r.choice([4, 8, 0x10, 0x24, 0x30])
    count = r.choice([0, 1, 2, 3, 5, -1, -3])
    regions = {ARR: r.bytes(0x200)}
    if name == 'Ctor':
        return Call(0x67acf0, [ARR, size, count, STUBS + 0x40, r.getrandbits(32)], regions, conv='s', ret='v')
    return Call(0x67afa0, [ARR, size, count, STUBS + 0x40], regions, conv='s', ret='v')


OPS = [k for k in os.environ.get('OPS', 'Ctor,Dtor').split(',') if k]


if __name__ == '__main__':
    B.main(case)

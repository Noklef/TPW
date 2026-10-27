# The compiler runtime's array constructor and destructor iterators (src/game/core/crt: 0x67acf0, 0x67afa0, 0x401000, the one without unwinding, and 0x67d830, the one for a virtual base) against the originals, calling the driver's stand-in
# (STUBS + 0x40) that stores a vtable pointer at the element over arrays of different element sizes and counts (zero, negative); the virtual base's
# a recorder (thiscall, its argument logged). GameCtor / GameDtor hand them a game function's own address instead: IntPair_Ctor (0x50f1f0,
# (0, 0) stored) and Stub_Ret (0x5d1750), which the driver runs through jumps at those addresses (driver.cpp's kRawCalled), as the hooked game would.
# usage: crt_vector_iterators.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Dtor,CtorNoEh,VbaseCtor,GameCtor,GameDtor)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, STUBS, log_region, recorder  # noqa: E402

B = Bench([(0x67acf0, 0x67ad90), (0x67afa0, 0x67b060), (0x401000, 0x40102a), (0x67d830, 0x67d8be), (0x50f1f0, 0x50f200), (0x5d1750, 0x5d1751)])
ARR = RAM + 0x100


def case(r):
    name = r.choice(OPS)
    size = r.choice([4, 8, 0x10, 0x24, 0x30])
    count = r.choice([0, 1, 2, 3, 5, -1, -3])
    regions = {ARR: r.bytes(0x200)}
    if name == 'CtorNoEh':
        return Call(0x401000, [ARR, size, count, STUBS + 0x40], regions, conv='s', ret='v')
    if name == 'VbaseCtor':
        regions[LOG] = log_region()
        return Call(0x67d830, [ARR, size, count, recorder(0, 1), r.getrandbits(32)], regions, conv='s', ret='v')
    if name == 'Ctor':
        return Call(0x67acf0, [ARR, size, count, STUBS + 0x40, r.getrandbits(32)], regions, conv='s', ret='v')
    if name == 'GameCtor':
        return Call(0x67acf0, [ARR, size, count, 0x50f1f0, 0x5d1750], regions, conv='s', ret='v')
    if name == 'GameDtor':
        return Call(0x67afa0, [ARR, size, count, 0x5d1750], regions, conv='s', ret='v')
    return Call(0x67afa0, [ARR, size, count, STUBS + 0x40], regions, conv='s', ret='v')


OPS = [k for k in os.environ.get('OPS', 'Ctor,Dtor,CtorNoEh,VbaseCtor,GameCtor,GameDtor').split(',') if k]


if __name__ == '__main__':
    B.main(case)

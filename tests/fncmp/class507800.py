# The array constructor and destructor of src/game/core/initializers (0x507800 Class507800_Ctor, 0x507840 Class507800_Dtor) against the originals: an object of random bytes (the 0x20
# elements of 0x14 bytes and the fields after them).
# usage: class507800.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Dtor)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x507800, 0x507840), (0x507840, 0x507850), (0x507080, 0x5070c0), (0x67acf0, 0x67ada0), (0x67afa0, 0x67b0d0), (0x67b050, 0x67b0d0), (0x5da3c0, 0x5da3c1)])


def case(r):
    ctor = r.random() < 0.5
    return Call(0x507800 if ctor else 0x507840, [RAM], {RAM: r.bytes(0x2a0)}, conv='t', ret='a' if ctor else 'v')


if __name__ == '__main__':
    B.main(case)

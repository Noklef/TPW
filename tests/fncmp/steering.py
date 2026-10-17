# The steering behaviours' constructors, base destructor and exit thunks (src/game/world/steering: 0x50ce50-0x50d140)
# against the originals: objects over any bytes, the globals over any bytes.
# usage: steering.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x50ce50, 0x50d14a)])
CTORS = [0x50ce50, 0x50ced0, 0x50cf40, 0x50cfb0, 0x50d020, 0x50d090, 0x50d100]
DESTROYS = [0x50ce90, 0x50cf10, 0x50cf80, 0x50cff0, 0x50d060, 0x50d0d0, 0x50d140]
GLOBALS = 0x7cf040  # the seven objects, 0x7cf050-0x7cf370


def case(r):
    op = r.randrange(3)
    if op == 0:
        return Call(r.choice(CTORS), [RAM + r.choice([0x10, 0x11, 0x13])], {RAM: r.bytes(0x40)}, conv='t', ret='a')
    if op == 1:
        return Call(0x50cea0, [RAM + r.choice([0x10, 0x12])], {RAM: r.bytes(0x40)}, conv='t')
    return Call(r.choice(DESTROYS), [], {GLOBALS: r.bytes(0x340)})


if __name__ == '__main__':
    B.main(case)

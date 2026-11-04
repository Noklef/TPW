# The GUI library's classes' destructors (src/game/ui/guiclassdtors) against the originals: an object of random bytes; every call a recorder as driver.cpp's kGuiClassDtorsEnv.
# usage: guiclassdtors.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

WINDOW, CLEAR, RECT, CURSOR, FREE = range(10, 15)
DTORS = [0x65acd0, 0x66195e, 0x66a255, 0x66ae57, 0x66c82e, 0x66caa7, 0x66da21]
DELETES = [0x65bc30, 0x661e00, 0x66a490, 0x66bf90, 0x66c940, 0x66d090, 0x66daf0]
B = Bench([(0x65acd0, 0x65ad33), (0x65bc30, 0x65bc5e), (0x66195e, 0x66197a), (0x661e00, 0x661e2e), (0x66a255, 0x66a271), (0x66a490, 0x66a4be), (0x66ae57, 0x66ae73), (0x66bf90, 0x66bfbe),
           (0x66c82e, 0x66c84a), (0x66c940, 0x66c96e), (0x66caa7, 0x66cac3), (0x66d090, 0x66d0be), (0x66da21, 0x66da3d), (0x66daf0, 0x66db1e)],
          hooks={0x65c025: _recorder(WINDOW, 0), 0x65b6d6: _recorder(CLEAR, 0), 0x659d90: _recorder(RECT, 0), 0x6618a6: _recorder(CURSOR, 0), 0x67acb0: _recorder(FREE, 1, cdecl=True)})


def case(r):
    regions = {LOG: log_region(), RAM: r.bytes(0x200)}
    if r.random() < 0.5:
        return Call(r.choice(DTORS), [RAM], regions, conv='t', ret='v')
    flags = r.deal([0, 1, 2, 3, r.getrandbits(32)])
    return Call(r.choice(DELETES), [RAM, flags], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

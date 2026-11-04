# The window's name (src/game/ui/windowname: 0x65d40c, 0x65d521) against the originals: the copies recorders as driver.cpp's kWindowNameEnv.
# usage: windowname.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region  # noqa: E402

NCPY, CPY = 10, 11
B = Bench([(0x65d40c, 0x65d42e), (0x65d521, 0x65d547)], hooks={0x67b0d0: _recorder(NCPY, 3, cdecl=True), 0x67bf10: _recorder(CPY, 2, cdecl=True)})


def case(r):
    regions = {LOG: log_region(), RAM: r.bytes(0x100)}
    if r.random() < 0.5:
        return Call(0x65d40c, [RAM, r.deal([0, RAM + 0x200])], regions, conv='t', ret='v')
    return Call(0x65d521, [RAM, r.deal([0, RAM + 0x200, RAM + 0x200])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

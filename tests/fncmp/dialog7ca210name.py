# The handler of the dialog [0x7ca210]'s child 0x4f3af (src/game/ui/dialog7ca210: 0x499060) against the original. Recorders as driver.cpp's kDialog7ca210Env.
# usage: dialog7ca210name.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x499060, 0x4990e3)],
          hooks={0x4990f0: _recorder(10, 0, cdecl=True), 0x492d20: _recorder(11, 1, cdecl=True), 0x65c5d5: _recorder(12, 4), 0x65f66e: _recorder(13, 3)})
WINDOW = RAM + 0x1000


def case(r):
    log = log_region()
    set_return(log, 13, r.getrandbits(32))
    message = r.deal([0x10001, 0x10002, 0x10006, 0x10006, 0x10000, 0x10003, 0x10005, 0x10007, 0xc, r.getrandbits(32)])
    a = r.deal([0, 0, 1, 0x100, 0x10000, r.getrandbits(32)])
    return Call(0x499060, [WINDOW, message, a, r.getrandbits(32)], {LOG: log, WINDOW: r.bytes(0x10)}, ret='a')


if __name__ == '__main__':
    B.main(case)

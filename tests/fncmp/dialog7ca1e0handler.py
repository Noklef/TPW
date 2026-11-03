# The handler of the dialog [0x7ca1e0] (src/game/ui/dialog7ca1e0: 0x498ad0) against the original. Recorders as driver.cpp's kDialog7ca1e0Env.
# usage: dialog7ca1e0handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x498ad0, 0x498b50)],
          hooks={0x481a10: _recorder(10, 0, cdecl=True), 0x4a9380: _recorder(11, 0, cdecl=True), 0x486b40: _recorder(12, 1, cdecl=True), 0x4a25f0: _recorder(13, 1, cdecl=True), 0x65f66e: _recorder(14, 3)})


def case(r):
    log = log_region()
    set_return(log, 14, r.getrandbits(32))
    regions = {LOG: log, 0x7ca1e0: r.bytes(4)}
    message = r.choice([0x14, 0x100, 0x100, 0x100, 0x100, 0x13, 0x101, r.getrandbits(32)])
    a = r.choice([0x63, 0x64, 0x62, 0x65, r.getrandbits(32)])
    return Call(0x498ad0, [r.getrandbits(32), message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

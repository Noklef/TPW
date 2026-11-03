# The money display's handler (src/game/ui/moneydisplayhandler: 0x4a0db0) against the original. Recorders as driver.cpp's kMoneyDisplayHandlerEnv.
# usage: moneydisplayhandler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x4a0db0, 0x4a0e30)], hooks={0x4a0ab0: _recorder(10, 1, cdecl=True), 0x52fb90: _recorder(11, 1, cdecl=True), 0x65f66e: _recorder(12, 3)})


def case(r):
    log = log_region()
    set_return(log, 12, r.getrandbits(32))
    regions = {LOG: log, 0x7cb2dc: r.bytes(4) + struct.pack('<I', r.choice([0, 1, 2, 3, 4, 6, 0x100, 0x102, r.getrandbits(32)]))}
    message = r.choice([0x15, 0x15, 0x1e, 0x1e, 0x1e, 0x14, 0x16, 0x1d, 0x1f, r.getrandbits(32)])
    return Call(0x4a0db0, [r.getrandbits(32), message, r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

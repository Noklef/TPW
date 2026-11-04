# The window timers set going again (src/game/ui/windowtimersrestart: 0x66213e, 0x662420) against the originals: a list of 0-5 timers of random bytes, GuiClock_Now a recorder as driver.cpp's kWindowTimersRestartEnv.
# usage: windowtimersrestart.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

B = Bench([(0x66213e, 0x662154), (0x662420, 0x66245c)], hooks={0x65968e: _recorder(10, 0, cdecl=True)})


def case(r):
    log = log_region()
    set_return(log, 10, r.getrandbits(32))
    count = r.deal([0, 1, 2, 3, 5])
    timers = bytearray(r.bytes(0x20 * 5))
    for k in range(count):
        put(timers, 0x20 * k + 0x18, 'I', RAM + 0x20 * (k + 1) if k + 1 < count else 0)
    regions = {LOG: log, RAM: timers, 0xfaa634: struct.pack('<I', RAM if count else 0), 0xfaa638: struct.pack('<I', r.deal([0, 1, 1, r.getrandbits(32)]))}
    if r.random() < 0.2:
        return Call(0x66213e, [RAM], regions, conv='t', ret='v')
    return Call(0x662420, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

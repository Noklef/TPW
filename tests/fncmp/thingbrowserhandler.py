# The thing browser's handler (src/game/ui/thingbrowser: 0x4b3790) against the original: every call a recorder numbered as ThingBrowserHandlerEnv from 10; the step also sets [0x7cc308] from a dword of the case (driver.cpp's browser_step the same), which is what the handler branches on after it.
# usage: thingbrowserhandler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

NEXT = RAM + 0x800
step = _recorder(12, 0, cdecl=True)


def step_hook(e):
    if e.read(NEXT, 4) != b'\xff\xff\xff\xff':   # (-1: left as it is)
        e.write(0x7cc308, e.read(NEXT, 4))
    step(e)


B = Bench([(0x4b3790, 0x4b3b6d), (0x4b3b70, 0x4b3b8c)],
          hooks={0x4b3640: _recorder(10, 0, cdecl=True), 0x4b3580: _recorder(11, 0, cdecl=True), 0x4b3340: step_hook, 0x4b3490: _recorder(13, 0, cdecl=True), 0x65d9dd: _recorder(14, 1), 0x65f538: _recorder(15, 0), 0x65f447: _recorder(16, 3), 0x486b40: _recorder(17, 1, cdecl=True), 0x65f66e: _recorder(18, 3)})


def case(r):
    log = log_region()
    regions = {LOG: log}
    regions[NEXT] = struct.pack('<I', r.choice([0, 0, 1, 0xffffffff, 0xffffffff, r.getrandbits(32)]))
    regions[0x7cc2fc] = r.bytes(4)
    regions[0x7cc2f0] = struct.pack('<I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    regions[0x7cc304] = struct.pack('<I', RAM + 0x100)
    regions[0x7cb2b8] = struct.pack('<I', RAM + 0x200)
    regions[0x7cc308] = struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[0x7cc30c] = struct.pack('<I', r.choice([0, 0, 1, 2, 3, 4, 5, 6, 7]))
    regions[0x790ab0] = r.bytes(4)
    set_return(log, 18, r.getrandbits(32))
    message = r.deal([0x14, 0x1e, 0x1e, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x1001d8, 0x1001d8, 0x13, 0x1f, 0x100, 0x102, 0x1001d7, r.getrandbits(32)])
    a = r.randrange(0x95d, 0x966) if r.random() < 0.9 else r.getrandbits(32)
    if 0x95e <= a <= 0x964 and r.random() < 0.4:   # (the button of the mode already on)
        regions[0x7cc30c] = struct.pack('<I', (1, 2, 5, 4, 3, 0, 6)[a - 0x95e])
    b = r.choice([1, 1, 1, 1, 0, 2, r.getrandbits(32)])
    return Call(0x4b3790, [RAM, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

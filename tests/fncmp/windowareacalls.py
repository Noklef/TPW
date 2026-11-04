# The window's area object asked or told (src/game/ui/windowareacalls: 0x65d58d, 0x65d717) against the originals: the area given or none, its vtable's slot 6 a recorder, WindowArea_GetRect a recorder
# as driver.cpp's kWindowAreaCallsEnv; the local rectangle and its assignment real (the out a rectangle of random bytes).
# usage: windowareacalls.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, recorder  # noqa: E402

GET, SLOT = 10, 11
WINDOW, AREA, VTABLE, OUT = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400
B = Bench([(0x65d58d, 0x65d616), (0x65d717, 0x65d745), (0x659d43, 0x659d90), (0x659d90, 0x659dbc), (0x65aa20, 0x65aa80), (0x65aa80, 0x65aa90), (0x65a9d0, 0x65aa20), (0x66bff7, 0x66c01b)], hooks={0x661810: _recorder(GET, 1)})


def case(r):
    window = bytearray(r.bytes(0x200))
    put(window, 0x98, 'I', r.deal([AREA, 0]))
    table = bytearray(0x40)
    put(table, 0x18, 'I', recorder(SLOT, 0))
    regions = {LOG: log_region(), WINDOW: window, AREA: struct.pack('<I', VTABLE) + r.bytes(0xfc), VTABLE: table, OUT: r.bytes(0x40)}
    if r.random() < 0.3:
        return Call(0x65d717, [WINDOW], regions, conv='t', ret='v')
    return Call(0x65d58d, [WINDOW, OUT], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

# A set's surface put on the display's (src/game/gfx/texset: 0x575fb0 TexSet_BltToBack) against the original: slot 0x14 (k 5) of the surface at [[0x879a24] + 0x14],
# its answer in [0x879cd4], a failure logged (DxResult_Message k 2, EngineDebug_Log k 3) and pushed (EngineErrors_Push k 4).
# usage: texsetblt.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return, vtable  # noqa: E402

MESSAGE, DEBUG, PUSH, BLT = 2, 3, 4, 5
B = Bench([(0x575fb0, 0x576006)], hooks={0x56a770: _recorder(MESSAGE, 1, cdecl=True), 0x564650: _recorder(DEBUG, 1, cdecl=True), 0x56af20: _recorder(PUSH, 2)})
SELF, DISPLAY, SURFACE, VT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300


def case(r):
    log = log_region()
    set_return(log, BLT, r.choice([0, 0, 0, 1, 0x7fffffff, 0x80000000 | r.getrandbits(31)]))
    display = r.bytes(0x14) + struct.pack('<I', SURFACE) + r.bytes(8)
    regions = {LOG: log, SELF: r.bytes(0x10), DISPLAY: display, SURFACE: struct.pack('<I', VT), VT: vtable({}, 32, com={BLT: 6}),
               0x879a24: struct.pack('<I', DISPLAY), 0x879cd4: r.bytes(4)}
    return Call(0x575fb0, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

# The handler of the thing panel [0x7cc218] (src/game/ui/panel7cc218: Panel7cc218_Handler, 0x4af600) against the original. Recorders (their own benches have them; driver.cpp's kPanel7cc218HandlerEnv): WindowAt10_Message4 10, Panel7cc218_RefreshFigures 11, the base handler 12, Window_SetTimer 13,
# Window_StartTimer 14, Game_CancelAdvisor 15, Sign_Swing 16, Dialog7ca150_Open 17, Things_StepGroup 18, Things_StepGroupBack 19, RestartOrClose 20, ThingGoto_Start 21, EnterMoveMode 22, Object_GetDef 23, ShowSettings 24, EnterTileMode 25, OpenLevelBox 26, SetFlag64 27, 0x4e15b0 28; the panel's slots 5 (5) and 0xb (9).
# usage: panel7cc218handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x4af600, 0x4af93b)],
          hooks={0x48cf70: _recorder(10, 0), 0x4ade40: _recorder(11, 0), 0x48d200: _recorder(12, 4, cdecl=True), 0x65ef90: _recorder(13, 2), 0x65f024: _recorder(14, 1), 0x486b40: _recorder(15, 1, cdecl=True), 0x4864f0: _recorder(16, 0, cdecl=True),
                 0x495aa0: _recorder(17, 2, cdecl=True), 0x48caf0: _recorder(18, 0), 0x48cbe0: _recorder(19, 0), 0x48ccf0: _recorder(20, 1), 0x48cd10: _recorder(21, 0), 0x48cfa0: _recorder(22, 0), 0x4dd4e0: _recorder(23, 0), 0x4aec30: _recorder(24, 1),
                 0x4af200: _recorder(25, 1), 0x4ae560: _recorder(26, 0), 0x48ce50: _recorder(27, 1), 0x4e15b0: _recorder(28, 1)})
VT = RAM + 0x100
CONTROLS = (0xfffffffe, 0xffffffff, 0xfffffffd, 0, 0x3e25, 0x3e26) + tuple(range(0x3e27, 0x3e39)) + (0x3e39,)


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 24, 1)   # (ShowSettings's flag a byte: the upper bytes the register's)
    set_return(log, 12, r.getrandbits(32))
    set_return(log, 13, r.choice([0, 0x10000, 1, 0x10001, r.getrandbits(32)]))
    message = r.choice([5, 0x10, 0x14, 0x15, 0x1e] + [0x800] * 4 + [0x100] * 8 + [r.choice([0, 4, 6, 0xf, 0x11, 0x13, 0x16, 0x1d, 0x1f, 0xff, 0x101, 0x7ff, 0x801]), r.getrandbits(32)])
    a = r.getrandbits(32)
    if message == 0x100:
        a = r.choice(CONTROLS)
    elif message == 0x800:
        a = r.choice([0x3e2d, 0x3e2f, 0x3e30, 0x3e2e, 0x3e31, 0x3e2c, a])
    elif message == 0x10:
        a = r.choice([0x80080, 0x80080, 0x80081, a])
    b = r.choice([0, 1, 1, 2, r.getrandbits(32)])
    panel = put(bytearray(r.bytes(0x40)), 0, 'I', VT)
    regions = {LOG: log, 0x7cc218: panel, VT: put(vtable({5: 0}, size=0x10), 0x2c, 'I', recorder(9, 0)), 0x7c2658: r.bytes(4)}
    return Call(0x4af600, [RAM, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

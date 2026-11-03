# The thing panels' handlers (src/game/ui/thingpanelhandlers: Panel7cc258_Handler 0x4b0b30, Panel7cc298_Handler 0x4b2150, Panel7ca178_Handler 0x497640, Panel7cc408_Handler 0x4b7720, Panel7cc3e0_Handler 0x4b67f0) against
# the original. Recorders, as ThingPanelHandlerEnv's: WindowAt10_Message4 10, the ShowStats 11, 24 and 25, the base handler 12, Window_SetTimer 13, Window_StartTimer 14, Game_CancelAdvisor 15, Sign_Swing 16, Dialog7ca150_Open 17,
# Things_StepGroup 18, Things_StepGroupBack 19, RestartOrClose 20, ThingGoto_Start 21, EnterMoveMode 22, Things_ForEachSelected 23, SetFlag64 26, CopyScript 27, ShowVisitor 28, ThingBrowser_Open 29, Dialog7ca13c_Open 30,
# Object_GetDef 31, the game table 2, ObjectControls_GetWord18 3, Object_ScrapRefund 4; the panels' slot 5 and the child's slot 7 vtable recorders. Window_FindChild runs on [0x7cc268]'s and [0x7cc3f4]'s children.
# usage: thingpanelhandlers.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x4b0b30, 0x4b0df1), (0x4b2150, 0x4b23ea), (0x65d7bc, 0x65d840), (0x497640, 0x497842), (0x4b7720, 0x4b7892), (0x4b67f0, 0x4b699c)],
          hooks={0x48cf70: _recorder(10, 0), 0x4b01d0: _recorder(11, 0), 0x4b16c0: _recorder(24, 0), 0x48d200: _recorder(12, 4, cdecl=True), 0x65ef90: _recorder(13, 2), 0x65f024: _recorder(14, 1),
                 0x486b40: _recorder(15, 1, cdecl=True), 0x4864f0: _recorder(16, 0, cdecl=True), 0x495aa0: _recorder(17, 2, cdecl=True), 0x48caf0: _recorder(18, 0), 0x48cbe0: _recorder(19, 0),
                 0x48ccf0: _recorder(20, 1), 0x48cd10: _recorder(21, 0), 0x48cfa0: _recorder(22, 0), 0x481bc0: _recorder(23, 2, cdecl=True), 0x4973a0: _recorder(25, 0), 0x48ce50: _recorder(26, 1),
                 0x486610: _recorder(27, 0, cdecl=True), 0x4b72a0: _recorder(28, 0), 0x4b3f60: _recorder(29, 1, cdecl=True), 0x493530: _recorder(30, 1, cdecl=True), 0x4dd4e0: _recorder(31, 0),
                 0x519510: _recorder(2, 0), 0x4ca8c0: _recorder(3, 1, True), 0x4e2400: _recorder(4, 0)})
WINDOW, KIDS, VT, THING = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x4000
MESSAGES = (5, 0x10, 0x14, 0x15, 0x1e, 0x800, 0x100)
PANELS = {
    0x4b0b30: ((-2, -1, 0xc067, 0xc068, 0xc069, 0xc06a, 0xc080, 0xc081, 0xc082, 0xc06b, 0xc07f, 0xc083, -3, 0, 0xc066), (0xc06b, 0xc085, 0xc086, 0xc06a, 0xc06c, 0xc084, 0xc087)),
    0x4b2150: ((-2, -1, 0xa089, 0xa08a, 0xa08b, 0xa08c, 0xa08d, 0xa0a8, 0xa0a9, 0xa08e, 0xa0a7, 0xa0aa, -3, 0, 0xa088), (0xa08e, 0xa08f, 0xa090, 0xa08d, 0xa091)),
    0x497640: ((-2, -1, 0x15bb3, 0x15bb4, 0x15bb4, 0x15bb5, 0x15bb6, 0x15bb7, 0x15bb8, 0x15bc1, 0x15bc2, 0x15bc3, 0x15bc4, 0x15bc4, 0x15bc5, 0), (1,)),
    0x4b7720: ((-3, -2, -1, 0, 0x371f, 0x3720, 0x3721, 0x3722, 0x3736, 0x3737, 0x3738, 0x3739), (1,)),
    0x4b67f0: ((-2, -1, 0, 0x5208, 0x5209, 0x520a, 0x520b, 0x520c, 0x5212, 0x5213, 0x5214, 0x5215), (1,)),
}


def case(r):
    log = log_region()
    set_return(log, 12, r.getrandbits(32))
    set_return(log, 13, r.choice([0, 1, 0x10000, 0x10001, r.getrandbits(32)]))
    for k in (2, 3, 4, 31):
        set_return(log, k, r.getrandbits(32))
    op = r.deal(list(PANELS))
    message = r.choice(MESSAGES * 3 + (r.choice([0, 4, 6, 0xf, 0x11, 0x13, 0x16, 0x1d, 0x1f, 0xff, 0x101, 0x7ff, 0x801]), r.getrandbits(32)))
    if message == 0x100:
        a = r.choice(PANELS[op][0]) & 0xffffffff
    elif message == 0x800:
        a = r.choice(PANELS[op][1])
    elif message == 0x10:
        a = r.choice([0x80080, 0x80080, 0x80081, r.getrandbits(32)])
    else:
        a = r.getrandbits(32)
    b = r.choice([0, 1, 1, 2, 31, 32, 33, r.getrandbits(32)])
    ids = [0xc07f, r.getrandbits(32) | 0x1000, 0x520e] + ([0x5210] if r.random() < 0.8 else [])
    r.shuffle(ids)
    kids = bytearray(r.bytes(0x400))
    for k, ident in enumerate(ids):
        put(kids, 0x100 * k, 'I', VT)
        put(kids, 0x100 * k + 0x54, 'I', KIDS + 0x100 * (k + 1) if k + 1 < len(ids) else 0)
        put(kids, 0x100 * k + 0x8c, 'I', ident)
    panels = bytearray(r.bytes(0x70))
    put(panels, 0, 'I', VT)
    put(panels, 0x10, 'I', WINDOW)
    put(panels, 0x40, 'I', VT)
    regions = {LOG: log, WINDOW: put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS), KIDS: kids, VT: vtable({5: 0, 7: 1}), 0x7cc258: panels, THING: r.bytes(0x400),
               0x7c2658: struct.pack('<I', THING + 4 * r.randrange(0x80)), 0x7cc3f4: struct.pack('<I', r.choice([WINDOW] * 5 + [0])), 0x7cf83c: r.bytes(4)}
    return Call(op, [RAM + 0x3000, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

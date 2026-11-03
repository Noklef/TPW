# Panel7cc258_Handler and Panel7cc298_Handler (src/game/ui/thingpanelhandlers, 0x4b0b30, 0x4b2150) against the original. Recorders, as ThingPanelHandlerEnv's: WindowAt10_Message4 10, the two ShowStats 11 and 24,
# the base handler 12, Window_SetTimer 13, Window_StartTimer 14, Game_CancelAdvisor 15, Sign_Swing 16, Dialog7ca150_Open 17, Things_StepGroup 18, Things_StepGroupBack 19, RestartOrClose 20, ThingGoto_Start 21,
# EnterMoveMode 22, Things_ForEachSelected 23; the panels' slot 5 and the child's slot 7 vtable recorders. Window_FindChild runs on [0x7cc268]'s children.
# usage: thingpanelhandlers.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x4b0b30, 0x4b0df1), (0x4b2150, 0x4b23ea), (0x65d7bc, 0x65d840)],
          hooks={0x48cf70: _recorder(10, 0), 0x4b01d0: _recorder(11, 0), 0x4b16c0: _recorder(24, 0), 0x48d200: _recorder(12, 4, cdecl=True), 0x65ef90: _recorder(13, 2), 0x65f024: _recorder(14, 1),
                 0x486b40: _recorder(15, 1, cdecl=True), 0x4864f0: _recorder(16, 0, cdecl=True), 0x495aa0: _recorder(17, 2, cdecl=True), 0x48caf0: _recorder(18, 0), 0x48cbe0: _recorder(19, 0),
                 0x48ccf0: _recorder(20, 1), 0x48cd10: _recorder(21, 0), 0x48cfa0: _recorder(22, 0), 0x481bc0: _recorder(23, 2, cdecl=True)})
WINDOW, KIDS, VT = RAM, RAM + 0x1000, RAM + 0x2000
MESSAGES = (5, 0x10, 0x14, 0x15, 0x1e, 0x800, 0x100)
PANELS = {
    0x4b0b30: ((-2, -1, 0xc067, 0xc068, 0xc069, 0xc06a, 0xc080, 0xc081, 0xc082, 0xc06b, 0xc07f, 0xc083, -3, 0, 0xc066), (0xc06b, 0xc085, 0xc086, 0xc06a, 0xc06c, 0xc084, 0xc087)),
    0x4b2150: ((-2, -1, 0xa089, 0xa08a, 0xa08b, 0xa08c, 0xa08d, 0xa0a8, 0xa0a9, 0xa08e, 0xa0a7, 0xa0aa, -3, 0, 0xa088), (0xa08e, 0xa08f, 0xa090, 0xa08d, 0xa091)),
}


def case(r):
    log = log_region()
    set_return(log, 12, r.getrandbits(32))
    set_return(log, 13, r.choice([0, 1, 0x10000, 0x10001, r.getrandbits(32)]))
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
    ids = [0xc07f, r.getrandbits(32) | 0x1000]
    r.shuffle(ids)
    kids = bytearray(r.bytes(0x200))
    for k, ident in enumerate(ids):
        put(kids, 0x100 * k, 'I', VT)
        put(kids, 0x100 * k + 0x54, 'I', KIDS + 0x100 * (k + 1) if k + 1 < len(ids) else 0)
        put(kids, 0x100 * k + 0x8c, 'I', ident)
    panels = bytearray(r.bytes(0x70))
    put(panels, 0, 'I', VT)
    put(panels, 0x10, 'I', WINDOW)
    put(panels, 0x40, 'I', VT)
    regions = {LOG: log, WINDOW: put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS), KIDS: kids, VT: vtable({5: 0, 7: 1}), 0x7cc258: panels, 0x7c2658: r.bytes(4)}
    return Call(op, [RAM + 0x3000, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

# The window handler of the panel at 0x7cc3b0 and its box's answer (src/game/ui/panel7cc3b0handler: Panel7cc3b0_Handler 0x4b5cb0, Panel7cc3b0_DismissAnswer 0x4b6070) against the original. Recorders (their own benches have them; driver.cpp's kPanel7cc3b0HandlerEnv): ThingPanel_Handler_48d200 12, WindowAt10_Message4 13, Game_CancelAdvisor 14,
# Window_SetTimer 15, Window_StartTimer 16, AnalyserRecord_ChecksCell 17, Global7c24d8_CopyScript 18, Panel7cc3b0_ShowStats 19, Byte2_ToIndex 20, Dialog7ca168_Open 21, Things_StepGroup 22, Things_StepGroupBack 23, Panel7cc3b0_SetPatrol 24, Language_WideText 25, 0x47f020 26 (the record's bytes hashed, its byte 7 unset), Staff_Dismiss 27,
# Global7c2650_Set 28, Staff_PickUp 29, ThingBrowser_Open 30; the child's slot 5 and the panel's slot 11 vtable recorders. Window_FindChild runs.
# usage: panel7cc3b0handler.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Handler,Answer)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, set_return, vtable  # noqa: E402

C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4b5cb0, 0x4b5f20), (0x4b6070, 0x4b607f), (0x65d7bc, 0x65d840)],
          hooks={0x48d200: C(12, 4), 0x48cf70: _recorder(13, 0), 0x486b40: C(14, 1), 0x65ef90: _recorder(15, 2), 0x65f024: _recorder(16, 1), 0x505bd0: _recorder(17, 0), 0x486610: C(18, 0), 0x4b58b0: _recorder(19, 0), 0x506490: _recorder(20, 0), 0x496620: C(21, 2), 0x48caf0: _recorder(22, 0), 0x48cbe0: _recorder(23, 0),
                 0x4b5b50: _recorder(24, 1), 0x485b00: C(25, 1), 0x47f020: C(26, 8), 0x505790: _recorder(27, 0), 0x48c950: C(28, 1), 0x505c50: _recorder(29, 0), 0x4b3f60: C(30, 1)})
WINDOW, TOP, KIDS, VT, THING = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x600
MESSAGES = [5, 0x10, 0x14, 0x15, 0x1e, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100]
ACTIONS = [0xffffffff, 0x50a, 0x50b, 0x50c, 0x50c, 0x50d, 0x50e, 0x50f, 0x510, 0x510, 0x511, 0x520, 0x521, 0x522, 0x80080, 0]


def handler_case(r):
    log = log_region()
    regions = {LOG: log, WINDOW: r.bytes(0x100), VT: vtable({5: 1, 11: 0})}
    message = r.choice(MESSAGES + [r.randrange(0x108), r.getrandbits(32)])
    a = r.choice(ACTIONS + [0x80080] * 8 if message != 0x100 else ACTIONS + [r.randrange(0x500, 0x530)])
    b = r.choice([0, 1, 1, 2, r.getrandbits(32)])
    set_return(log, 12, r.getrandbits(32))
    set_return(log, 15, r.choice([0, 0, 1, 0x10000, 0x10001, r.getrandbits(32)]))
    set_return(log, 17, r.choice([0, 1, 2, r.getrandbits(32)]))
    set_return(log, 20, r.getrandbits(32))
    set_return(log, 24, r.choice([0, 1, 0x100, r.getrandbits(32)]))
    set_return(log, 25, r.getrandbits(32))
    blob_bytes(log, 26, 0, 0xff7f)
    top = bytearray(r.bytes(0x100))
    put(top, 0x5c, 'I', KIDS)
    kids = bytearray(r.bytes(0x100))
    put(kids, 0, 'I', VT)
    put(kids, 0x54, 'I', 0)
    put(kids, 0x8c, 'I', 0x50e)
    regions[TOP] = top
    regions[KIDS] = kids
    panel = bytearray(r.bytes(0x20))
    put(panel, 0, 'I', VT)
    put(panel, 0x10, 'I', TOP)
    regions[0x7cc3b0] = panel
    regions[THING] = r.bytes(0x40)
    regions[0x7c2658] = struct.pack('<I', r.choice([THING, THING, THING, THING, 0]))
    regions[0x78d90f] = bytes([r.choice([0, 1, 0xff])])
    return Call(0x4b5cb0, [WINDOW, message, a, b], regions, conv='c', ret='a')


def answer_case(r):
    return Call(0x4b6070, [r.getrandbits(32)], {LOG: log_region()}, conv='c', ret='a')


def case(r):
    return handler_case(r) if r.deal(OPS) == 'Handler' else answer_case(r)


OPS = [k for k in os.environ.get('OPS', 'Handler,Answer').split(',') if k]

if __name__ == '__main__':
    B.main(case)

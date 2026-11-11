# The build dialog's handler (src/game/ui/dialog7cc1f8: Dialog7cc1f8_Handler, 0x4ac270) against the original. Every callee a recorder (driver.cpp's kDialog7cc1f8HandlerCalls) but the tile
# modes' constructors (and the heap); the mode holder always there; the money line's slot 9 a vtable of recorders. ObjectDefs_Find, GlobalSave_HasKey and ObjectDef_IsUnlocked are logged by their argument's
# low word: the original pushes a dword whose high word is stack garbage.
# usage: dialog7cc1f8handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, fill, heap, log_region, put, set_return, strings, vtable  # noqa: E402

# (thiscall 1-8 and 10, stdcall 11-12, cdecl 13-23 and 25-29: driver.cpp's kDialog7cc1f8HandlerCalls; 24 and 30 the modes' GameMode_Set and 0x5234d0)
FIND, ROWS, GET, FIELD, SET_TIMER, START, NTH, CONTROLS, DEFS = 1, 2, 3, 4, 5, 6, 7, 8, 10
HAS_KEY, UNLOCKED = 11, 12
SEND, FORMAT, DESTROY, CANCEL, DRESS, STATS, CLOCK, PREVIEW, SWING, TIP, OPEN = range(13, 24)
SOUND, NEXT, POST, FIGURES, KEY = range(25, 30)
SET_MODE = 31   # (MouseMode_Set by address, stdcall; the holder is always there, so the original's inline holder code never runs)


def _low(k, n, **kw):
    rec = _recorder(k, n, **kw)

    def hook(e):
        sp = e.r['ESP'] + (4 if e.tail else 0)
        e.write(sp, struct.pack('<I', e.u32(sp) & 0xffff))
        return rec(e)
    return hook


B = Bench([(0x4ac270, 0x4aca53), (0x46c580, 0x46c5c6)],
          hooks={0x65d7d0: _recorder(FIND, 1), 0x663324: _recorder(ROWS, 0), 0x5195d0: _recorder(GET, 0), 0x6ad810: _recorder(FIELD, 0), 0x65ef90: _recorder(SET_TIMER, 2),
                 0x65f024: _recorder(START, 1), 0x664c71: _recorder(NTH, 2), 0x4d3d10: _recorder(CONTROLS, 2), 0x412e90: _low(DEFS, 1), 0x4d4b70: _low(HAS_KEY, 1, com=True),
                 0x4d4b90: _low(UNLOCKED, 1, com=True), 0x658d9f: _recorder(SEND, 4, cdecl=True), 0x6acd60: _recorder(FORMAT, 5, cdecl=True), 0x486510: _recorder(DESTROY, 0, cdecl=True),
                 0x486b40: _recorder(CANCEL, 1, cdecl=True), 0x4ab6b0: _recorder(DRESS, 0, cdecl=True), 0x4abfb0: _recorder(STATS, 1, cdecl=True), 0x65968e: _recorder(CLOCK, 0, cdecl=True),
                 0x4ab1b0: _recorder(PREVIEW, 1, cdecl=True), 0x4864f0: _recorder(SWING, 0, cdecl=True), 0x4abc80: _recorder(TIP, 1, cdecl=True), 0x49bdd0: _recorder(OPEN, 1, cdecl=True),
                 0x486ae0: _recorder(SOUND, 0, cdecl=True), 0x4824d0: _recorder(NEXT, 2, cdecl=True), 0x486b00: _recorder(POST, 1, cdecl=True), 0x4abb00: _recorder(FIGURES, 1, cdecl=True),
                 0x488ba0: _recorder(KEY, 4, cdecl=True), 0x46c350: _recorder(SET_MODE, 1, com=True)})
CHILD, CHILD_VT, CONTROL, DEF, WINDOW = RAM + 0x4300, RAM + 0x4400, RAM + 0x4500, RAM + 0x4600, RAM
OPS = ['Close', 'Timer', 'Destroy', 'Dress', 'Tick', 'Tick', 'Command', 'Pick', 'Pick', 'Pick', 'Pick', 'Over', 'Over', 'Sort', 'Any']


def case(r):
    log = log_region()
    op = r.deal(OPS)
    child = r.deal([CHILD, CHILD, CHILD, 0])
    item = r.deal([5, 5, 5, 5, -1, -2, 0, -3, 0x10005])
    balance = r.deal([-5, 0, 100, 1000, -1000000])
    since = r.getrandbits(32)
    set_return(log, FIND, child)
    set_return(log, GET, r.getrandbits(32))
    set_return(log, FIELD, balance & 0xffffffff)
    set_return(log, SET_TIMER, r.deal([0, 1, 0x10000]))
    set_return(log, CONTROLS, CONTROL)
    set_return(log, DEFS, DEF)
    set_return(log, HAS_KEY, r.deal([0, 1, 0x100]))
    set_return(log, UNLOCKED, r.deal([0, 1, 0x100]))
    set_return(log, CLOCK, (since + r.deal([0, 0x1f4, 0x1f5, 0x1000, 0xfffffff0])) & 0xffffffff)
    set_return(log, NEXT, r.deal([0, 0, 1]))
    set_return(log, KEY, r.getrandbits(32))
    strings(log, NTH, 0, out=2)
    fill(log, NTH, 1, struct.pack('<i', item))
    blob(log, FORMAT, 4, 0x1f)
    blob(log, FIGURES, 0, 1)
    message, a, b = {'Close': (5, 0, 0), 'Timer': (0x10, r.deal([0x80080, 0x80080, 1]), 0), 'Destroy': (0x14, 0, 0), 'Dress': (0x15, 0, 0), 'Tick': (0x1e, 0, 0),
                     'Command': (0x100, r.deal([0xfffffffe, 0x1ff, 5]), 0), 'Pick': (0x400, r.getrandbits(16), r.deal([0xffffffff, 0, 3, 3])),
                     'Over': (0x401, r.getrandbits(16), r.deal([0xffffffff, 0, 3, 3])), 'Sort': (0x406, 0, r.getrandbits(32)),
                     'Any': (r.deal([0x11, 0x101, 0x402, 0x1f, 3]), r.getrandbits(32), r.getrandbits(32))}[op]
    bytes_ = lambda n: bytearray(r.bytes(n))  # noqa: E731
    dialog = bytes_(0x20)                                  # 0x7cc1e0..0x7cc1fc
    put(dialog, 4, 'H', r.deal([0, 0, 5, item & 0xffff]))  # 0x7cc1e4
    put(dialog, 8, 'H', r.deal([0, 5]))                    # 0x7cc1e8
    put(dialog, 0x10, 'I', since)                          # 0x7cc1f0
    control = bytes_(0x10)
    put(control, 4, 'i', r.deal([balance, balance + 1, balance - 1, 0]))
    definition = bytes_(0xd0)
    put(definition, 0x4c, 'I', r.deal([0, 0, 1]))
    put(definition, 0xc4, 'I', r.deal([0, 0, 5]))
    regions = {LOG: log, HEAP: heap(0x1000, r.deal([0, 0, 0, 0, 1, 2])), 0x7b05e8: struct.pack('<I', RAM + 0x4200), CHILD_VT: vtable({9: 1}), CHILD: put(bytes_(0x10), 0, 'I', CHILD_VT), CONTROL: control, DEF: definition, 0x7cc1e0: dialog, 0x755140: bytes_(4), 0x78ba94: bytes_(4),
               0x7cf83c: bytes_(4)}
    return Call(0x4ac270, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

# The online view's windows' handler (src/game/ui/window7cc8c0handler: Window7cc8c0_Handler, 0x4bd4a0) against the original. Every callee a recorder (driver.cpp's kWindow7cc8c0HandlerEnv) but
# Window_FindChild, Window_SetCursor (the mouse modes' constructors call it too), Fn_65d57c, Camera_PackCellFractions, GuiRect, Global7b05d8_CallSlot14 and the mouse modes (focus.py's MODES). 0x4ea370 / 0x4ea410's arguments are logged by their low words: the original
# pushes both with uninitialised stack in the high words, which the callees do not read.
# usage: window7cc8c0handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, blob_when, fill, heap, log_region, put, set_return, strings, vtable  # noqa: E402
from focus import MODES  # noqa: E402

# (recorder slots run 0-31 in the driver's stubs, and a cdecl recorder may share a slot number with a thiscall one, their log entries differing: a shared slot's set_return reaches both)
GET_RECT, CAPTURE, RELEASE, CHAT_CTOR, SAY, MOVE, MOVE3, DEF_WORD, DEF, FOLLOW, CURSOR, FIRE_DOWN, FIRE_UP, FOCUS, KILL, FIND_KEY, DEFAULT = range(12, 29)
LIGHT, NAME, MODIFIERS, PLACE, BUTTON, ESCAPE, LEAVE, UNLIGHT, SEND, POST, TEXT, CAMERA, OBJECT_AT, FORMAT, CHAT = 12, 13, 14, 15, 16, 17, 18, 19, 20, 25, 27, 28, 29, 30, 31


def _low_words(k):
    rec = _recorder(k, 2)

    def hook(e):
        sp = e.r['ESP'] + (4 if e.tail else 0)
        for i in range(2):
            e.write(sp + 4 * i, struct.pack('<I', e.u32(sp + 4 * i) & 0xffff))
        return rec(e)
    return hook


B = Bench(sorted(set(MODES + [(0x4bd4a0, 0x4be1d0), (0x46c330, 0x46c350), (0x4bd020, 0x4bd0a0), (0x65d57c, 0x65d59d), (0x65d7bc, 0x65d832), (0x659d00, 0x659d53), (0x659d90, 0x659dbc),
                              (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x65a43e, 0x65a4a3), (0x66bff7, 0x66c01b)])),
          hooks={0x65d42e: _recorder(GET_RECT, 2), 0x65e549: _recorder(CAPTURE, 0), 0x65e56e: _recorder(RELEASE, 0), 0x4e5260: _recorder(CHAT_CTOR, 0), 0x4ea3f0: _recorder(SAY, 0),
                 0x4ea370: _low_words(MOVE), 0x4ea410: _low_words(MOVE3), 0x4e2710: _recorder(DEF_WORD, 1), 0x4dd4e0: _recorder(DEF, 0), 0x4e15b0: _recorder(FOLLOW, 1),
                 0x40c900: _recorder(FIRE_DOWN, 2), 0x40c990: _recorder(FIRE_UP, 2), 0x65e59b: _recorder(FOCUS, 1), 0x65e5fa: _recorder(KILL, 1),
                 0x40c870: _recorder(FIND_KEY, 2), 0x65f66e: _recorder(DEFAULT, 3), 0x492530: _recorder(PLACE, 2, cdecl=True), 0x658d9f: _recorder(SEND, 4, cdecl=True),
                 0x658ccc: _recorder(POST, 4, cdecl=True), 0x42a760: _recorder(CAMERA, 3, cdecl=True), 0x46cf10: _recorder(CHAT, 0, cdecl=True), 0x527e80: _recorder(OBJECT_AT, 3, cdecl=True),
                 0x4924b0: _recorder(TEXT, 1, cdecl=True), 0x6acd60: _recorder(FORMAT, 5, cdecl=True), 0x486280: _recorder(NAME, 1, cdecl=True), 0x469180: _recorder(LIGHT, 0, cdecl=True),
                 0x469470: _recorder(UNLIGHT, 0, cdecl=True), 0x46b420: _recorder(MODIFIERS, 0, cdecl=True), 0x46c210: _recorder(BUTTON, 5, cdecl=True), 0x4a9380: _recorder(ESCAPE, 0, cdecl=True),
                 0x48c830: _recorder(LEAVE, 1, cdecl=True)})
MODE, MODE_VT, HOLDER, OBJECT, DEF_AT, WORD = RAM + 0x4000, RAM + 0x4100, RAM + 0x4200, RAM + 0x4300, RAM + 0x4400, RAM + 0x4900
VIEW, SIDE, OTHER = RAM, RAM + 0x400, RAM + 0x800
OPS = ['Rect', 'Rect0', 'Destroy', 'Timer', 'Timer', 'Timer', 'Timer', 'TimerSide', 'TimerSide', 'TimerSide', 'TimerSide', 'Enter', 'Leave', 'Up', 'Down', 'Down0', 'Down0', 'Down0', 'Move', 'KeyUp',
       'Tab', 'Tab', 'Swap', 'Swap', 'Key', 'Key', 'Key', 'Key', 'Any']


def tree(r, base):
    """A window and its children 0xc3123, 0xc312b, 0xc312c."""
    regions = {}
    for i, ident in enumerate((1, 0xc3123, 0xc312b, 0xc312c)):
        node = bytearray(r.bytes(0x100))
        put(node, 0x54, 'I', 0)
        put(node, 0x5c, 'I', base + 0x100 * (i + 1) if i < 3 else 0)
        put(node, 0x8c, 'I', ident)
        regions[base + 0x100 * i] = node
    return regions


def case(r):
    log = log_region()
    op = r.deal(OPS)
    view, side = r.deal([VIEW, VIEW, VIEW, 0]), r.deal([SIDE, SIDE, SIDE, 0])
    window = {'Rect': VIEW, 'TimerSide': SIDE}.get(op, r.deal([VIEW, SIDE, OTHER]))
    if window == VIEW and op in ('Destroy', 'Rect', 'Rect0'):
        view = r.choice([VIEW, VIEW, 0])
    if op == 'TimerSide':
        side = SIDE
    strings(log, GET_RECT, 0, out=1)
    strings(log, DEF_WORD, 0, out=1)
    strings(log, OBJECT_AT, 0, out=1)
    fill(log, OBJECT_AT, 0, struct.pack('<H', r.deal([0, 1, 1, 1, 2, 3])))
    blob(log, FORMAT, 4, 0xf, only_when=True)
    blob_when(log, FORMAT, 3, 0x244, 0xf)
    set_return(log, DEF_WORD, WORD)
    set_return(log, DEF, DEF_AT)
    set_return(log, FOCUS, r.deal([0, 0x10000, 1, 0x100]))
    set_return(log, CHAT_CTOR, r.getrandbits(32))
    set_return(log, CHAT, r.getrandbits(32))
    set_return(log, NAME, r.getrandbits(32))
    set_return(log, DEFAULT, r.getrandbits(32))
    set_return(log, MODIFIERS, r.getrandbits(32))
    set_return(log, 9, r.deal([0, 1, 9, 9]))

    message, a, b = {'Rect': (0x11, r.deal([1, 1, 0]), 0), 'Rect0': (0x11, r.deal([0, 2, 1]), 0), 'Destroy': (0x14, 0, 0), 'Timer': (0x1e, 0, 0), 'TimerSide': (0x1e, 0, 0), 'Enter': (0x10001, 0, 0),
                     'Leave': (0x10002, 0, 0), 'Up': (0x10004, r.deal([0, 1, 0x20]), r.getrandbits(32)), 'Down': (0x10005, r.deal([1, 0x20, 5]), r.getrandbits(32)), 'Down0': (0x10005, 0, r.getrandbits(32)),
                     'Move': (0x10007, r.deal([0, 0, 0, 1]), r.getrandbits(32)), 'KeyUp': (0x1000a, r.getrandbits(32), r.getrandbits(32)),
                     'Tab': (0x1000b, r.deal([9, 13, 0x2f]), r.deal([0, 0, 0x10000, 0xffff0000]) | r.getrandbits(16)), 'Swap': (0x1000b, 0x73, r.deal([0, 0, 0x10000]) | r.getrandbits(16)),
                     'Key': (0x1000b, r.deal([8, 10, 0x72, 0x74, 0x41, 0x80000041]), r.getrandbits(32)), 'Any': (r.deal([0x12, 0x15, 0x1f, 0x10000, 0x10003, 0x10006, 0x10008, 0x10009, 0x1000c]), r.getrandbits(32), r.getrandbits(32))}[op]
    set_return(log, FIND_KEY, r.deal([0, 0, 0xf, 0x10, 0x10000, 0x1000f, 1, 0xe, 0x11]))

    bytes_ = lambda n: bytearray(r.bytes(n))  # noqa: E731
    calls = bytes_(0x40)
    put(calls, 0, 'I', r.deal([0, 0x400, 0x400, 0x16]))   # (0x7cc8ac, the last flags)
    calls[8] = r.deal([0, 1])                             # 0x7cc8b4
    put(calls, 0x10, 'I', view)
    put(calls, 0x14, 'I', side)
    put(calls, 0x18, 'I', OTHER)
    put(calls, 0x1c, 'I', OTHER + 0x100)
    calls[0x28] = r.deal([0, 1, 1])                       # 0x7cc8d4
    put(calls, 0x2c, 'I', r.deal([0, 0, 1, 1, 2]))        # 0x7cc8d8, the timer
    calls[0x30] = r.deal([0, 1, 1])                       # 0x7cc8dc
    calls[0x34] = r.deal([0, 0, 1])                       # 0x7cc8e0
    put(calls, 0x38, 'I', r.deal([0, 1, 2, 2, 3, 4]))     # 0x7cc8e4
    put(calls, 0x3c, 'I', r.deal([OBJECT, OBJECT, OBJECT, 0]))
    cell = r.deal([0, 3, 40, 200])                       # (a position whose packed words 0x7cc690 / 0x7cc694 can be the same)
    words = bytes_(0x14)
    put(words, 0, 'H', cell << 8 if r.deal([0, 1, 1]) else r.getrandbits(16))
    put(words, 4, 'H', cell << 8 if r.deal([0, 1, 1]) else r.getrandbits(16))
    words[0x10] = r.deal([0, 0, 1])                       # 0x7cc6a0
    place = struct.pack('<ffff', cell * 10.0, 0.0, cell * 10.0, 0.0)
    mouse = bytes_(0x20)
    put(mouse, 0, 'H', r.deal([0, 1, 0x81, 0x4000]))     # 0x7b05cc
    down = op in ('Move',)
    put(mouse, 0xc, 'I', MODE if down else r.deal([MODE, MODE, MODE, 0]))     # 0x7b05d8
    put(mouse, 0x1c, 'I', HOLDER if down else r.deal([0, HOLDER, HOLDER]))    # 0x7b05e8
    thing = bytes_(0x40)
    thing[2] = r.deal([3, 3, 3, 2])
    thing[0x32] = r.deal([0x10, 0, 0xff, 0xef])
    table = bytearray(0x50)
    for handle in range(1, 4):
        put(table, 20 * handle, 'I', OBJECT)
    definition = bytearray(0x4b0)
    put(definition, 0x4ac, 'I', r.deal([0, 3, 3, 1]))
    scales = bytes_(8)
    put(scales, 0, 'h', r.deal([640, 800, 1024, -640, r.getrandbits(15)]))
    put(scales, 4, 'h', r.deal([480, 600, 768, -480, r.getrandbits(15)]))
    regions = {LOG: log, HEAP: heap(0x1000 + 0xf0c, r.deal([0, 0, 0, 0, 1, 2])), MODE_VT: vtable({0: 1, 5: 2}), MODE: put(bytearray(r.bytes(0x10)), 0, 'I', MODE_VT),
               HOLDER: b'\x00' * 4, OBJECT: thing, DEF_AT: definition, WORD: struct.pack('<H', r.deal([0, 0, 0, 1])), 0x7cc8ac: calls, 0x7cc690: words, 0x7cfb90: table, 0xfaa5c0: scales,
               0x7cddec: struct.pack('<I', r.deal([0, RAM + 0x4a00])), 0x790ab0: struct.pack('<I', r.deal([0, 0x400, 0x400, 2, 4, 0x10, 1])), 0x790aa0: place, 0x7908f0: place,
               0x7b05cc: mouse, 0x787c14: bytes_(4), 0x78721c: bytes_(4), 0x78ba98: bytes_(4), 0xfb3518: bytes([r.choice([1, 3, 0xff])]), 0xfb3b7c: struct.pack('<I', 1)}
    for base in (VIEW, SIDE, OTHER):
        regions.update(tree(r, base))
    return Call(0x4bd4a0, [window, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

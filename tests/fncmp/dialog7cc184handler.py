# The dialog [0x7cc184]'s handlers (src/game/ui/dialog7cc184handler: 0x4a7d70, 0x4a7c50) against the originals. Every call a recorder (driver.cpp's kDialog7cc184HandlerEnv; 0x67c430's answers from a table of tokens, 0x67c2f0 real, emulated and the port's loop); children small windows with slot 9 a recorder. The heap frees the panel. Addresses good and bad on the tokens' '@' and '.'.
# usage: dialog7cc184handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, blob_bytes, blob, heap, log_region, put, set_return, strings, vtable  # noqa: E402

SEND, FREE_PARTS, AT_EXIT, PAIR, CANCEL, FOCUS, ENABLE, RESET, OPEN, FIND, FORMAT, SHOW, TOKEN, TEXT, BOX, CARD, KEYS, DEFAULT, CAPTURE, RELEASE = range(10, 30)
HOOKS = {0x658d9f: _recorder(SEND, 4, cdecl=True), 0x5d46d0: _recorder(FREE_PARTS, 0), 0x67a7d0: _recorder(AT_EXIT, 1, cdecl=True), 0x4bcef0: _recorder(PAIR, 0, cdecl=True), 0x486b40: _recorder(CANCEL, 1, cdecl=True), 0x4862a0: _recorder(FOCUS, 0, cdecl=True), 0x486b60: _recorder(ENABLE, 0, cdecl=True), 0x486b70: _recorder(RESET, 0, cdecl=True),
         0x4a8140: _recorder(OPEN, 0, cdecl=True), 0x65d7d0: _recorder(FIND, 1), 0x6acd60: _recorder(FORMAT, 4, cdecl=True), 0x4a84c0: _recorder(SHOW, 1, cdecl=True), 0x67c430: _recorder(TOKEN, 2, cdecl=True), 0x485b00: _recorder(TEXT, 1, cdecl=True), 0x47f020: _recorder(BOX, 8, cdecl=True), 0x4a8600: _recorder(CARD, 0, cdecl=True),
         0x488ba0: _recorder(KEYS, 4, cdecl=True), 0x65f66e: _recorder(DEFAULT, 3), 0x65e549: _recorder(CAPTURE, 0), 0x65e56e: _recorder(RELEASE, 0)}
B = Bench([(0x4a7c50, 0x4a7d68), (0x4a7d70, 0x4a8140), (0x550ca0, 0x550d90), (0x5d1750, 0x5d1751), (0x5da3c0, 0x5da3c1), (0x67a400, 0x6902c0)], hooks=HOOKS)
WINDOW, CHILD, VTABLE, DIALOG, PANEL, TOKENS, TURNS = RAM + 0x1000, RAM + 0x1100, RAM + 0x1200, RAM + 0x1300, RAM + 0x1400, RAM + 0x2000, RAM + 0x3000
SIZES = [-1, 0, 1, 0x100, 0x7fff, -0x8000, 640, 480]


def wide(s):
    return b''.join(bytes([c, 0]) for c in s) + b'\0\0'


def tokens(r, regions, log):
    """the addresses' tokens, some bad; the table of the recorder's answers"""
    good = [b'a@b.cd', b'tim@some.where', b'x@yy.z', b'a@b.c.d']
    bad = [b'', b'@b.cd', b'nothing', b'a@.b', b'a@b', b'a.b@c', b'a@b.', b'.@.']
    data, table = bytearray(0x200), bytearray(0xc0)
    n = r.deal([0, 1, 1, 2, 3])
    at = 0
    for i in range(n):
        text = wide(r.choice(bad if r.random() < 0.2 else good))
        data[at:at + len(text)] = text
        put(table, 4 * i, 'I', TOKENS + at)
        at += len(text) + 2
    regions[TOKENS] = data
    regions[TURNS] = table
    answers(log, TOKEN, TURNS)


def picture(r, regions):
    regions[0x7cc1a4] = struct.pack('<I', r.deal([0, 1, 1, 1, 5]))
    regions[0x7cc198] = struct.pack('<i', r.deal([0, 1, 5, 100, 0x10000, -1]))
    regions[0x7cc19c] = struct.pack('<i', r.deal([0, 1, 5, 100, 0x10000, -1]))
    regions[0x7cc1a8] = struct.pack('<i', r.deal([0, 3, 100, -5, 300, r.randrange(-0x8000, 0x8000)]))
    regions[0x7cc1ac] = struct.pack('<i', r.deal([0, 3, 100, 300, 0xffff, r.randrange(0x10000)]))
    regions[0xfaa5c0] = struct.pack('<h', r.deal(SIZES))
    regions[0xfaa5c4] = struct.pack('<h', r.deal(SIZES))


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(), WINDOW: r.bytes(0x10), DIALOG: r.bytes(0x10), PANEL: r.bytes(0x10), VTABLE: vtable({9: 1})}
    regions[CHILD] = struct.pack('<I', VTABLE)
    set_return(log, DEFAULT, r.getrandbits(32))
    if r.random() < 0.3:
        picture(r, regions)
        message = r.deal([0x10003, 0x10003, 0x10003, 0x10004, 0x10005, 0x10002, 0x10006, 5])
        return Call(0x4a7c50, [WINDOW, message, r.deal([r.getrandbits(32), 0, 0x10001, 0x640064, 0xfffb0003, 0x50005]), r.deal([r.getrandbits(32), 0x10001, 0x640064, 0xfffb0003])], regions, ret='a')
    regions[0x7cc184] = struct.pack('<I', r.deal([DIALOG, DIALOG, DIALOG, 0]))
    regions[0x7cc18c] = struct.pack('<I', r.deal([HEAP + 0x10, HEAP + 0x10, 0]))
    regions[0x7cc1a0] = r.bytes(4)
    regions[0xfb34bf] = bytes([r.deal([1, 1, 1, 0xff, 0, 0xfe])])
    regions[0xfb3b7c] = struct.pack('<I', r.deal([0, 1, 1, 2]))
    regions[0x7a1a8c] = struct.pack('<I', r.deal([0, 0x800000, 0x1000000, 0x1000000, 0x2000000, r.getrandbits(32)]))
    regions[0x7cbf40] = bytes([r.deal([0, 1, 0, 0xff])])
    regions[0x7cbf3c] = bytes([r.deal([0, 1, 0, 0xff])])
    regions[0x78ba94] = r.bytes(4)
    regions[0x7cb4d0] = wide(r.deal([b'', b'a@b.cd', b'x', b'a@b.cd;x@y.zz', b'nothing at all']))
    tokens(r, regions, log)
    set_return(log, FIND, CHILD)
    set_return(log, KEYS, r.getrandbits(32))
    set_return(log, TEXT, r.getrandbits(32))
    blob(log, TOKEN, 0, 1)   # (the copied text's first two letters: the rest of the stack buffer is garbage)
    blob_bytes(log, BOX, 0, 0xf7f)
    message = r.deal([5, 0x14, 0x15, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x801, 0x801, 4, 0x16, 0x101])
    a = r.deal([0x5da3, 0x5da4, 0x5da5, 0]) if message == 0x801 else r.deal([0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffffe, 0x5da0, 0x5da1, 0x5da2, 0x5d9f, 0x5da3, 0])
    return Call(0x4a7d70, [WINDOW, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

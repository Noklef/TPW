# The park map opened (src/game/world/parkmapwindow: ParkMap_Open 0x5f0b40) and its two handlers (0x5f0b00, 0x5f0b20) against the original: Window_FindChild real over a built window's children, the rest recorders.
# usage: parkmapopen.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, fill, heap, log_fold, log_region, put, set_return, vtable  # noqa: E402

CLASSIFY, HANDLER, PAIR, SELECT, STATE, DEPTH, IMPL100 = 0, 1, 2, 3, 4, 7, 6
PAUSE, FLAGS, SETFLAGS, RELEASE, BUILD, FOCUS, TEXTPART, SETIMPL, NOTIFYC00, NOTIFY3000, SET4C, NOTIFY40000 = 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
PREPARE, PICTURE, CREATE, CLEAR, DRAW, UNCACHE, HANDLEWINDOW, HANDLEMOUSE = 20, 21, 22, 23, 24, 25, 26, 27
F662411, F5996D0, CLOSE, SEND, FOCUSSET, FONT, ADVISOR, HELP, GRID, HAS = 21, 22, 23, 24, 25, 26, 28, 29, 30, 31  # (cdecl; GRID and HAS, whose fill and answer are set, share a k with no other recorder)
B = Bench([(0x5f0b00, 0x5f1034), (0x65d7bc, 0x65d822)],
          hooks={0x48c8d0: _recorder(HAS, 0, cdecl=True), 0x4092a0: _recorder(PAUSE, 2), 0x662411: _recorder(F662411, 0, cdecl=True), 0x5996d0: _recorder(F5996D0, 1, cdecl=True),
                 0x576400: _recorder(FLAGS, 0), 0x5763f0: _recorder(SETFLAGS, 1), 0x58dd60: _recorder(RELEASE, 1), 0x485b40: _recorder(CLOSE, 0, cdecl=True), 0x658d9f: _recorder(SEND, 4, cdecl=True),
                 0x65fd0e: _recorder(BUILD, 2), 0x485b70: _recorder(FOCUSSET, 1, cdecl=True), 0x65e59b: _recorder(FOCUS, 1), 0x48f6a0: _recorder(TEXTPART, 1), 0x65d1c8: _recorder(SETIMPL, 1),
                 0x65c428: _recorder(NOTIFYC00, 2), 0x65c4b9: _recorder(NOTIFY3000, 2), 0x485a70: _recorder(FONT, 1, cdecl=True), 0x65c388: _recorder(SET4C, 1), 0x65c5d5: _recorder(NOTIFY40000, 4),
                 0x65f5b7: _recorder(IMPL100, 1), 0x65f16b: _recorder(DEPTH, 1), 0x668bec: _recorder(STATE, 2), 0x66a295: _recorder(SELECT, 1), 0x668f35: _recorder(PAIR, 2), 0x65e526: _recorder(HANDLER, 1),
                 0x450b00: _recorder(GRID, 2, cdecl=True), 0x5f2050: _recorder(CLASSIFY, 0), 0x5f3710: _recorder(PREPARE, 0), 0x5f1ea0: _recorder(PICTURE, 1), 0x5777b0: _recorder(CREATE, 4),
                 0x5777a0: _recorder(CLEAR, 0), 0x5f2380: _recorder(DRAW, 0), 0x577990: _recorder(UNCACHE, 4), 0x486b00: _recorder(ADVISOR, 1, cdecl=True), 0x492530: _recorder(HELP, 2, cdecl=True),
                 0x5f1130: _recorder(HANDLEWINDOW, 4), 0x5f18d0: _recorder(HANDLEMOUSE, 4)})
MAP, WINDOW, CHILDREN, VT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x1800
TAIL = MAP + 0x20060
SIZE = 0xe4  # a child: the vtable, next +0x54, id +0x8c, the depth word +0xe0
IDS = [0x991, 0x990, 0x995, 0x982, 0x987, 0x989, 0x993, 0x994, 0x983, 0x98b, 0x996, 0x98a, 0x984, 0x981, 0x988, 0x992]
CAMERA = [-50.0, -0.5, 0.0, 0.25, 1.5, 12.0, 99.9, 640.0, 1279.5, 1280.0, 16777217.0, 1e12, float('inf'), float('nan')]
SIZES = [0, 1, 2, 50, 64, 128, 128, 256, 16777217, -3]


def open_case(r):
    log = log_region()
    log_fold(log)
    regions = {LOG: log, IMPORT_TABLE: bytes(0x600), HEAP: heap(0x1000, r.choice([0, 0, 0, 0, 1]))}
    set_return(log, HAS, r.choice([0] * 9 + [1, 0x10000]))
    set_return(log, BUILD, WINDOW)
    # The children in any order, perhaps one missing, an id of none of them at the head at times.
    ids = list(IDS)
    r.shuffle(ids)
    if r.random() < 0.15:
        ids.pop(r.randrange(len(ids)))
    if r.random() < 0.3:
        ids.insert(0, r.choice([0x980, 0x99a, 0x10991]))
    children = bytearray(SIZE * len(ids))
    for i, child in enumerate(ids):
        put(children, SIZE * i, 'I', VT)
        put(children, SIZE * i + 0x54, 'I', CHILDREN + SIZE * (i + 1) if i + 1 < len(ids) else 0)
        put(children, SIZE * i + 0x8c, 'I', child)
        put(children, SIZE * i + 0xe0, 'h', r.choice([0, 1, 7, 0xff, 0x100, 0x7fff, -1, -2, -0x8000]))
    regions[CHILDREN] = children
    regions[VT] = vtable({5: 1})
    window = bytearray(0x60)
    put(window, 0x5c, 'I', CHILDREN)
    regions[WINDOW] = window
    head = bytearray(r.bytes(0x60))
    put(head, 4, 'I', r.choice([0] * 9 + [WINDOW, 1]))
    put(head, 0x18, 'I', r.choice([0, 0xff, 1, 2, 4, 8, 0x10, 0x20, 0xc0, 0xffffff00, r.getrandbits(32)]))
    put(head, 0x1c, 'i', r.choice([0, 1, 2, 3, 4, 5, -1, 0x10001]))
    put(head, 0x20, 'i', r.choice([0, 1, 2, 3, 4, -1, 0x10002]))
    regions[MAP] = head
    regions[TAIL] = r.bytes(0x30)
    # The grid's size, written by the recorder; the camera and the terrain's size, writable globals.
    fill(log, GRID, 0, struct.pack('<i', r.choice(SIZES)))
    fill(log, GRID, 1, struct.pack('<i', r.choice(SIZES)), which=1)
    regions[0x790ab8] = struct.pack('<f', r.choice(CAMERA))
    regions[0x790ac0] = struct.pack('<f', r.choice(CAMERA))
    regions[0x7a0314] = struct.pack('<i', r.choice(SIZES))
    regions[0x7a0374] = struct.pack('<i', r.choice(SIZES))
    for at in (0x786ba4, 0x8bd47c, 0x87a6d0, 0x8c4b44, 0x7cb2ac, 0xfaa5d8):
        regions[at] = struct.pack('<I', r.getrandbits(32))
    return Call(0x5f0b40, [MAP], regions, conv='t', ret='v')


def handler_case(r, address, k):
    log = log_region()
    set_return(log, k, r.getrandbits(32))
    return Call(address, [r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], {LOG: log}, conv='c', ret='a')


def case(r):
    op = r.deal(['open', 'open', 'open', 'open', 'open', 'open', 'window', 'mouse'])
    if op == 'window':
        return handler_case(r, 0x5f0b00, HANDLEWINDOW)
    if op == 'mouse':
        return handler_case(r, 0x5f0b20, HANDLEMOUSE)
    return open_case(r)


if __name__ == '__main__':
    B.main(case)

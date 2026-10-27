# The GUI library's 0x705a20 window, a text field going by its buffer (src/game/ui/window705a20: 0x66656c) against the original.
# SetCaret: a window of its own (0x170 random bytes: the words it reads any) with 0-2 parents (+0x60), each shown or not (flag 0 of +0x48),
# its caret object +0x15c one recorder object or none; the new one that, another, or none. The objects' words +0x10 / +0x12 small
# (WindowImpl_Release answers -1 now and then: the old one's slot 0 (1) then); slot 3's event hashed by its set words +4-+9 and, through
# its +0x14, the GUI rectangle's twelve bytes (the pointer itself is a stack address, the rest of the event unset).
# MoveCaret (0x66836a): SetCaret's window and caret objects (+0x15c one or none), slot 3's event hashed only by the rectangle its +0x14
# points to (nothing else of it is set). KeyToParent (0x667fee): keys 9, 0xd, 0x1b or any (the pushed register's upper half random),
# flag 0x80 of +0x48 either way, the parent +0x60 a window (byte +4 0 or 1, the handler +0x114 fncmp's logger or 0) or none, the hook
# [0xfaa5fc] the logger or 0. ToggleInsert (0x6667eb): the words +0x14c and +0x158 -1, 0, 1 or any, the caret +0x15c a recorder
# object (slot 5) or none, [0xfaa5d4] 0 or 1, window.py's queues (the window's byte +4 mostly clear). SetInsertMode (0x666875): that,
# the mode 0, 1 or any word (upper half random), called as fastcall with EDX random (its upper half pushed for mode 0). NotifyText
# (0x66671d): the object +0x9c a recorder (slot 3, its event hashed by the text its +0x10 points to: dword +0, word +0xa) or none, post
# 0, 1 or any, the parent +0x60 a window (byte +4 mostly clear) or none, window.py's queues.
# SetBuffer (0x6662b7): flag 0x40 of +0x48 either way, the buffer and the size 0 or any. SelectionStart / Selection (0x668447, 0x66848f):
# the anchor +0x14c and the cursor +0x14a -1, small or any (signed both ways), Selection's word pointers each a slot of the window or 0.
# NotifyScroll (0x66679e): see notify_scroll().
# usage: window705a20.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetCaret,MoveCaret,KeyToParent,ToggleInsert,SetInsertMode,
# NotifyText,SetBuffer,SelectionStart,Selection,NotifyScroll)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, blob_bytes, log_region, put, vtable  # noqa: E402
from window import RANGES, queues  # noqa: E402

CALLS = {'SetCaret': 0x66656c, 'MoveCaret': 0x66836a, 'KeyToParent': 0x667fee, 'ToggleInsert': 0x6667eb,
         'SetInsertMode': 0x666875, 'NotifyText': 0x66671d, 'SetBuffer': 0x6662b7, 'SelectionStart': 0x668447, 'Selection': 0x66848f, 'NotifyScroll': 0x66679e}
B = Bench(sorted(set(RANGES + [(0x66656c, 0x66671d), (0x659d43, 0x659d90), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00),
                            (0x66bff7, 0x66c01b), (0x66836a, 0x668447), (0x667fee, 0x6680c7), (0x658d9f, 0x658e04),
                            (0x6667eb, 0x666875), (0x666875, 0x6668e9), (0x66671d, 0x66679e), (0x66679e, 0x6667eb), (0x658ccc, 0x658d9f), (0x6697e3, 0x6698b4), (0x680050, 0x68005b),
                            (0x6662b7, 0x666308), (0x668447, 0x66852b)])))
WINDOW, OBJ_A, OBJ_B, VT, PARENTS = RAM, RAM + 0x200, RAM + 0x240, RAM + 0x280, RAM + 0x400


def set_caret(r):
    log = log_region()
    regions = {LOG: log, VT: vtable({0: 1, 3: 2, 5: 1})}
    for obj in (OBJ_A, OBJ_B):
        regions[obj] = put(put(bytearray(r.bytes(0x20)), 0, 'I', VT), 0x10, 'hh', r.choice([0, 1, 2]), r.choice([0, 1, 1, 2, 3]))
    w = bytearray(r.bytes(0x170))
    w[0x48] = (w[0x48] & 0xfe) | r.choice([0, 1, 1])
    n = r.choice([0, 1, 2])
    put(w, 0x60, 'I', PARENTS if n else 0)
    for i in range(n):
        p = bytearray(r.bytes(0x70))
        p[0x48] = (p[0x48] & 0xfe) | r.choice([0, 1, 1, 1])
        put(p, 0x60, 'I', PARENTS + 0x100 * (i + 1) if i + 1 < n else 0)
        regions[PARENTS + 0x100 * i] = p
    old = r.choice([OBJ_A, OBJ_A, 0])
    put(w, 0x15c, 'I', old)
    regions[WINDOW] = w
    blob_bytes(log, 3, 1, 0x3f0, (0x14, 0xfff))
    return Call(CALLS['SetCaret'], [WINDOW, r.choice([OBJ_A, OBJ_B, OBJ_B, OBJ_B, 0])], regions, conv='t', ret='v')


def move_caret(r):
    log = log_region()
    regions = {LOG: log, VT: vtable({3: 2})}
    regions[OBJ_A] = put(bytearray(r.bytes(0x20)), 0, 'I', VT)
    w = bytearray(r.bytes(0x170))
    put(w, 0x15c, 'I', r.choice([OBJ_A, OBJ_A, OBJ_A, 0]))
    regions[WINDOW] = w
    blob_bytes(log, 3, 1, 0, (0x14, 0xfff))
    return Call(CALLS['MoveCaret'], [WINDOW, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='v')


def key_to_parent(r):
    log = log_region()
    regions = {LOG: log}
    w = bytearray(r.bytes(0x170))
    w[0x48] = (w[0x48] & 0x7f) | r.choice([0, 0x80])
    p = bytearray(r.bytes(0x120))
    p[4] = r.choice([0, 0, 0, 1])
    put(p, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
    put(w, 0x60, 'I', r.choice([PARENTS, PARENTS, PARENTS, 0]))
    regions[WINDOW], regions[PARENTS] = w, p
    regions[0xfaa5fc] = put(bytearray(4), 0, 'I', r.choice([0, CDECL_LOG4]))
    key = r.choice([9, 0xd, 0xd, 0x1b, r.getrandbits(16), 0x8000 | r.getrandbits(15)]) | r.choice([0, r.getrandbits(16)]) << 16
    return Call(CALLS['KeyToParent'], [WINDOW, key, r.getrandbits(32)], regions, conv='t', ret='a')


def toggle_insert(r, op='ToggleInsert'):
    log = log_region()
    regions = {LOG: log, VT: vtable({5: 1})}
    regions[OBJ_A] = put(bytearray(r.bytes(0x20)), 0, 'I', VT)
    w = bytearray(r.bytes(0x170))
    w[4] = r.choice([0, 0, 0, 1])
    put(w, 0x14c, 'h', r.choice([-1, -1, -1, 0, 1, r.getrandbits(15)]))
    put(w, 0x158, 'h', r.choice([0, 1, 0, 1, -1, r.getrandbits(15)]))
    put(w, 0x15c, 'I', r.choice([OBJ_A, OBJ_A, 0]))
    regions[WINDOW] = w
    regions[0xfaa5d4] = struct.pack('<I', r.choice([0, 1]))
    queues(r, regions)
    if op == 'SetInsertMode':
        put(w, 0x158, 'h', r.choice([0, 1, -1, r.getrandbits(15)]))
        mode = r.choice([0, 0, 1, 1, 2, 0xffff, r.getrandbits(16)]) | r.choice([0, r.getrandbits(16)]) << 16
        return Call(CALLS[op], [WINDOW, r.getrandbits(32), mode], regions, conv='f', ret='v')
    return Call(CALLS['ToggleInsert'], [WINDOW], regions, conv='t', ret='v')


def notify_text(r):
    log = log_region()
    regions = {LOG: log, VT: vtable({3: 2})}
    regions[OBJ_B] = put(bytearray(r.bytes(0x20)), 0, 'I', VT)
    w = bytearray(r.bytes(0x170))
    put(w, 0x9c, 'I', r.choice([OBJ_B, OBJ_B, 0]))
    put(w, 0x60, 'I', r.choice([PARENTS, PARENTS, 0]))
    regions[PARENTS] = put(bytearray(r.bytes(0x20)), 4, 'B', r.choice([0, 0, 0, 1]))
    regions[WINDOW] = w
    queues(r, regions)
    blob_bytes(log, 3, 1, 0, (0x10, 0xc0f))
    return Call(CALLS['NotifyText'], [WINDOW, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='v')


def notify_scroll(r):
    """NotifyScroll (0x66679e): the object +0x9c a recorder (slot 3, the event's second one's word +0xc hashed) or none."""
    log = log_region()
    regions = {LOG: log, VT: vtable({3: 2})}
    regions[OBJ_B] = put(bytearray(r.bytes(0x20)), 0, 'I', VT)
    w = bytearray(r.bytes(0x170))
    put(w, 0x9c, 'I', r.choice([OBJ_B, OBJ_B, 0]))
    regions[WINDOW] = w
    blob_bytes(log, 3, 1, 0, (0x10, 0x3000))
    return Call(CALLS['NotifyScroll'], [WINDOW], regions, conv='t', ret='v')


def selection(r, op):
    w = bytearray(r.bytes(0x170))
    w[0x48] = (w[0x48] & 0xbf) | r.choice([0, 0x40])
    for at in (0x14a, 0x14c):
        put(w, at, 'h', r.choice([-1, -1, 0, 1, 2, r.randrange(0, 0x40), r.getrandbits(15), -r.getrandbits(15)]))
    regions = {LOG: log_region(), WINDOW: w}
    if op == 'SetBuffer':
        return Call(CALLS[op], [WINDOW, r.choice([0, RAM + 0x800, r.getrandbits(32)]), r.choice([0, 0x200, r.getrandbits(32)])],
                    regions, conv='t', ret='a')
    if op == 'SelectionStart':
        return Call(CALLS[op], [WINDOW], regions, conv='t', ret='a')
    return Call(CALLS[op], [WINDOW, r.choice([WINDOW + 0x168, 0]), r.choice([WINDOW + 0x16a, WINDOW + 0x168, 0])], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    return {'SetCaret': set_caret, 'MoveCaret': move_caret, 'KeyToParent': key_to_parent, 'ToggleInsert': toggle_insert,
            'SetInsertMode': lambda r: toggle_insert(r, 'SetInsertMode'), 'NotifyText': notify_text, 'NotifyScroll': notify_scroll,
            'SetBuffer': lambda r: selection(r, 'SetBuffer'), 'SelectionStart': lambda r: selection(r, 'SelectionStart'),
            'Selection': lambda r: selection(r, 'Selection')}[op](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

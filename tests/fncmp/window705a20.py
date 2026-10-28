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
# NotifyScroll (0x66679e): see notify_scroll(). SetSelection / SetCursor / SelectAll (0x6668e9, 0x66778d, 0x6677ae; SelectAll's
# byte +0x162 set more often): see set_selection(). SetText and its two
# callers (0x666308, 0x6664fa, 0x666515): see set_text(). GetText (0x666530): see get_text().
# usage: window705a20.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetCaret,MoveCaret,KeyToParent,ToggleInsert,SetInsertMode,
# NotifyText,SetBuffer,SelectionStart,Selection,NotifyScroll,SetSelection)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, answers, blob_bytes, blob_when_inner, heap, log_region, put, recorder, set_return, vtable  # noqa: E402
from window import RANGES, queues  # noqa: E402

CALLS = {'SetCaret': 0x66656c, 'MoveCaret': 0x66836a, 'KeyToParent': 0x667fee, 'ToggleInsert': 0x6667eb,
         'SetInsertMode': 0x666875, 'NotifyText': 0x66671d, 'SetBuffer': 0x6662b7, 'SelectionStart': 0x668447, 'Selection': 0x66848f, 'NotifyScroll': 0x66679e,
         'SetSelection': 0x6668e9, 'SetText': 0x666308, 'SetTextNotify': 0x6664fa, 'SetTextQuiet': 0x666515, 'GetText': 0x666530,
         'SetCursor': 0x66778d, 'SelectAll': 0x6677ae}
WIN_RANGES = sorted(set(RANGES + [(0x66656c, 0x66671d), (0x659d43, 0x659d90), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00),
                            (0x66bff7, 0x66c01b), (0x66836a, 0x668447), (0x667fee, 0x6680c7), (0x658d9f, 0x658e04),
                            (0x6667eb, 0x666875), (0x666875, 0x6668e9), (0x66671d, 0x66679e), (0x66679e, 0x6667eb), (0x658ccc, 0x658d9f), (0x6697e3, 0x6698b4), (0x680050, 0x68005b),
                            (0x6662b7, 0x666308), (0x668447, 0x66852b), (0x6668e9, 0x66775d), (0x659d00, 0x659d43),
                            (0x659710, 0x659740), (0x67acd0, 0x67ad00), (0x659dbc, 0x659de5),
                            (0x666308, 0x666515), (0x666515, 0x66656c), (0x66778d, 0x6677ae), (0x6677ae, 0x667801), (0x65f5b7, 0x65f66e),
                            (0x67bf10, 0x67bf50), (0x67c290, 0x67c320), (0x67b0d0, 0x67b140)]))
B = Bench(WIN_RANGES)
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


SEL = RAM + 0x20000
SEL_FONT, SEL_CHARS, SEL_CARET, SEL_FONTVT, SEL_CHARSVT, SEL_CARETVT = SEL + 0x200, SEL + 0x220, SEL + 0x240, SEL + 0x300, SEL + 0x380, SEL + 0x400
SEL_TABLE, SEL_ENTS, SEL_TEXT, SEL_SRC = SEL + 0x480, SEL + 0x600, SEL + 0x800, SEL + 0xa00
GET_TO = RAM + 0x40000   # (an upper half other than the window's)


def sel_fixture(r):
    """SetSelection (0x6668e9): a text of 0-20 characters (or none), the font +0x4c a recorder object (slot 1 the height) or none, the line
    height +0x156 0 or 8-16, [0xfaa5c0] 0x600 mostly; flag 0x80 of +0x48 either way; the cursor / anchor +0x14a / +0x14c and the caret's
    words +0x148-+0x158 small about the text and the rectangle +0x30-+0x36 (width 0, 40 or 100); the object +0x9c (mostly) a recorder whose
    slot 4 answers in turn from a table of 48 of ten entries (x about the caret's, y the caret's line, one above or below; fncmp.answers()),
    slot 3's events hashed by their second event's set bytes (0x4100: text +0, scroll +0xa; 0x8000: +0xc); the caret +0x15c a recorder
    object (slot 5; slot 3's event hashed by its rectangle) or none, [0xfaa5d4] 0 or 1 with window.py's queues. The cursor -7 to -1, about
    the text or any; the anchor -5 to -1, about the text or any (their upper halves random); the mark 0, 1 or any."""
    log = log_region()
    regions = {LOG: log, SEL_FONTVT: vtable({}), SEL_CHARSVT: vtable({3: 2, 4: 2})}
    caret_vt = vtable({5: 1})
    put(caret_vt, 12, 'I', recorder(13, 2))
    regions[SEL_CARETVT] = caret_vt
    for obj, vt in ((SEL_FONT, SEL_FONTVT), (SEL_CHARS, SEL_CHARSVT), (SEL_CARET, SEL_CARETVT)):
        regions[obj] = put(bytearray(r.bytes(0x20)), 0, 'I', vt)
    n = r.choice([0, 1, 3, 8, 12, 20])
    regions[SEL_TEXT] = text = b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(n)) + bytes(2)
    regions[SEL_TEXT] += r.bytes(0x200 - len(text))   # (room for SetText's copies)
    flags = r.choice([0, 0x80])
    line = r.choice([8, 12, 16])
    w = bytearray(r.bytes(0x170))
    put(w, 0x48, 'I', (struct.unpack_from('<I', w, 0x48)[0] & ~0x80) | flags)
    put(w, 0x138, 'I', r.choice([SEL_TEXT] * 12 + [0]))
    put(w, 0x4c, 'I', r.choice([SEL_FONT] * 12 + [0]))
    left, top = r.randrange(-5, 50), r.randrange(-5, 50)
    put(w, 0x30, 'hhhh', left, top, left + r.choice([0, 40, 100, 100]), top + r.choice([0, 20, 60, 60]))
    put(w, 0x148, 'h', r.choice([0, 5, -10, r.getrandbits(15)]))
    put(w, 0x14a, 'hh', r.choice([0, 1, n // 2, n, n + 1, -1]), r.choice([-1, -1, 0, 1, n // 2, n, n + 2]))
    put(w, 0x14e, 'h', r.choice([0, -line, 2 * line, r.getrandbits(15)]))
    caret_y = r.choice([0, line, 2 * line, -line])
    put(w, 0x150, 'hhh', r.randrange(-8, 110), r.randrange(-8, 110), caret_y)
    put(w, 0x156, 'hh', r.choice([0, 0, line]), r.choice([-1, 0, 1]))
    w[0x162] = r.choice([0] * 5 + [1])
    put(w, 0x9c, 'I', r.choice([SEL_CHARS] * 8 + [0]) if flags else SEL_CHARS)   # (one line: the object is called unchecked)
    put(w, 0x15c, 'I', r.choice([SEL_CARET, SEL_CARET, 0]))
    regions[SEL] = w
    ents = bytearray()
    for _ in range(10):
        e = bytearray(r.bytes(16))
        put(e, 4, 'hh', r.choice([r.randrange(-12, 120), r.randrange(-12, 120), r.getrandbits(15)]),
            caret_y + r.choice([-line, 0, 0, line, line, 2 * line, -2 * line, r.randrange(-3, 4)]))
        ents += e
    regions[SEL_ENTS] = ents
    regions[SEL_TABLE] = struct.pack('<48I', *[SEL_ENTS + 16 * r.randrange(10) for _ in range(48)])
    answers(log, 4, SEL_TABLE)
    set_return(log, 1, r.choice([8, 12, 16, 0, 0x7fff]) | r.choice([0, r.getrandbits(16)]) << 16)
    regions[0xfaa5c0] = struct.pack('<H', r.choice([0x600] * 6 + [1, 0x300, 0xffff]))
    regions[0xfaa5d4] = struct.pack('<I', r.choice([0, 1]))
    queues(r, regions)
    blob_bytes(log, 3, 1, 0)
    blob_when_inner(log, 3, 0, 0x4100, (0x10, 0xc0f), 0)
    blob_when_inner(log, 3, 0, 0x8000, (0x10, 0x3000), 1)
    blob_bytes(log, 13, 1, 0, (0x14, 0xfff))
    return regions, log, n


def sel_args(r, n):
    upper = r.choice([0, r.getrandbits(16)]) << 16
    cursor = r.choice(list(range(-7, 0)) * 2 + [0, 1, n // 2, n, n + 3, r.getrandbits(16)]) & 0xffff | upper
    anchor = r.choice(list(range(-5, 0)) * 2 + [0, 1, n, n + 4, r.getrandbits(16)]) & 0xffff | r.choice([0, r.getrandbits(16)]) << 16
    return cursor, anchor


def set_selection(r, op='SetSelection'):
    regions, log, n = sel_fixture(r)
    if op == 'SelectAll':   # (the byte +0x162 set more often; the text +0x138 or none)
        regions[SEL][0x162] = r.choice([0, 0, 1, r.getrandbits(8)])
        return Call(CALLS[op], [SEL], regions, conv='t', ret='v')
    cursor, anchor = sel_args(r, n)
    if op == 'SetCursor':
        return Call(CALLS[op], [SEL, cursor, anchor], regions, conv='t', ret='v')
    mark = r.choice([0, 1, r.getrandbits(32)])
    return Call(CALLS['SetSelection'], [SEL, cursor, mark, anchor], regions, conv='t', ret='v')


def set_text(r, op):
    """SetText / SetTextNotify / SetTextQuiet (0x666308, 0x6664fa, 0x666515): SetSelection's fixture (the buffer +0x138 its text, 0x200
    bytes, or none), flag 0x40 of +0x48 either way, the size +0x13c 0, about the texts' lengths or 0xff; the text a second one of 0-30
    characters (now and then the buffer's own text, or the same as it), the buffer itself or none; the byte +0x160 0 or 1; +0x98 none
    or the object +0x9c (slot 3's 0x100 hashed: the second event's dword +0); heap() for its blocks (failing now and then)."""
    regions, log, n = text_fixture(r)
    m = r.choice([0, 1, 3, 12, 20, 30])
    src = b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(m)) + bytes(2)
    if r.random() < 0.25:
        m, src = n, regions[SEL_TEXT][:2 * n + 2]
    regions[SEL_SRC] = src + r.bytes(0x40)
    put(regions[SEL], 0x13c, 'I', r.choice([0, 1, n, m, m - 1 if m else 0, m + 1, 0xff]))
    text = r.choice([SEL_SRC] * 6 + [SEL_TEXT, 0])
    if op == 'SetText':
        return Call(CALLS[op], [SEL, text, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions, conv='t', ret='w')
    return Call(CALLS[op], [SEL, text], regions, conv='t', ret='w')


def text_fixture(r):
    """SetSelection's fixture for setting the text: flag 0x40 of +0x48 either way, the byte +0x160 0 or 1, +0x98 none or the object
    +0x9c (always it), the parent +0x60 or none, heap(); the size +0x13c left to the caller. Returns (regions, log, n)."""
    regions, log, n = sel_fixture(r)
    w = regions[SEL]
    w[0x48] = (w[0x48] & 0xbf) | r.choice([0, 0x40])
    w[0x160] = r.choice([0, 1])
    put(w, 0x98, 'I', r.choice([0, SEL_CHARS]))
    put(w, 0x9c, 'I', SEL_CHARS)   # (none with several lines leaves SetSelection's bottom unset: the stack's)
    put(w, 0x60, 'I', r.choice([PARENTS, PARENTS, 0]))
    regions[PARENTS] = put(bytearray(r.bytes(0x20)), 4, 'B', r.choice([0, 0, 0, 1]))
    regions[HEAP] = heap(0x1000, r.choice([0] * 10 + [1]))
    blob_when_inner(log, 3, 0, 0x100, (0x10, 0xf), 2)
    return regions, log, n


def get_text(r):
    """GetText (0x666530): the buffer a text of 0-20 characters or none; `to` at another upper half, 0-0x30 characters of 0x80
    bytes; EAX compared."""
    n = r.choice([0, 1, 5, 20])
    w = bytearray(r.bytes(0x170))
    put(w, 0x138, 'I', r.choice([SEL_TEXT] * 4 + [0]))
    regions = {LOG: log_region(), SEL: w, GET_TO: bytearray(r.bytes(0x80)),
               SEL_TEXT: b''.join(struct.pack('<H', r.randrange(1, 0x10000)) for _ in range(n)) + bytes(2) + r.bytes(0x40)}
    return Call(CALLS['GetText'], [SEL, GET_TO, r.choice([0, 1, n, n + 1, 0x30])], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    return {'SetCaret': set_caret, 'MoveCaret': move_caret, 'KeyToParent': key_to_parent, 'ToggleInsert': toggle_insert,
            'SetInsertMode': lambda r: toggle_insert(r, 'SetInsertMode'), 'NotifyText': notify_text, 'NotifyScroll': notify_scroll,
            'SetBuffer': lambda r: selection(r, 'SetBuffer'), 'SelectionStart': lambda r: selection(r, 'SelectionStart'),
            'Selection': lambda r: selection(r, 'Selection'), 'SetSelection': set_selection,
            'SetText': lambda r: set_text(r, 'SetText'), 'SetTextNotify': lambda r: set_text(r, 'SetTextNotify'),
            'SetTextQuiet': lambda r: set_text(r, 'SetTextQuiet'), 'GetText': get_text,
            'SetCursor': lambda r: set_selection(r, 'SetCursor'), 'SelectAll': lambda r: set_selection(r, 'SelectAll')}[op](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

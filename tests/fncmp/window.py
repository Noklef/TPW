# The GUI library's window child search (src/game/ui/window, 0x65d7d0) and the 0x7ca1c8 dialog's check buttons (src/game/ui/dialog7ca1c8,
# 0x498090 / 0x498a50) against the originals. FindChild: a window with 0-6 children (ids from a few, repeats and misses) searched for one id.
# SetChecks / SetChecksIfUp: the dialog's child 0x18 holds the nine buttons 0x18-0x21 shuffled among a few others (now and then one missing:
# a fault in both), each with a recorder vtable whose slot 5 logs its bool (the low byte: the original pushes 0x20's with ECX's upper half);
# [0x7ca1d0] +0x50 and [0x7ca1c8] / [0x7ca1cc] (the IfUp guard) set or 0. CallHandler / SendMessage (0x65f697 / 0x658d9f) and the dialog
# wrappers that send to [0x7cc47c] (src/game/ui/dialogs): the window (or none) with its byte +4 now and then set, its handler +0x114 and the
# hook [0xfaa5fc] fncmp's four-argument cdecl logger or 0 (its result now and then set). PostMessage (0x658ccc): each message kind, the
# window or none (its byte +4 now and then set), the two queues 1-4 entries (free or not) or none. Dialog7cb2b8_Child21Post (0x4a2590):
# the dialog or none, its child 0x21 or none among others, the child's dword +0xa8 set or not, the value 0 or not. HitTest / UpdateHover
# (0x65db25, 0x6589f9): a tree of 1-7 windows with real GUI rectangles (the driver resolves vtable 0x705920 to the ports), points near
# them or anywhere (upper halves random), the root, the hovered window and the queues set or not. NotifyShown / SetShown (0x65d90b,
# 0x65d9dd): UpdateHover's tree with parents, shown or hidden, recorder objects +0x98 / +0x9c, handlers, the manager. ImplRelease / IsShown / OffsetByParents
# (0x6616b0, 0x65d773, 0x65c979): recorder objects, chains of 0-4 windows by +0x60. RectAssign / SaveRect (0x65aa20, 0x65bc90): random.
# Ctor (0x65bcad): a random window built with a parent from a tree (its +0x60 chain and word +0xe0 near 0x100) or none, flags 0x400 and 1
# set or not, words edges or near (upper halves random); the root the new window, the tree's or none, the queues as above.
# ForgetGlobals / DropModal / Destroying (0x6592b6, 0x65e672, 0x65c2d8): the globals the window (or one in its tree) or others (upper
# halves too); Destroying on a tree as HitTest's, the handlers fncmp's logger or 0, the hook [0xfaa5fc] set or not.
# CancelPosted / CancelPostedFirst / CancelTimed (0x658fe9, 0x659012, 0x65903b): the queues as above and the timed table [0xfaa5f8]
# (0-4 entries or none) or 0, entries the window's now and then; the window or 0. Dtor / DeletingDtor (0x65c025, 0x661680): a window
# (going or not) with its objects +0x98 / +0x9c recorders (their counts near letting go) or 0, its timer flags set or not over a timer
# list [0xfaa634] with its and others' nodes, its rectangle +0x44 its own, a recorder or 0, a parent and siblings or none, 0-3 real
# child windows (vtable 0x705980: their own deleting destructors unlink them), the manager [0x77c48c] a recorder, the globals, queues and
# timed table as above, the hover's root a tree's or none; now and then an object's counts both -0x8000 (its release's 16-bit result
# 0xffff, the 32-bit one not -1). Dtor705a50 / DeletingDtor705a50 (0x6689bc, 0x6693a0): the same, with the text row +0x148 or 0; the
# children of either window class (the driver resolves both vtables).
# HilightList_NewRow / List7cc188_NewRow (0x480d30 / 0x4a8380, src/game/ui/listrows): the Ctor's surroundings, the heap failing now and then,
# font 7 (Language_Font, of group 0-2) a recorder whose slot 1 gives its height (edges too), the divisor word [0xfaa5c0] any but 0.
# CallDefault / WindowAt10_Handler / Dialog7cb2b8_Handler / DefaultHandler (0x65f66e, 0x492e00, 0x4a0a20, 0x4a0a90; [0x7cb2b8] random): the default handler +0x118 fncmp's logger, the window's object A0[0] random bytes,
# the message 0x14 often. GetRect / SetSavedRect (0x65d42e, 0x65cfd4): a GUI rectangle, a recorder or none, the insets small or edges. SaveInsetRect (0x485e30):
# random rectangles and insets. ResetQueues (0x659064): the queues and the timed table as for the cancels, each now and then missing.
# SetFocus / KillFocus (0x65e59b, 0x65e5fa): UpdateHover's tree and hover, the focus [0xfaa5ec] the window, another or none.
# Windows7cc8bc_SelectSecond (0x4bce80): SetFocus's set-up, the two windows [0x7cc8bc] / [0x7cc8c0] of the tree with handlers the logger or none.
# MouseDown / MouseUp (0x658af1, 0x658b5b): buttons 0-7, the windows the window or none, the queues as for PostMessage.
# usage: window.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FindChild,SetChecks,SetChecksIfUp)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, HEAP, LOG, LOG_BYTES, Bench, Call, RAM, heap, log_region, put, recorder, set_return, vtable  # noqa: E402

CALLS = {'FindChild': 0x65d7d0, 'SetChecks': 0x498090, 'SetChecksIfUp': 0x498a50, 'CallHandler': 0x65f697, 'SendMessage': 0x658d9f,
         'Dialog7cc47c_Message4': 0x4b87f0, 'Dialog7cc47c_Message6On': 0x4b8810, 'Dialog7cc47c_Message6Off': 0x4b8830,
         'Dialog7cc4b4_Message4': 0x4b8e80, 'Dialog7cc4b4_Message6Off': 0x4b8ec0, 'Dialog7cc4c4_Message4': 0x4b9c00, 'Dialog7cc4c4_Message6On': 0x4b9c20,
         'Dialog7cdb0c_Message4': 0x4c5780, 'Dialog7cdb0c_Message6On': 0x4c57a0, 'Dialog7cdb0c_Message6Off': 0x4c57c0,
         'Dialog7cdb0c_Child16007Message6On': 0x4c57e0, 'Dialog7cdb0c_Child16007Message6Off': 0x4c5800,
         'Dialog7ca098_Message4IfUp': 0x48d990, 'Dialog7cca94_Message4IfUp': 0x4c25e0, 'WindowAt10_Message4': 0x48cf70,
         'WindowAt10_Message4IfAny': 0x492e60, 'Windows7c26b0_Message5': 0x48ee20,
         'WindowAt10_ScalarDtor': 0x48c8b0, 'SetChecksIfUpThis': 0x481a00,
         'Windows7cc8bc_Message6Pair': 0x4bcef0, 'Windows7cc8bc_SelectSecond': 0x4bce80, 'Global7cc698_Choose': 0x4bcec0, 'ImplSlot7': 0x65d6db, 'ImplSlot6': 0x65d745, 'DwordA0': 0x65e705,
         'PostMessage': 0x658ccc, 'Dialog7cb2b8_Child21Post': 0x4a2590,
         'HitTest': 0x65db25, 'UpdateHover': 0x6589f9, 'NotifyShown': 0x65d90b, 'SetShown': 0x65d9dd, 'ImplRelease': 0x6616b0, 'IsShown': 0x65d773,
         'RectAssign': 0x65aa20, 'SaveRect': 0x65bc90, 'OffsetByParents': 0x65c979,
         'Ctor': 0x65bcad, 'ForgetGlobals': 0x6592b6, 'DropModal': 0x65e672, 'Destroying': 0x65c2d8,
         'CancelPosted': 0x658fe9, 'CancelPostedFirst': 0x659012, 'CancelTimed': 0x65903b,
         'Dtor': 0x65c025, 'DeletingDtor': 0x661680, 'Dtor705a50': 0x6689bc, 'DeletingDtor705a50': 0x6693a0,
         'HilightList_NewRow': 0x480d30, 'List7cc188_NewRow': 0x4a8380, 'CallDefault': 0x65f66e, 'WindowAt10_Handler': 0x492e00,
         'GetRect': 0x65d42e, 'SetSavedRect': 0x65cfd4, 'SaveInsetRect': 0x485e30,
         'Dialog7cb2b8_Handler': 0x4a0a20, 'DefaultHandler': 0x4a0a90, 'ResetQueues': 0x659064, 'SetFocus': 0x65e59b, 'KillFocus': 0x65e5fa,
         'MouseDown': 0x658af1, 'MouseUp': 0x658b5b}
SENDS = {k: int(k[6:12], 16) for k in CALLS if k.startswith('Dialog7')}   # the window's global
RANGES = [(0x498090, 0x4981e0), (0x498a50, 0x498a80), (0x65d7bc, 0x65d832), (0x65f697, 0x65f6d1), (0x658d9f, 0x658df4), (0x659710, 0x659721),
           (0x4b87f0, 0x4b8850), (0x4b8e80, 0x4b8ea0), (0x4b8ec0, 0x4b8ee0), (0x4b9c00, 0x4b9c40),
           (0x4c5780, 0x4c5820),
           (0x48d990, 0x48d9b3), (0x4c25e0, 0x4c25f9), (0x48cf70, 0x48cf83), (0x492e60, 0x492e77), (0x48ee20, 0x48ee4e),
           (0x48c8b0, 0x48c8ce), (0x481a00, 0x481a10),
           (0x4bcef0, 0x4bcf60), (0x4bcec0, 0x4bcef0), (0x4bce80, 0x4bcec0), (0x65d90b, 0x65d9dd), (0x65d9dd, 0x65da8d), (0x65d6db, 0x65d727), (0x65d745, 0x65d783), (0x65e705, 0x65e73d),
           (0x658ccc, 0x658d9f), (0x6697e3, 0x6698a4), (0x4a2590, 0x4a25e5),
           (0x65db25, 0x65dbee), (0x6589f9, 0x658aab), (0x659e83, 0x659ee2),
           (0x6616b0, 0x6616f1), (0x65d773, 0x65d7bc),
           (0x65aa20, 0x65aa90), (0x65bc90, 0x65bcad), (0x65c979, 0x65c9bd),
           (0x65bcad, 0x65c025), (0x659d00, 0x659d43), (0x65a9d0, 0x65aa00), (0x65a3e9, 0x65a43e), (0x67fff0, 0x680048),
           (0x6592b6, 0x659333), (0x65e672, 0x65e6a4), (0x65c2d8, 0x65c357),
           (0x658fe9, 0x659064), (0x669b3e, 0x669bdb), (0x669e3a, 0x669eb3),
           (0x65c025, 0x65c2c8), (0x661680, 0x6616b0), (0x659d90, 0x659dbc), (0x66bff7, 0x66c00b), (0x661f9b, 0x661fe5),
           (0x6620e2, 0x66213e), (0x662306, 0x662384), (0x662500, 0x66252c), (0x6689bc, 0x668a23), (0x6693a0, 0x6693cc),
           (0x66969e, 0x6696b4), (0x480d30, 0x480dc3), (0x4a8380, 0x4a8413), (0x485a70, 0x485a94),
           (0x65f66e, 0x65f697), (0x492e00, 0x492e34), (0x65d42e, 0x65d49e), (0x65cfd4, 0x65cffd), (0x65a43e, 0x65a493), (0x485e30, 0x485eb6), (0x4a0a20, 0x4a0aa9),
           (0x659064, 0x6590a5), (0x669eb3, 0x669efb), (0x669c62, 0x669c8d), (0x669bdb, 0x669c30),
           (0x65e59b, 0x65e5fa), (0x65e5fa, 0x65e640), (0x658af1, 0x658b5b), (0x658b5b, 0x658bcb)]
B = Bench(RANGES)
WINDOW, NODES, VTABLE, MISC, KIDS, HOLDER, WINDOW2 = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x1100, RAM + 0x1200, RAM + 0x1600, RAM + 0x1800
QUEUES, TREE, RECTS, TIMED = RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x5000
MANAGER, TNODES, RECT, ROWS, PARENT, CHILDREN = RAM + 0x5400, RAM + 0x5600, RAM + 0x5800, RAM + 0x5c00, RAM + 0x6000, RAM + 0x7000
NODE = 0x90
BUTTONS = list(range(0x18, 0x22))


def node(r, ident, nxt, child=0):
    buf = bytearray(r.bytes(NODE))
    put(buf, 0, 'I', VTABLE)
    put(buf, 0x54, 'I', nxt)
    put(buf, 0x5c, 'I', child)
    put(buf, 0x8c, 'I', ident)
    return buf


def chain(r, regions, ids, first, child_of=None):
    """nodes for ids from `first` on (each NODE bytes); the index in child_of gets that child pointer. Returns the head (0 for none)."""
    for i, ident in enumerate(ids):
        nxt = first + NODE * (i + 1) if i + 1 < len(ids) else 0
        regions[first + NODE * i] = node(r, ident, nxt, (child_of or {}).get(i, r.choice([0, r.getrandbits(32) | 0x80000000])))
    return first if ids else 0


def queues(r, regions):
    """the message queues [0xfaa5f4] / [0xfaa5f0] (as msgqueue.py: 1-4 entries, free or not) or 0"""
    for k, q in enumerate((QUEUES, QUEUES + 0x100)):
        cap = r.choice([1, 2, 4])
        entries = bytearray(r.bytes(16 * cap))
        for i in range(cap):
            if r.random() < 0.6:
                put(entries, 16 * i, 'I', 0)
        regions[q + 0x40] = entries
        regions[q] = struct.pack('<5I', q + 0x40, cap, 0, r.randrange(cap), r.getrandbits(8))
        regions[0xfaa5f4 - 4 * k] = struct.pack('<I', q if r.random() < 0.85 else 0)


def timers(r, regions, window):
    """the queues (queues()) and the timed table [0xfaa5f8] (0-4 entries of 0x14 bytes or none) or 0, entries now and then the window's"""
    queues(r, regions)
    for q in (QUEUES, QUEUES + 0x100):
        entries = regions[q + 0x40]
        for i in range(len(entries) // 16):
            if r.random() < 0.4:
                put(entries, 16 * i, 'I', window)
    n = r.choice([0, 1, 2, 4])
    entries = bytearray(r.bytes(0x14 * n))
    for i in range(n):
        if r.random() < 0.5:
            put(entries, 0x14 * i, 'I', window)
    if n:
        regions[TIMED + 0x20] = entries
    regions[TIMED] = struct.pack('<II', n, r.choice([TIMED + 0x20] * 5 + [0]))
    regions[0xfaa5f8] = struct.pack('<I', r.choice([TIMED] * 5 + [0]))


def destructor(r, regions, op):
    regions[HEAP] = heap()
    wins = tree(r, regions)
    x, y = point(r, regions, wins)
    regions[0xfaa540] = struct.pack('<II', x, y)
    win = bytearray(r.bytes(0x150))
    win[4] = 0 if r.random() < 0.7 else 1
    put(win, 0x148, 'I', r.choice([ROWS, ROWS, 0]))
    regions[ROWS] = r.bytes(0x40)
    put(win, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
    regions[0xfaa5fc] = struct.pack('<I', r.choice([0, CDECL_LOG4]))
    regions[0x77c48c] = struct.pack('<I', MANAGER)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40)
    regions[MANAGER + 0x40] = vtable({13: 1})
    regions[VTABLE] = vtable({0: 1, 5: 1})
    for k, off in enumerate((0x98, 0x9c)):
        impl = bytearray(r.bytes(0x20))
        put(impl, 0, 'I', VTABLE)
        edge = r.random() < 0.05   # both counts -0x8000 after the release
        put(impl, 0x10, 'H', 0x8000 if edge else r.choice([0, 1, 1, 2, 0xffff]))
        put(impl, 0x12, 'H', 0x8001 if edge else r.choice([0, 1, 1, 2, 3]))
        regions[MISC + 0x40 * k] = impl
        put(win, off, 'I', r.choice([MISC + 0x40 * k, MISC + 0x40 * k, 0]))
    for i in range(4):
        put(win, 0x120 + 4 * i, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    nodes = []   # the timer list: the window's ids and others, linked both ways
    for i in range(r.choice([0, 1, 3, 5])):
        node = bytearray(r.bytes(0x20))
        put(node, 0, 'I', r.choice([WINDOW2, WINDOW2, wins[0]]))
        put(node, 0xc, 'I', r.choice([0x80080, 0x80081, 0x80082, 0x80083, 0x10]))
        put(node, 0x18, 'I', 0)
        put(node, 0x1c, 'I', TNODES + 0x20 * (i - 1) if i else 0)
        if i:
            put(nodes[-1], 0x18, 'I', TNODES + 0x20 * i)
        nodes.append(node)
    if nodes:
        regions[TNODES] = b''.join(nodes)
    regions[0xfaa634] = struct.pack('<I', TNODES if nodes and r.random() < 0.9 else 0)
    put(win, 0x44, 'I', r.choice([WINDOW2 + 8] * 3 + [RECT, 0]))
    regions[RECT] = struct.pack('<I', RECT + 0x40) + r.bytes(0xc)
    regions[RECT + 0x40] = vtable({4: 1})
    parent = r.choice([PARENT, 0])
    put(win, 0x60, 'I', parent)
    put(win, 0x58, 'I', r.choice([PARENT + 0x100, 0]))
    put(win, 0x54, 'I', r.choice([PARENT + 0x200, 0]))
    for k in range(3):
        regions[PARENT + 0x100 * k] = r.bytes(0x70)
    n = r.choice([0, 0, 1, 2, 3])
    kids = [CHILDREN + 0x150 * i for i in range(n)]
    put(win, 0x5c, 'I', kids[0] if n else 0)
    for i, a in enumerate(kids):
        kid = bytearray(r.bytes(0x150))
        put(kid, 0, 'I', r.choice([0x705980, 0x705a50]))
        put(kid, 0x148, 'I', r.choice([ROWS + 0x10 * (i + 1), 0]))
        kid[4] = r.choice([0, 1])
        put(kid, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
        put(kid, 0x60, 'I', WINDOW2)
        put(kid, 0x58, 'I', kids[i - 1] if i else 0)
        put(kid, 0x54, 'I', kids[i + 1] if i + 1 < n else 0)
        put(kid, 0x5c, 'I', 0)
        put(kid, 0x98, 'I', 0)
        put(kid, 0x9c, 'I', 0)
        for j in range(4):
            put(kid, 0x120 + 4 * j, 'I', 0)
        put(kid, 0x44, 'I', a + 8)
        regions[a] = kid
    regions[WINDOW2] = win
    timers(r, regions, WINDOW2)

    def some():
        return r.choice([WINDOW2, WINDOW2, wins[0], 0, r.getrandbits(32)] + kids)
    regions[0xfaa578] = struct.pack('<II', r.getrandbits(32), some())
    regions[0xfaa588] = struct.pack('<3I', some(), some(), some())
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0], 0, 0]), some(), some(), r.getrandbits(32), r.choice([0, 0, 0, some()]))
    regions[0xfaa5ec] = struct.pack('<I', some())
    if op.startswith('Dtor'):
        return Call(CALLS[op], [WINDOW2], regions, conv='t', ret='a')
    return Call(CALLS[op], [WINDOW2, r.choice([0, 1, 1, 3, r.getrandbits(32)])], regions, conv='t', ret='a')


def tree(r, regions):
    """1-7 windows (the first the root, each other a child of an earlier one), their flags +0x48 and byte +4 mostly letting them be hit,
    each with a GUI rectangle (vtable 0x705920) at +0x44 near the points used. Returns the windows."""
    n = r.choice([1, 2, 3, 4, 7])
    wins = [TREE + 0x120 * i for i in range(n)]
    bufs = [bytearray(r.bytes(0x120)) for _ in range(n)]
    for i, buf in enumerate(bufs):
        buf[4] = 0 if r.random() < 0.9 else r.choice([1, 0x80])
        put(buf, 0x48, 'I', 1 if r.random() < 0.7 else r.choice([0x10000001, 0x80000000, 0x40000000, 0, 2, 3, r.getrandbits(32)]))
        put(buf, 0x5c, 'I', 0)
        put(buf, 0x54, 'I', 0)
        put(buf, 0x44, 'I', RECTS + 0x10 * i)
        left, top = r.randrange(-40, 25), r.randrange(-40, 25)
        regions[RECTS + 0x10 * i] = struct.pack('<Ihhhh', 0x705920, left, top, left + r.choice([0, 1] + [r.randrange(10, 60)] * 14),
                                                top + r.choice([0, 1] + [r.randrange(10, 60)] * 14)) + r.bytes(4)
    last = {}
    for i in range(1, n):
        parent = r.randrange(i)
        if parent in last:
            put(bufs[last[parent]], 0x54, 'I', wins[i])
        else:
            put(bufs[parent], 0x5c, 'I', wins[i])
        last[parent] = i
    for w, buf in zip(wins, bufs):
        regions[w] = buf
    return wins


def point(r, regions, wins):
    """a point inside one of the windows' rectangles (most often), near them, or anywhere; upper halves random"""
    if r.random() < 0.6:
        left, top, right, bottom = struct.unpack_from('<hhhh', regions[RECTS + 0x10 * r.randrange(len(wins))], 4)
        x = r.choice([left, right - 1, r.randrange(left, right)]) if left < right else left
        y = r.choice([top, bottom - 1, r.randrange(top, bottom)]) if top < bottom else top
        return (x & 0xffff) | (r.getrandbits(16) << 16), (y & 0xffff) | (r.getrandbits(16) << 16)
    if r.random() < 0.1:
        return r.getrandbits(32), r.getrandbits(32)
    return (r.randrange(-45, 60) & 0xffff) | (r.getrandbits(16) << 16), (r.randrange(-45, 60) & 0xffff) | (r.getrandbits(16) << 16)


OBJECT98 = 0x31230000


def shown_env(r, regions, log):
    """For Window_SetShown (0x65d9dd): UpdateHover's tree, some of its windows moved to addresses with other upper halves, with parents +0x60, flag 0
    of +0x48 either way, objects +0x98 / +0x9c (recorders; the first at another upper half too) and handlers set or not, the queues; the manager a
    recorder (its result now and then set). Returns the windows."""
    wins = tree(r, regions)
    moved = [w if i == 0 or r.random() < 0.5 else 0x32000000 + 0x10000 * r.randrange(1, 0x100) + 0x120 * i for i, w in enumerate(wins)]
    where = dict(zip(wins, moved))
    bufs = [regions.pop(w) for w in wins]
    for w, buf in zip(moved, bufs):
        for off in (0x54, 0x5c):
            v = struct.unpack_from('<I', buf, off)[0]
            put(buf, off, 'I', where.get(v, v))
        regions[w] = buf
    wins = moved
    queues(r, regions)
    x, y = point(r, regions, wins)
    regions[0xfaa540] = struct.pack('<II', x, y)
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0]] * 6 + [0]), r.getrandbits(32), r.choice(wins + [0, 0]),
                                    r.getrandbits(32), r.choice([0] * 6 + [1, r.getrandbits(32)]))
    for w in wins:
        put(regions[w], 0x60, 'I', 0)
    for w in wins:
        c = struct.unpack_from('<I', regions[w], 0x5c)[0]
        while c:
            put(regions[c], 0x60, 'I', w)
            c = struct.unpack_from('<I', regions[c], 0x54)[0]
    for w in wins:
        buf = regions[w]
        buf[0x48] = (buf[0x48] & 0xfe) | r.choice([0, 1, 1])
        put(buf, 0x98, 'I', r.choice([OBJECT98, OBJECT98, 0]))
        put(buf, 0x9c, 'I', r.choice([MISC + 0x40, MISC + 0x40, 0]))
        put(buf, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
    regions[OBJECT98] = struct.pack('<I', VTABLE)
    regions[VTABLE] = vtable({5: 1})
    regions[MISC + 0x40] = struct.pack('<I', MISC + 0x80)
    regions[MISC + 0x80] = put(vtable({}, size=8), 0x14, 'I', recorder(21, 1))
    regions[0x77c48c] = struct.pack('<I', MANAGER)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40)
    regions[MANAGER + 0x40] = vtable({13: 1})
    if r.random() < 0.5:
        set_return(log, 13, r.choice([0, 1, -1, r.getrandbits(32)]))
    return wins


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    if op == 'Global7cc698_Choose':   # 0x7cc8b8 set or not, the byte 0x7cc8b4 set or not, any windows
        g = bytearray(r.bytes(0x40))
        put(g, 0x28, 'I', r.choice([0, 0, 7, 0x80000000]))  # 0x7cc8b8 (region base 0x7cc890)
        g[0x24] = r.choice([0, 0, 1, 0xff])  # 0x7cc8b4
        return Call(CALLS[op], [], {0x7cc890: g, 0x7cc690: r.bytes(0x20)}, ret='a')
    if op in ('ImplSlot7', 'ImplSlot6', 'DwordA0'):   # +0x9c a recorder object or 0; +0xa0 sixteen dwords (an index reads before them too)
        win = bytearray(r.bytes(0x120))
        put(win, 0x9c, 'I', r.choice([MISC, MISC, 0]))
        regions[WINDOW] = win
        regions[MISC] = struct.pack('<I', VTABLE)
        for k in (6, 7):
            if r.random() < 0.5:
                set_return(log, k, r.choice([0, 1, r.getrandbits(32)]))
        regions[VTABLE] = vtable({7: 2})
        if op == 'ImplSlot7':
            return Call(CALLS[op], [WINDOW, r.getrandbits(32), r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')
        if op == 'ImplSlot6':
            return Call(CALLS[op], [WINDOW], regions, conv='t', ret='a')
        index = r.choice([0, 1, 7, 15, 16, 17, 0x7fff, 0xffff, 0xffe8, 0x10005, 0xfffe000f, r.getrandbits(32) & 0xffff000f, r.getrandbits(32)])
        return Call(CALLS[op], [WINDOW, index], regions, conv='t', ret='a')
    if op == 'ImplRelease':   # the object: a recorder vtable (slot 5 takes one argument), its words +0x10 and +0x12 small or edges
        impl = bytearray(r.bytes(0x20))
        put(impl, 0, 'I', VTABLE)
        for off in (0x10, 0x12):
            put(impl, off, 'H', r.choice([0, 1, 2, 3, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]))
        regions[MISC] = impl
        return Call(CALLS[op], [MISC], regions, conv='t', ret='a')
    if op == 'IsShown':   # a chain of 0-4 windows by +0x60, flag 0 of +0x48 mostly set (other bits random)
        n = r.randrange(0, 5)
        for i in range(n):
            w = bytearray(r.bytes(0x70))
            put(w, 0x48, 'I', r.getrandbits(32) | (1 if r.random() < 0.8 else 0) if r.random() < 0.9 else r.getrandbits(32) & ~1)
            put(w, 0x60, 'I', TREE + 0x80 * (i + 1) if i + 1 < n else 0)
            regions[TREE + 0x80 * i] = w
        return Call(CALLS[op], [TREE if n else 0], regions, conv='t', ret='w')
    if op in ('GetRect', 'SetSavedRect'):   # the rectangle a GUI one, a recorder (slot 3: four) or 0; insets small or edges; inner's low word 0 or not
        win = bytearray(r.bytes(0x120))
        for k in range(0xf8, 0x100, 2):
            put(win, k, 'H', r.choice([0, 1, 2, 5, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]))
        regions[WINDOW] = win
        rect = bytearray(r.bytes(0x10))
        put(rect, 0, 'I', r.choice([0x705920, 0x705920, VTABLE]))
        regions[MISC] = rect
        regions[VTABLE] = vtable({3: 4})
        inner = r.choice([0, 1, 0x10000, 0xffff0000, r.getrandbits(32)])
        args = [r.choice([MISC] * 6 + [0])] + ([inner] if op == 'GetRect' else [])
        return Call(CALLS[op], [WINDOW] + args, regions, conv='t', ret='w')
    if op == 'SaveInsetRect':   # the window's rectangles (+8 and the saved +0x2c) and insets random (small or edges), the insets dx / dy too
        win = bytearray(r.bytes(0x120))
        for k in list(range(0xc, 0x14, 2)) + list(range(0xf8, 0x100, 2)):
            put(win, k, 'H', r.choice([0, 1, 2, 5, 0x7fff, 0x8000, 0xffff, r.randrange(-50, 50) & 0xffff, r.getrandbits(16)]))
        regions[WINDOW] = win
        return Call(CALLS[op], [WINDOW] + [r.choice([0, 1, 3, -2, 0x7fff, -0x8000, 0x12345, r.getrandbits(32)]) & 0xffffffff for _ in range(2)], regions, ret='v')
    if op in ('RectAssign', 'SaveRect'):   # random rectangles (the window's at +8 and +0x2c)
        regions[MISC] = r.bytes(0x10)
        regions[MISC + 0x40] = r.bytes(0x10)
        regions[WINDOW] = r.bytes(0x40)
        if op == 'SaveRect':
            return Call(CALLS[op], [WINDOW], regions, conv='t', ret='a')
        return Call(CALLS[op], [MISC, MISC + 0x40], regions, conv='t', ret='a')
    if op == 'OffsetByParents':   # 0-4 parents by +0x60, some at addresses with other upper halves; the rectangle a recorder (slot 2: two)
        n = r.randrange(0, 5)
        parents = [r.choice([TREE + 0x80 * i, 0x31000000 + 0x10000 * r.randrange(1, 0x40) + 0x80 * i]) for i in range(n)]
        win = bytearray(r.bytes(0x70))
        put(win, 0x60, 'I', parents[0] if n else 0)
        regions[WINDOW] = win
        for i, a in enumerate(parents):
            w = bytearray(r.bytes(0x70))
            put(w, 0x60, 'I', parents[i + 1] if i + 1 < n else 0)
            regions[a] = w
        regions[MISC] = struct.pack('<I', VTABLE) + r.bytes(0xc)
        regions[VTABLE] = vtable({2: 2})
        return Call(CALLS[op], [WINDOW, MISC], regions, conv='t', ret='a')
    if op == 'Ctor':
        wins = tree(r, regions)
        for i, w in enumerate(wins):
            put(regions[w], 0x60, 'I', r.choice([wins[i - 1], 0]) if i else 0)
            put(regions[w], 0xe0, 'H', r.choice([0, 1, 0xfe, 0xff, 0x100, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]))
        regions[WINDOW2] = r.bytes(0x140)
        queues(r, regions)
        words = []
        for _ in range(4):
            w = r.choice([r.randrange(-40, 60) & 0xffff] * 4 + [0, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)])
            words.append(w | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0))
        x, y = point(r, regions, wins)
        if r.random() < 0.4:
            x, y = words[0] + r.randrange(-2, 30), words[1] + r.randrange(-2, 30)
        regions[0xfaa540] = struct.pack('<II', x & 0xffffffff, y & 0xffffffff)
        regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0], WINDOW2, WINDOW2, 0]), r.getrandbits(32), r.choice(wins + [0, 0]),
                                        r.getrandbits(32), r.choice([0] * 6 + [1]))
        flags = r.choice([1, 0x401, 0x400, 0, 0x10000001, r.getrandbits(32)]) | (r.getrandbits(32) & 0x0ffff800 if r.random() < 0.3 else 0)
        return Call(CALLS[op], [WINDOW2, r.choice(wins + [0, 0]), flags, r.getrandbits(32)] + words, regions, conv='t', ret='a')
    if op in ('HilightList_NewRow', 'List7cc188_NewRow'):   # the Ctor's surroundings for a window from the heap (or none), font 7's height
        wins = tree(r, regions)
        queues(r, regions)
        regions[HEAP] = heap(0x400, r.choice([0] * 5 + [1]))
        regions[0xfaa540] = struct.pack('<II', r.randrange(-10, 120) & 0xffffffff, r.randrange(-10, 40) & 0xffffffff)
        regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0], 0]), r.getrandbits(32), r.choice(wins + [0, 0]), r.getrandbits(32),
                                        r.choice([0] * 6 + [1]))
        regions[0xfaa5c0] = struct.pack('<H', r.choice([1, 2, 3, 0x600, 0x7fff, 0x8000, 0xffff, 0xfffd, r.randrange(1, 0x10000)]))
        lang = bytearray(r.bytes(0x400))
        group = r.choice([0, 1, 2])
        put(lang, 4, 'I', group)
        put(lang, 0x1c + (7 + 13 * group) * 0x18, 'I', VTABLE)
        regions[0x78b530] = lang
        regions[VTABLE] = vtable({})
        set_return(log, 1, r.choice([r.randrange(0, 40), 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]) | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0))
        return Call(CALLS[op], [], regions, ret='a')
    if op in ('ForgetGlobals', 'DropModal', 'Destroying'):
        wins = tree(r, regions) if op == 'Destroying' else [WINDOW]
        if op != 'Destroying':
            regions[WINDOW] = r.bytes(0x20)
        else:
            for w in wins:
                put(regions[w], 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
        regions[0xfaa5fc] = struct.pack('<I', r.choice([0, CDECL_LOG4]))

        def some():
            return r.choice(wins + [wins[0], 0, r.getrandbits(32), (wins[0] & 0xffff) | 0x12340000])
        regions[0xfaa578] = struct.pack('<II', r.getrandbits(32), some())
        regions[0xfaa588] = struct.pack('<3I', some(), some(), some())
        regions[0xfaa5dc] = struct.pack('<4I', some(), some(), r.getrandbits(32), some())
        regions[0xfaa5ec] = struct.pack('<I', some())
        if op == 'ForgetGlobals':
            return Call(CALLS[op], [r.choice([WINDOW] * 5 + [0])], regions, ret='a')
        return Call(CALLS[op], [r.choice(wins[:1] * 3 + wins)], regions, conv='t', ret='a')
    if op == 'ResetQueues':   # the queues and the timed table as for the cancels
        timers(r, regions, WINDOW)
        return Call(CALLS[op], [], regions, ret='v')
    if op.startswith('Cancel'):
        timers(r, regions, WINDOW)
        return Call(CALLS[op], [r.choice([WINDOW] * 5 + [0])], regions, ret='w')
    if op in ('Dtor', 'DeletingDtor', 'Dtor705a50', 'DeletingDtor705a50'):
        return destructor(r, regions, op)
    if op == 'HitTest':
        wins = tree(r, regions)
        return Call(CALLS[op], [r.choice(wins[:1] * 3 + wins)] + list(point(r, regions, wins)), regions, conv='t', ret='a')
    if op in ('MouseDown', 'MouseUp'):
        # the GUI up ([0x77c488]) or not, the hovered window and the buttons' windows (0xfaa588) the window or none, the button bits
        # [0xfaa5bc]; buttons 0-7 (their high words random), x and y anything
        win = bytearray(r.bytes(0x120))
        win[4] = r.choice([0, 0, 1])
        regions[WINDOW] = win
        queues(r, regions)
        regions[0x77c488] = struct.pack('<I', r.choice([1, 1, 1, 0, 0x100]))
        regions[0xfaa588] = struct.pack('<8I', *[r.choice([WINDOW, 0]) for _ in range(8)])
        regions[0xfaa5bc] = r.bytes(2)
        regions[0xfaa5e0] = struct.pack('<I', r.choice([WINDOW, WINDOW, 0]))
        button = r.choice([0, 1, 2, 2, 7]) | r.choice([0, 0, 0x10000, 0xffff0000])
        return Call(CALLS[op], [button, r.getrandbits(32), r.choice([0, 5, 0x8000, r.getrandbits(32)])], regions, ret='w')
    if op in ('SetFocus', 'KillFocus', 'Windows7cc8bc_SelectSecond'):   # the hover as UpdateHover's; the focus [0xfaa5ec] the window, another of the tree or none
        wins = tree(r, regions)
        queues(r, regions)
        x, y = point(r, regions, wins)
        regions[0xfaa540] = struct.pack('<II', x, y)
        regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0]] * 6 + [0]), r.getrandbits(32), r.choice(wins + [0, 0]),
                                        r.getrandbits(32), r.choice([0] * 6 + [1, r.getrandbits(32)]))
        window = r.choice(wins)
        regions[0xfaa5ec] = struct.pack('<I', r.choice([window, window, 0] + wins))
        if op == 'Windows7cc8bc_SelectSecond':   # the two windows of the tree (now and then none), their handlers fncmp's logger or none
            for w in wins:
                put(regions[w], 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
            regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
            regions[0x7cc8bc] = struct.pack('<II', r.choice(wins + [0]), r.choice(wins * 4 + [0]))
            regions[0x7cc6a0] = r.bytes(4)
            if r.random() < 0.5:
                set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
            return Call(CALLS[op], [], regions, ret='a')
        return Call(CALLS[op], [window, r.getrandbits(32)], regions, conv='t', ret='w')
    if op in ('NotifyShown', 'SetShown'):   # see shown_env(); EDX the window's upper half or random
        wins = shown_env(r, regions, log)
        window = r.choice(wins)
        shown = r.choice([0, 0, 1, 1, 2, 0x10000, 0xffff0000, 0x8000, r.getrandbits(32)])
        if op == 'NotifyShown':
            edx = r.choice([window & 0xffff0000, r.getrandbits(32)])
            return Call(CALLS[op], [window, edx, shown], regions, conv='f', ret='v')
        return Call(CALLS[op], [window, shown], regions, conv='t', ret='a')
    if op == 'UpdateHover':   # the root [0xfaa5d8] or none, [0xfaa5e8] mostly 0, the hovered window [0xfaa5e0] one of them or none
        wins = tree(r, regions)
        queues(r, regions)
        x, y = point(r, regions, wins)
        regions[0xfaa540] = struct.pack('<II', x, y)
        regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0]] * 6 + [0]), r.getrandbits(32), r.choice(wins + [0, 0]),
                                        r.getrandbits(32), r.choice([0] * 6 + [1, r.getrandbits(32)]))
        return Call(CALLS[op], [], regions, ret='w')
    if op == 'Dialog7cb2b8_Child21Post':   # the dialog [0x7cb2b8] or none; its child 0x21 among 0-2 others (or missing), its dword +0xa8 0 or not
        regions[0xfaa5fc] = struct.pack('<I', r.choice([0, CDECL_LOG4]))
        queues(r, regions)
        ids = [r.choice([0x20, 0x22, 1]) for _ in range(r.choice([0, 1, 2]))]
        if r.random() < 0.85:
            ids.insert(r.randrange(len(ids) + 1), 0x21)
        for i, ident in enumerate(ids):
            kid = bytearray(r.bytes(0x120))
            kid[4] = r.choice([0, 0, 0, 1])
            put(kid, 0x54, 'I', KIDS + 0x120 * (i + 1) if i + 1 < len(ids) else 0)
            put(kid, 0x8c, 'I', ident)
            put(kid, 0xa8, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
            put(kid, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
            regions[KIDS + 0x120 * i] = kid
        dialog = bytearray(r.bytes(0x120))
        put(dialog, 0x5c, 'I', KIDS if ids else 0)
        regions[WINDOW2] = dialog
        regions[0x7cb2b8] = struct.pack('<I', r.choice([WINDOW2] * 6 + [0]))
        return Call(CALLS[op], [r.choice([0, 1, r.getrandbits(32), r.getrandbits(32)])], regions, ret='a')
    if op == 'PostMessage':   # the queues [0xfaa5f4] / [0xfaa5f0] (as msgqueue.py) or 0 (for 0x1000a-c too: a fault in both)
        win = bytearray(r.bytes(0x120))
        win[4] = r.choice([0, 0, 0, 1, 0x80])
        regions[WINDOW] = win
        queues(r, regions)
        message = r.choice([1, 2, 3, 0x1000a, 0x1000b, 0x1000c, 0, 4, 0x10001, 0x1000d, r.getrandbits(32)])
        return Call(CALLS[op], [r.choice([WINDOW] * 6 + [0]), message, r.getrandbits(32), r.getrandbits(32)], regions, ret='a')
    if op in ('CallDefault', 'WindowAt10_Handler', 'Dialog7cb2b8_Handler', 'DefaultHandler'):   # the default handler +0x118 fncmp's logger (its result now and then set); the object A0[0]
        win = bytearray(r.bytes(0x120))
        put(win, 0x118, 'I', CDECL_LOG4)
        put(win, 0xa0, 'I', MISC)
        regions[WINDOW] = win
        regions[MISC] = r.bytes(0x20)
        regions[0x7cb2b8] = r.bytes(4)
        if r.random() < 0.5:
            set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
        args = [r.choice([0x14, 0x14, 0x15, 0, 4, r.getrandbits(32)])] + [r.choice([0, 1, 4, r.getrandbits(32)]) for _ in range(2)]
        if op == 'CallDefault':
            return Call(CALLS[op], [WINDOW] + args, regions, conv='t', ret='a')
        return Call(CALLS[op], [WINDOW] + args, regions, ret='a')
    if op in ('CallHandler', 'SendMessage') or op in SENDS or op.startswith('Window'):
        win = bytearray(r.bytes(0x120))
        win[4] = r.choice([0, 0, 0, 1, 0x80])
        put(win, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, CDECL_LOG4, 0]))
        regions[WINDOW] = win
        regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
        if r.random() < 0.5:
            set_return(log, 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
        window = r.choice([WINDOW] * 6 + [0])
        args = [r.choice([0, 1, 4, 6, r.getrandbits(32)]) for _ in range(3)]
        if op == 'CallHandler':
            return Call(CALLS[op], [WINDOW] + args, regions, conv='t', ret='a')
        if op == 'SendMessage':
            return Call(CALLS[op], [window] + args, regions, ret='a')
        if op.startswith('WindowAt10'):
            holder = bytearray(r.bytes(0x20))
            put(holder, 0x10, 'I', window)
            regions[HOLDER] = holder
            if op == 'WindowAt10_ScalarDtor':   # (freed for flags bit 0: the heap's free count and hash)
                regions[HEAP] = heap(0x20)
                return Call(CALLS[op], [HOLDER, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
            return Call(CALLS[op], [HOLDER], regions, conv='t', ret='a')
        if op == 'Windows7c26b0_Message5':   # ten entries of 0x208: +0 the window, +4 a word (0: skipped)
            other = bytearray(r.bytes(0x120))
            other[4] = r.choice([0, 0, 1])
            put(other, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
            regions[WINDOW2] = other
            for i in range(10):
                regions[0x7c26b0 + 0x208 * i] = struct.pack('<IH', r.choice([WINDOW, WINDOW2, 0]), r.choice([0, 0, 1, 0x100, 0xffff]))
            return Call(CALLS[op], [], regions)
        if op == 'Windows7cc8bc_Message6Pair':   # (0x7cc698 the first window or not, then cleared)
            other = bytearray(r.bytes(0x120))
            other[4] = r.choice([0, 0, 1])
            put(other, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
            regions[WINDOW2] = other
            first = r.choice([WINDOW, WINDOW2, 0])
            regions[0x7cc8bc] = struct.pack('<II', first, r.choice([WINDOW, WINDOW2, 0]))
            regions[0x7cc698] = struct.pack('<I', r.choice([first, first, WINDOW, WINDOW2, 0, r.getrandbits(32)]))
            return Call(CALLS[op], [], regions, ret='a')
        if op == 'Dialog7ca098_Message4IfUp':   # (the dword 0x7ca090 is cleared)
            regions[0x7ca090] = r.bytes(8) + struct.pack('<I', window)
            return Call(CALLS[op], [], regions, ret='a')
        if 'Child' in op:   # the dialog's child (from 0-3 others, now and then missing: the send gets no window) sent to instead
            want = int(op.split('Child')[1][:5], 16)
            ids = [r.choice([want - 1, want + 1, 1]) for _ in range(r.choice([0, 1, 2]))]
            if r.random() < 0.9:
                ids.insert(r.randrange(len(ids) + 1), want)
            for i, ident in enumerate(ids):
                kid = bytearray(r.bytes(0x120))
                kid[4] = r.choice([0, 0, 0, 1])
                put(kid, 0x54, 'I', KIDS + 0x120 * (i + 1) if i + 1 < len(ids) else 0)
                put(kid, 0x8c, 'I', ident)
                put(kid, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
                regions[KIDS + 0x120 * i] = kid
            put(win, 0x5c, 'I', KIDS if ids else 0)
            window = WINDOW
        regions[SENDS[op]] = struct.pack('<I', window)
        return Call(CALLS[op], [], regions, ret='a')
    if op == 'FindChild':
        ids = [r.choice([0, 1, 5, 0x18, 0x1f, 0x100, r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2, 3, 6]))]
        want = r.choice(ids + [r.choice([0, 5, 0x18, r.getrandbits(32)])]) if ids else r.choice([0, 5])
        win = bytearray(r.bytes(0x90))
        put(win, 0x5c, 'I', chain(r, regions, ids, NODES))
        regions[WINDOW] = win
        return Call(CALLS[op], [WINDOW, want], regions, conv='t', ret='a')
    struct.pack_into('<I', log, LOG_BYTES + 4 * 5, 1)
    buttons = list(BUTTONS)
    if r.random() < 0.05:
        buttons.remove(r.choice(BUTTONS[1:]))
    buttons += [r.choice([0, 7, 0x22, 0x1000]) for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(buttons)
    panel_at = NODES + NODE * 12
    head = chain(r, regions, buttons, panel_at)
    outer = [r.choice([1, 0x17, 0x22]) for _ in range(r.choice([0, 1, 2]))]
    pos = r.randrange(len(outer) + 1)
    outer.insert(pos, 0x18)
    first = chain(r, regions, outer, NODES, {pos: head})
    win = bytearray(r.bytes(0x90))
    put(win, 0x5c, 'I', first)
    regions[WINDOW] = win
    misc = bytearray(r.bytes(0x60))
    misc[0x50] = r.choice([0, 0, 1, 2, 0xff])
    regions[MISC] = misc
    regions[0x7ca1d0] = struct.pack('<I', MISC)
    regions[0x7ca1c8] = struct.pack('<II', WINDOW if op == 'SetChecks' or r.random() < 0.8 else 0, r.choice([0, 1, 1, 1, 0x100]))
    flags = r.choice([0, 1, 2, 3, 0x100, 0x1ff, 0xffff, r.getrandbits(9), r.getrandbits(32)])
    if op == 'SetChecksIfUpThis':
        return Call(CALLS[op], [r.getrandbits(32), flags], regions, conv='t')
    return Call(CALLS[op], [flags], regions)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

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
# them or anywhere (upper halves random), the root, the hovered window and the queues set or not.
# usage: window.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FindChild,SetChecks,SetChecksIfUp)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, HEAP, LOG, LOG_BYTES, Bench, Call, RAM, heap, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'FindChild': 0x65d7d0, 'SetChecks': 0x498090, 'SetChecksIfUp': 0x498a50, 'CallHandler': 0x65f697, 'SendMessage': 0x658d9f,
         'Dialog7cc47c_Message4': 0x4b87f0, 'Dialog7cc47c_Message6On': 0x4b8810, 'Dialog7cc47c_Message6Off': 0x4b8830,
         'Dialog7cc4b4_Message4': 0x4b8e80, 'Dialog7cc4b4_Message6Off': 0x4b8ec0, 'Dialog7cc4c4_Message4': 0x4b9c00, 'Dialog7cc4c4_Message6On': 0x4b9c20,
         'Dialog7cdb0c_Message4': 0x4c5780, 'Dialog7cdb0c_Message6On': 0x4c57a0, 'Dialog7cdb0c_Message6Off': 0x4c57c0,
         'Dialog7cdb0c_Child16007Message6On': 0x4c57e0, 'Dialog7cdb0c_Child16007Message6Off': 0x4c5800,
         'Dialog7ca098_Message4IfUp': 0x48d990, 'Dialog7cca94_Message4IfUp': 0x4c25e0, 'WindowAt10_Message4': 0x48cf70,
         'WindowAt10_Message4IfAny': 0x492e60, 'Windows7c26b0_Message5': 0x48ee20,
         'WindowAt10_ScalarDtor': 0x48c8b0, 'SetChecksIfUpThis': 0x481a00,
         'Windows7cc8bc_Message6Pair': 0x4bcef0, 'ImplSlot7': 0x65d6db, 'ImplSlot6': 0x65d745, 'DwordA0': 0x65e705,
         'PostMessage': 0x658ccc, 'Dialog7cb2b8_Child21Post': 0x4a2590,
         'HitTest': 0x65db25, 'UpdateHover': 0x6589f9, 'ImplRelease': 0x6616b0, 'IsShown': 0x65d773}
SENDS = {k: int(k[6:12], 16) for k in CALLS if k.startswith('Dialog7')}   # the window's global
B = Bench([(0x498090, 0x4981e0), (0x498a50, 0x498a80), (0x65d7bc, 0x65d832), (0x65f697, 0x65f6d1), (0x658d9f, 0x658df4), (0x659710, 0x659721),
           (0x4b87f0, 0x4b8850), (0x4b8e80, 0x4b8ea0), (0x4b8ec0, 0x4b8ee0), (0x4b9c00, 0x4b9c40),
           (0x4c5780, 0x4c5820),
           (0x48d990, 0x48d9b3), (0x4c25e0, 0x4c25f9), (0x48cf70, 0x48cf83), (0x492e60, 0x492e77), (0x48ee20, 0x48ee4e),
           (0x48c8b0, 0x48c8ce), (0x481a00, 0x481a10),
           (0x4bcef0, 0x4bcf60), (0x65d6db, 0x65d727), (0x65d745, 0x65d783), (0x65e705, 0x65e73d),
           (0x658ccc, 0x658d9f), (0x6697e3, 0x6698a4), (0x4a2590, 0x4a25e5),
           (0x65db25, 0x65dbee), (0x6589f9, 0x658aab), (0x659e83, 0x659ee2),
           (0x6616b0, 0x6616f1), (0x65d773, 0x65d7bc)])
WINDOW, NODES, VTABLE, MISC, KIDS, HOLDER, WINDOW2 = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x1100, RAM + 0x1200, RAM + 0x1600, RAM + 0x1800
QUEUES, TREE, RECTS = RAM + 0x2000, RAM + 0x3000, RAM + 0x4000
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


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
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
    if op == 'HitTest':
        wins = tree(r, regions)
        return Call(CALLS[op], [r.choice(wins[:1] * 3 + wins)] + list(point(r, regions, wins)), regions, conv='t', ret='a')
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

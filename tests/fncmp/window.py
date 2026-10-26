# The GUI window benches' shared set-up (no cases of its own: they are in windowtree.py, windownotify.py, windowrect.py, windowlife.py,
# windowmessages.py, focus.py, dialogs.py, checkbuttons.py and listrows.py). RANGES: the listing every one of them (and the benches importing
# this module) runs on. node() / chain(): child lists by +0x5c / +0x54 with ids +0x8c. queues() / timers(): the message queues [0xfaa5f4] /
# [0xfaa5f0] (as msgqueue.py: 1-4 entries, free or not) or none, and the timed table [0xfaa5f8] (0-4 entries, now and then the window's) or 0.
# tree() / point(): 1-7 windows with real GUI rectangles (the driver resolves vtable 0x705920 to the ports) and points near them or anywhere
# (upper halves random). shown_env(): that tree with parents, shown or hidden, recorder objects +0x98 / +0x9c, handlers, the manager.
# offset_env(): those windows given GUI rectangles. notify_env() / notify_blobs(): +0x4c look objects or none, the objects' events hashed.
# word(): a word for a move.
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG_BLOBS, RAM, blob, blob_bytes, blob_when_inner, put, recorder, set_return, vtable  # noqa: E402

RANGES = [(0x498090, 0x4981e0), (0x498a50, 0x498a80), (0x65d7bc, 0x65d832), (0x65f697, 0x65f6d1), (0x658d9f, 0x658df4), (0x659710, 0x659721),
          (0x4b87f0, 0x4b8850), (0x4b8e80, 0x4b8ea0), (0x4b8ec0, 0x4b8ee0), (0x4b9c00, 0x4b9c40),
          (0x4c5780, 0x4c5820),
          (0x48d990, 0x48d9b3), (0x4c25e0, 0x4c25f9), (0x48cf70, 0x48cf83), (0x492e60, 0x492e77), (0x48ee20, 0x48ee4e),
          (0x48c8b0, 0x48c8ce), (0x481a00, 0x481a10),
          (0x4bcef0, 0x4bcf60), (0x4bcec0, 0x4bcef0), (0x4bce80, 0x4bcec0), (0x65d90b, 0x65d9dd), (0x65d9dd, 0x65da8d), (0x65c7fe, 0x65c979), (0x65c388, 0x65c428), (0x65c428, 0x65c4b9), (0x65c4b9, 0x65c54a), (0x65c54a, 0x65c5d5), (0x65d1c8, 0x65d323), (0x6617f0, 0x661804), (0x65d6db, 0x65d727), (0x65d745, 0x65d783), (0x65e705, 0x65e73d),
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
          (0x65e59b, 0x65e5fa), (0x65e5fa, 0x65e640), (0x498a80, 0x498a8e), (0x658af1, 0x658b5b), (0x658b5b, 0x658bcb),
          (0x4862a0, 0x486404), (0x48c8d0, 0x48c8ec), (0x485b40, 0x485b70), (0x4a27c0, 0x4a27e7), (0x6ad810, 0x6ad814), (0x65d57c, 0x65d58d), (0x40cb40, 0x40cb47)]
WINDOW, NODES, VTABLE, MISC, KIDS, HOLDER, WINDOW2 = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x1100, RAM + 0x1200, RAM + 0x1600, RAM + 0x1800
QUEUES, TREE, RECTS, TIMED = RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x5000
MANAGER, TNODES, RECT, ROWS, PARENT, CHILDREN = RAM + 0x5400, RAM + 0x5600, RAM + 0x5800, RAM + 0x5c00, RAM + 0x6000, RAM + 0x7000
FOCUS = RAM + 0x8000   # (FocusRestore's: the source at 0x7c2534, the frame at 0xf82884, the key maps)
NODE = 0x90


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


def hover(r, regions, wins):
    """UpdateHover's surroundings for the tree: the queues, the mouse [0xfaa540] a point(), the root [0xfaa5d8] (or none), [0xfaa5e8]
    mostly 0 and the hovered window [0xfaa5e0] one of them or none"""
    queues(r, regions)
    x, y = point(r, regions, wins)
    regions[0xfaa540] = struct.pack('<II', x, y)
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0]] * 6 + [0]), r.getrandbits(32), r.choice(wins + [0, 0]),
                                    r.getrandbits(32), r.choice([0] * 6 + [1, r.getrandbits(32)]))


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


def offset_env(r, regions, log):
    """For Window_Offset (0x65c7fe): shown_env's windows with GUI rectangles (vtable 0x705920) at +8 and +0x2c, +0x44 now and then +8, the
    objects' slot 3 recorders (their event's set dwords, +0xc and +0x14, hashed), the hook [0xfaa5fc] set or not. Returns the windows."""
    wins = shown_env(r, regions, log)
    for w in wins:
        put(regions[w], 8, 'I', 0x705920)
        put(regions[w], 0x2c, 'I', 0x705920)
        if r.random() < 0.5:
            put(regions[w], 0x44, 'I', w + 8)
    regions[VTABLE] = vtable({5: 1, 3: 2})
    put(regions[MISC + 0x80], 0xc, 'I', recorder(22, 2))
    blob(log, 3, 1, 0x28)
    blob(log, 22, 1, 0x28)
    regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
    return wins


LOOK, IMPL = 0x31240000, 0x31250000


def notify_env(r, regions, log):
    """For the GUI's notifications (Window_Set4c / NotifyC00 / SetImpl): shown_env's windows with +0x4c one of two look objects (their byte
    +0x10 any) or none, the objects' slot 3 recorders (see notify_blobs()), +0x98's slot 0 a recorder of one argument, the object +0x9c
    with its words +0x10 / +0x12 small (WindowImpl_Release answers -1 now and then) and another (IMPL) like it. Returns the windows."""
    wins = shown_env(r, regions, log)
    for w in wins:
        put(regions[w], 0x4c, 'I', r.choice([LOOK, LOOK + 0x40, 0]))
    regions[LOOK] = r.bytes(0x20)
    regions[LOOK + 0x40] = r.bytes(0x20)
    regions[VTABLE] = vtable({5: 1, 3: 2, 0: 1})
    put(regions[MISC + 0x80], 0xc, 'I', recorder(22, 2))
    for impl in (MISC + 0x40, IMPL):
        regions[impl] = put(put(bytearray(r.bytes(0x20)), 0, 'I', MISC + 0x80), 0x10, 'hh', r.choice([0, 1, 2]), r.choice([0, 1, 1, 2, 3]))
    return wins


def notify_blobs(log, mask, inner):
    """the objects' slot 3 recorders log their event as a hash of its set bytes `mask` and the bytes `inner` of the second event its +0x10
    points to (none for 0: the pointer is unset then)"""
    for k in (3, 22):
        blob_bytes(log, k, 1, mask, (0x10, inner) if inner else None)


def notify_inner(log, k=3):
    """slot k's recorder (its event a blob_bytes() hash; of none of its bytes when no blob is set yet) hashing, for the GUI's notifications
    of one kind each, the bytes they set in the second event: 0xc00 (Window_NotifyC00: +4 / +5), 0x10000 (Window_Set4c: +8), 0x40000
    (Window_Notify40000: +0x10-+0x13) and 0x100 (Window_NotifyImpl100: the dword +0). Takes blob_when()'s four entries."""
    if not struct.unpack_from('<I', log, LOG_BLOBS + 8 * (k & 63))[0]:
        blob_bytes(log, k, 1, 0)
    for entry, (flags, mask, base) in enumerate(((0xc00, 3, 4), (0x10000, 1, 8), (0x40000, 0xf, 0x10), (0x100, 0xf, 0))):
        blob_when_inner(log, k, 0, flags, (0x10, mask, base), entry)


def word(r):
    """a word for a move: 0 most often, small, negative or any, the upper half random"""
    return r.choice([0, 0, 0, 1, 5, 0xffff, 0xffd8, 0x8000, r.getrandbits(16)]) | r.choice([0, r.getrandbits(16)]) << 16

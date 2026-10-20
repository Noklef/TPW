# A GUI window's tree (src/game/ui/window) against the originals. FindChild (0x65d7d0): a window with 0-6 children (ids from a few, repeats
# and misses) searched for one id. IsShown (0x65d773): a chain of 0-4 windows by +0x60. OffsetByParents (0x65c979): 0-4 parents by +0x60,
# some at addresses with other upper halves; the rectangle a recorder. HitTest / UpdateHover (0x65db25, 0x6589f9): a tree of 1-7 windows with
# real GUI rectangles (window.py's tree()), points near them or anywhere (upper halves random), the root, the hovered window and the queues set
# or not. NotifyShown / SetShown (0x65d90b, 0x65d9dd): window.py's shown_env. Offset (0x65c7fe): window.py's offset_env, moved by words 0 or
# not (upper halves random). SetEnabled (0x65da8d): shown_env's, flag 2 of each window's +0x48 either way; AX compared (only AX is set).
# ScriptStep / SlideStep / ScriptTick (0x65e9a4, 0x65ee7a, 0x65c357): see script_case().
# usage: windowtree.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FindChild,IsShown,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, blob, blob_bytes, log_region, put, recorder, set_return, vtable  # noqa: E402
from window import MANAGER, MISC, NODES, OBJECT98, RANGES, TREE, VTABLE, WINDOW, chain, hover, offset_env, point, shown_env, tree, word  # noqa: E402

CALLS = {'FindChild': 0x65d7d0, 'IsShown': 0x65d773, 'OffsetByParents': 0x65c979, 'HitTest': 0x65db25, 'UpdateHover': 0x6589f9,
         'NotifyShown': 0x65d90b, 'SetShown': 0x65d9dd, 'Offset': 0x65c7fe, 'SetEnabled': 0x65da8d,
         'RaiseToTop': 0x65dbf0, 'OpenTree': 0x65c707, 'CloseTree': 0x65c778, 'SetRect': 0x65cc04, 'ResizeChildren': 0x65c9bd,
         'SetRectTo': 0x65cf5e, 'AddChild': 0x65f447, 'ScriptStep': 0x65e9a4, 'SlideStep': 0x65ee7a, 'ScriptTick': 0x65c357,
         'DefaultProc': 0x65f6d1}
ARG = RAM + 0xb000
B = Bench(RANGES + [(0x65da8d, 0x65db25), (0x65dbf0, 0x65dcaf), (0x65c707, 0x65c7fe), (0x65c9bd, 0x65cd95),
                       (0x65cf5e, 0x65cfd4), (0x661700, 0x66176a), (0x659e3e, 0x659e83), (0x659d43, 0x659d90),
                       (0x65f447, 0x65f538), (0x65f345, 0x65f410), (0x65f16b, 0x65f246),
                       (0x65e9a4, 0x65ecb0), (0x65ee7a, 0x65ef90), (0x65f286, 0x65f330), (0x65d3a3, 0x65d40c), (0x662097, 0x6620ac),
                       (0x65968e, 0x6596a2), (0x65c357, 0x65c388),
                       (0x65f6d1, 0x65fba8), (0x680050, 0x68005b), (0x65f150, 0x65f16b), (0x6590f7, 0x6591d4), (0x65dcaf, 0x65dd18),
                       (0x65dd18, 0x65dd5f)])
SCRIPT, TNODE, CLOCK = ARG + 0x400, ARG + 0x600, ARG + 0x700


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    if op == 'FindChild':
        ids = [r.choice([0, 1, 5, 0x18, 0x1f, 0x100, r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2, 3, 6]))]
        want = r.choice(ids + [r.choice([0, 5, 0x18, r.getrandbits(32)])]) if ids else r.choice([0, 5])
        win = bytearray(r.bytes(0x90))
        put(win, 0x5c, 'I', chain(r, regions, ids, NODES))
        regions[WINDOW] = win
        return Call(CALLS[op], [WINDOW, want], regions, conv='t', ret='a')
    if op == 'IsShown':   # a chain of 0-4 windows by +0x60, flag 0 of +0x48 mostly set (other bits random)
        n = r.randrange(0, 5)
        for i in range(n):
            w = bytearray(r.bytes(0x70))
            put(w, 0x48, 'I', r.getrandbits(32) | (1 if r.random() < 0.8 else 0) if r.random() < 0.9 else r.getrandbits(32) & ~1)
            put(w, 0x60, 'I', TREE + 0x80 * (i + 1) if i + 1 < n else 0)
            regions[TREE + 0x80 * i] = w
        return Call(CALLS[op], [TREE if n else 0], regions, conv='t', ret='w')
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
    if op == 'HitTest':
        wins = tree(r, regions)
        return Call(CALLS[op], [r.choice(wins[:1] * 3 + wins)] + list(point(r, regions, wins)), regions, conv='t', ret='a')
    if op == 'UpdateHover':   # the root [0xfaa5d8] or none, [0xfaa5e8] mostly 0, the hovered window [0xfaa5e0] one of them or none
        hover(r, regions, tree(r, regions))
        return Call(CALLS[op], [], regions, ret='w')
    if op in ('ScriptStep', 'SlideStep', 'ScriptTick'):
        return script_case(r, op, regions, log)
    if op == 'DefaultProc':
        return proc_case(r, regions, log)
    if op in ('SetRect', 'ResizeChildren', 'SetRectTo'):   # see rect_env()
        wins = rect_env(r, regions, log)
        window = r.choice(wins[:1] * 3 + wins)
        if op == 'ResizeChildren':
            return Call(CALLS[op], [window] + [word(r) for _ in range(4)], regions, conv='t', ret='v')
        left, top, right, bottom = struct.unpack_from('<hhhh', regions[window], 0xc)
        how = r.choice(['same', 'moved', 'moved', 'sized', 'sized', 'any'])
        if how == 'moved':
            dx, dy = r.choice([0, 3, -7]), r.choice([0, 2, -5])
            left, top, right, bottom = left + dx, top + dy, right + dx, bottom + dy
        elif how == 'sized':
            left, top, right, bottom = [v + r.choice([0, 0, 1, -2, 6]) for v in (left, top, right, bottom)]
        elif how == 'any':
            left, top, right, bottom = [r.randrange(-0x8000, 0x8000) for _ in range(4)]
        if op == 'SetRectTo':
            return Call(CALLS[op], [window] + [(v & 0xffff) | r.choice([0, r.getrandbits(16)]) << 16 for v in (left, top, right, bottom)],
                        regions, conv='t', ret='w')
        regions[ARG] = struct.pack('<Ihhhh', 0x705920, left, top, right, bottom)
        return Call(CALLS[op], [window, r.choice([ARG] * 9 + [0])], regions, conv='t', ret='w')
    if op == 'Offset':   # see offset_env()
        wins = offset_env(r, regions, log)
        return Call(CALLS[op], [r.choice(wins), word(r), word(r)], regions, conv='t', ret='a')
    wins = shown_env(r, regions, log)   # NotifyShown / SetShown: EDX the window's upper half or random
    if op == 'AddChild':   # shown_env's (previous links set) and a new window (no links now and then one set, children or not), or one of
        # the tree's or none; the flags' low bytes 0 or not; the objects' events hashed by what the flags ask; the focus [0xfaa5ec]
        for w in wins:
            prev, c = 0, struct.unpack_from('<I', regions[w], 0x5c)[0]
            while c:
                put(regions[c], 0x58, 'I', prev)
                prev, c = c, struct.unpack_from('<I', regions[c], 0x54)[0]
        put(regions[wins[0]], 0x58, 'I', 0)
        child = bytearray(r.bytes(0x120))
        for off in (0x60, 0x54, 0x58):
            put(child, off, 'I', 0 if r.random() < 0.85 else r.choice(wins))
        put(child, 0x5c, 'I', r.choice([0, 0, ARG + 0x200]))   # (a leaf child of its own: a tree's window could loop back)
        leaf = bytearray(r.bytes(0x120))
        put(leaf, 0x54, 'I', 0)
        put(leaf, 0x5c, 'I', 0)
        put(leaf, 0x98, 'I', r.choice([OBJECT98, 0]))
        put(leaf, 0x9c, 'I', r.choice([MISC + 0x40, 0]))
        regions[ARG + 0x200] = leaf
        put(child, 0x98, 'I', r.choice([OBJECT98, 0]))
        put(child, 0x9c, 'I', r.choice([MISC + 0x40, 0]))
        regions[ARG] = child
        window = r.choice(wins[:1] * 3 + wins)
        up, w = [], window   # (the window and its parents left out of the children offered: they would make a loop)
        while w in regions and w not in up:
            up.append(w)
            w = struct.unpack_from('<I', regions[w], 0x60)[0]
        arg = r.choice([ARG] * 8 + [0] + [w for w in wins if w not in up])
        depth, words = r.choice([0, 0, 1, 0x80, 0x100]), r.choice([0, 0, 1, 0xff, 0x200])
        mask = (0xc0 if depth & 0xff else 0) | (0xf000 if words & 0xff else 0)
        blob_bytes(log, 3, 1, 0 if mask == 0xf0c0 else mask)
        blob_bytes(log, 22, 1, 0 if mask == 0xf0c0 else mask)
        regions[VTABLE] = vtable({5: 1, 3: 2})
        put(regions[MISC + 0x80], 0xc, 'I', recorder(22, 2))
        regions[0xfaa5ec] = struct.pack('<I', r.choice([0, wins[0], r.choice(wins)]))
        return Call(CALLS[op], [window, arg, depth | r.getrandbits(8) << 24, words | r.getrandbits(8) << 24],
                    regions, conv='t', ret='w')
    if op in ('RaiseToTop', 'OpenTree', 'CloseTree'):   # shown_env's, the previous links +0x58 set; the windows' vtable a recorder's (slot 0)
        for w in wins:
            prev, c = 0, struct.unpack_from('<I', regions[w], 0x5c)[0]
            while c:
                put(regions[c], 0x58, 'I', prev)
                prev, c = c, struct.unpack_from('<I', regions[c], 0x54)[0]
            put(regions[w], 0, 'I', VTABLE)
        put(regions[wins[0]], 0x58, 'I', 0)
        regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
        if op == 'RaiseToTop':
            return Call(CALLS[op], [r.choice(wins[1:] * 4 + wins[:1]) if len(wins) > 1 else wins[0]], regions, conv='t', ret='a')
        return Call(CALLS[op], [r.choice(wins[:1] * 3 + wins)], regions, conv='t', ret='w')
    window = r.choice(wins)
    if op == 'SetEnabled':
        for w in wins:
            regions[w][0x48] = (regions[w][0x48] & 0xfd) | r.choice([0, 2])
    shown = r.choice([0, 0, 1, 1, 2, 0x10000, 0xffff0000, 0x8000, r.getrandbits(32)])
    if op == 'NotifyShown':
        edx = r.choice([window & 0xffff0000, r.getrandbits(32)])
        return Call(CALLS[op], [window, edx, shown], regions, conv='f', ret='v')
    return Call(CALLS[op], [window, shown], regions, conv='t', ret='w' if op == 'SetEnabled' else 'a')


def rect_env(r, regions, log):
    """offset_env's windows, small rectangles +8 / +0x2c and margins +0xf8, anchors 0xf0000 mixed in; the objects' events (SetRect's sets only
    its +0x14) hashed"""
    wins = offset_env(r, regions, log)
    blob(log, 3, 1, 0x20)
    blob(log, 22, 1, 0x20)
    for w in wins:
        buf = regions[w]
        left, top = r.randrange(-30, 30), r.randrange(-30, 30)
        put(buf, 0xc, 'hhhh', left, top, left + r.choice([0, 5, 40]), top + r.choice([0, 5, 40]))
        put(buf, 0x30, 'hhhh', *[r.randrange(-30, 70) for _ in range(4)])
        put(buf, 0xf8, 'hhhh', *[r.choice([0, 0, 1, 2, -1]) for _ in range(4)])
        put(buf, 0x48, 'I', struct.unpack_from('<I', buf, 0x48)[0] & 0xfff0ffff | r.choice(
            [0, 0, 0x10000, 0x20000, 0x30000, 0x40000, 0x80000, 0xc0000, 0xf0000, 0xf0000, 0x50000, 0xa0000, 0x70000, 0xd0000]))
    return wins


def anyword(r):
    return r.randrange(-0x8000, 0x8000)


def script_case(r, op, regions, log):
    """ScriptStep / SlideStep / ScriptTick: rect_env's, the window one with room past +0x120 (made 0x140 bytes): its script of 2-7 steps (the
    last the end; jumps only forward), the step in it, the byte +0x135 1 or not, the interval +0xf0 (negative ones too: a jump then runs the
    next step at once) and the last time against the clock [0x77c490] (a recorder answering one time); the slide's byte +0x136, count +0x10c
    (0 or not), moves, timer node and target rectangle +0x38; the manager's +0x14 the window or not. The objects' events hashed by the step
    that runs."""
    wins = rect_env(r, regions, log)
    window = r.choice([w for w in wins if not any(w < o < w + 0x140 for o in regions)])
    buf = regions[window]
    buf.extend(r.bytes(0x140 - len(buf)))
    n = r.randrange(2, 8)
    steps = []
    for i in range(n - 1):
        kind = r.choice(['end', 'word', 'jump', 'notify', 'move', 'move', 'move'])
        if kind == 'end':
            steps.append((-9999, anyword(r), anyword(r), 0))
        elif kind == 'word':
            steps.append((-9998, anyword(r), anyword(r), 0))
        elif kind == 'jump':
            steps.append((-9997, r.randrange(i + 1, n), anyword(r), 0))
        elif kind == 'notify':
            steps.append((-9996, r.choice([0, 1, -1, anyword(r)]), 0, 0))
        else:
            steps.append(tuple(r.choice([0, 1, -2, 5, r.randrange(-0x8000, -10000), r.randrange(-9995, 0x8000)]) for _ in range(4)))
    steps.append((-9999, anyword(r), 0, 0))
    regions[SCRIPT] = b''.join(struct.pack('<hhhh', *st) for st in steps)
    now = r.getrandbits(32)
    set_return(log, 0, now)
    regions[CLOCK] = struct.pack('<I', CLOCK + 0x10) + bytes(12) + vtable({}, size=4)
    regions[0x77c490] = struct.pack('<I', CLOCK)
    on, script, at = r.choice([1, 1, 1, 0, 0x81]), r.choice([SCRIPT] * 9 + [0]), r.randrange(n)
    interval = r.choice([0, 5, 100, -1, -50, 0x7fffffff, -0x80000000])
    last = (now - r.choice([0, 3, 6, 101, 1000, r.getrandbits(32)])) & 0xffffffff
    put(buf, 0x135, 'B', on)
    put(buf, 0xe8, 'I', script)
    put(buf, 0xec, 'h', at)
    put(buf, 0xf0, 'i', interval)
    put(buf, 0xf4, 'I', last)
    put(buf, 0x136, 'B', r.choice([0, 1, 0x80]))
    put(buf, 0x10c, 'h', r.choice([0, 0, 1, 3, -1]))
    put(buf, 0x104, 'hhhh', *[r.choice([0, 1, -3, 7]) for _ in range(4)])
    put(buf, 0x100, 'I', TNODE)
    regions[TNODE] = r.bytes(0x20)
    put(buf, 0x38, 'Ihhhh', 0x705920, *[r.randrange(-30, 70) for _ in range(4)])
    regions[MANAGER] = struct.pack('<6I', MANAGER + 0x40, 0, 0, 0, 0, r.choice([window, window, 0, wins[0]]))
    kind = None   # (the step that runs: past the jumps, each one's next step run at once only for a negative interval)
    if op == 'SlideStep':
        kind = 'rect' if buf[0x136] else None
    elif on == 1 and script and ((now - last + 0x80000000) & 0xffffffff) - 0x80000000 > interval:
        i = at
        while steps[i][0] == -9997 and 0 > interval:
            i = steps[i][1]
        kind = None if steps[i][0] == -9997 else {-9998: 'word', -9996: 'notify'}.get(steps[i][0], 'rect')
    mask = {None: 0, 'rect': 0xf00000, 'word': 0x300, 'notify': 0x30}[kind]
    blob_bytes(log, 3, 1, mask)
    blob_bytes(log, 22, 1, mask)
    c = Call(CALLS[op], [window], regions, conv='t', ret='w')
    c.wins, c.now = wins, now   # (for proc_case)
    return c


PROC_MESSAGES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 9, 10, 11, 12, 0x10, 0x10, 0x10, 0x1e, 0x1e, 0x10001, 0x10002, 0x10003, 0x10003, 0x10004, 0x10004,
                 0x10004, 0x10004, 0x10005, 0x10005, 0x10005, 0x10006, 0x10007, 0, 13, 0x11, 0x1f, 0x10000, 0x10008, 0x80000001, 0x80010005]


def proc_case(r, regions, log):
    """DefaultProc (0x65f6d1): script_case's (the timers' and the tick's), the windows' vtable +0 recorders (slot 2 one argument; slot 11's
    answer random) and their previous links +0x58 set (RaiseToTop, OpenTree); messages 0-13, the timer 0x10 (0x80084 / 0x80085 or not), 0x1e,
    0x10001-0x10008 and a few others (the top bit set too); a and b by the message: the moves' and edges' words, the press buttons 0-3 (the
    table [0xfaa598]'s entries pressed or not, points near the one moved to), the timers' ids; the double-click time [0x77c480], the flags
    +0x48 bit 3 and the buttons held +0x11c; the cursor table [0xfaa604] 0 (ApplyCursor returns at once), [0xfaa5d4] and the active window
    [0xfaa5dc]. The objects' events hashed by what the message runs (SetRectTo's, Offset's, the steps'; nothing for the rest)."""
    message = r.choice(PROC_MESSAGES)
    a, b = r.getrandbits(32), r.getrandbits(32)
    if message == 0x10:
        a = r.choice([0x80084, 0x80085, 0x80084, 0x80085, 0x80083, 0x80086, a])
    sub = 'SlideStep' if message == 0x10 and a == 0x80085 else 'ScriptStep'
    c = script_case(r, sub, regions, log)
    window, regions = c.args[0], c.regions
    wins = c.wins
    for w in wins:
        prev, k = 0, struct.unpack_from('<I', regions[w], 0x5c)[0]
        while k:
            put(regions[k], 0x58, 'I', prev)
            prev, k = k, struct.unpack_from('<I', regions[k], 0x54)[0]
        put(regions[w], 0, 'I', VTABLE)
    put(regions[wins[0]], 0x58, 'I', 0)
    regions[VTABLE] = vtable({5: 1, 3: 2, 2: 1})
    set_return(log, 11, r.getrandbits(32))
    regions[MANAGER + 0x40] = vtable({13: 1, 8: 2, 9: 1})
    later = [w for w in wins if struct.unpack_from('<I', regions[w], 0x54)[0]]
    if message == 9 and later and r.random() < 0.7:   # (raised only with a next sibling)
        window = r.choice(later)
    buf = regions[window]
    put(buf, 0x11c, 'I', r.choice([0, 0, 1, 2, 4, 7, r.getrandbits(32)]))
    buf[4] = r.choice([0, 0, 0, 1])   # (posts refused for a window whose byte +4 is set)
    put(buf, 0x48, 'I', struct.unpack_from('<I', buf, 0x48)[0] & ~8 | r.choice([0, 0, 8]))
    if message in (0x10003, 0x10004, 0x10005):
        x, y = r.randrange(-50, 50), r.randrange(-50, 50)
        table = bytearray()
        for _ in range(4):
            table += struct.pack('<hhhHI', r.choice([0, 0, 1, 1, 2, -1]), x + r.choice([0, 3, -6, 7, -9, 40]), y + r.choice([0, 6, -7, 2, 30]),
                                 r.getrandbits(16), r.choice([0, r.getrandbits(32)]))
        regions[0xfaa598] = table
        point = (x & 0xffff) << 16 | (y & 0xffff)
        if message == 0x10003:
            a = point if r.random() < 0.9 else r.getrandbits(32)
        else:
            a, b = r.choice([0, 1, 2, 3]), point if r.random() < 0.8 else r.getrandbits(32)
            for i in range(4):
                if r.random() < 0.6:   # (the last time near the clock's now: within the double-click time or not)
                    struct.pack_into('<I', regions[0xfaa598], 0xc * i + 8, (c.now - r.choice([0, 5, 200, 1000])) & 0xffffffff)
            if r.random() < 0.6:   # (the button's entry pressed and its bit held more often)
                struct.pack_into('<h', regions[0xfaa598], 0xc * a, 1)
                put(buf, 0x11c, 'I', struct.unpack_from('<I', buf, 0x11c)[0] | 1 << a)
    elif message in (0x10006, 0x10007):
        a = r.choice([0, 1, 2, 3])
        regions[0xfaa598] = r.bytes(0x30)
    regions[0x77c480] = struct.pack('<i', r.choice([0, 100, 500, -1, 0x7fffffff]))
    regions[0xfaa604] = bytes(4)
    regions[0xfaa5d4] = struct.pack('<I', r.choice([0, 1]))
    regions[0xfaa5dc] = struct.pack('<I', r.choice([window, 0, wins[0]]))
    if message in (6, 7, 8, 9):
        a, b = r.choice([a, a & 0xffff, word(r), 0, 0, 1]), r.choice([b, word(r), 0])
    if message == 8:   # (the edges near the window's: a >> 16 left, a's word top)
        left, top, right, bottom = struct.unpack_from('<hhhh', buf, 0xc)
        if r.random() < 0.6:
            a = (left + r.choice([0, 2])) << 16 & 0xffffffff | (top + r.choice([0, -3])) & 0xffff
            b = (right + r.choice([0, 1])) << 16 & 0xffffffff | (bottom + r.choice([0, 4])) & 0xffff
    mask = None
    if message == 8:
        mask = 0xf00000
    elif message == 7:
        mask = 0xf0f000
    elif message not in (0x10, 0x1e):
        mask = 0
    if mask is not None:
        blob_bytes(log, 3, 1, mask)
        blob_bytes(log, 22, 1, mask)
    return Call(CALLS['DefaultProc'], [window, message, a, b], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The GUI library's kind 7 window (0x7059f0) and its item tree (src/game/ui/window7059f0) against the originals. ColumnBottom (0x6636f6): the
# column array +0x16c of 12 random rectangles, the column 0-11 (now and then -1 or -2: before the array), the upper half random. NextItem /
# NthItem (0x663a93, 0x663ba2): items(): a random forest of 1-12 items (each four dwords at an index 4k, or 4k + 1 now and then: the links
# then read across items, still in the array) whose missing links are -1 (now and then -2 or -5), the item given one of them, the level
# pointer given or 0; NthItem's n -1 to past the count +0x150 (itself 0 to past the items), +0x144 set or 0, the first shown +0x160 one of
# the items (or -1). SetNthItemData (0x664b68): NthItem's, any data. FindItemData (0x664d2a): the items' data dwords a few values, the count
# mostly the items'. SetRowHeight (0x664ba8): the inner rectangle's top / bottom words, the height small, negative, 0 (a fault in both) or any.
# RefreshRows (0x663324): window.py's notify_env tree (the list's parent one of it), 0-4 rows (the word +0x172 now and then -1) of a row
# window with 0-3 column windows (its first child and that one's siblings; +0x164 now and then -1), each shown or not with recorder
# objects, handlers and a recorder vtable (slots 7 and 9); the items nine dwords apart (links, four cells that are objects of CELLS, a
# dword whose address is sent), the column bottoms 1 or not, the count lowered until walk() keeps every read in the links. ResetItems
# (0x663085): the columns +0x164 small, negative (-4 a fault in both), the dword count +0x158 small or negative, the links with room before
# them. RootEnd (0x663cb8): items() with some made free (parent -2), the first shown one of them, -1 or -2, now and then no array.
# FitBar (0x664495): a bar of its own (shown or not, its parent one of notify_env's tree, a recorder slot 7, its range and step any), or
# none; the count around the rows or at the edges. FreeItem (0x664605): items() of 4 + 0-3 columns, their cells distinct objects (with
# counted buffers or none) or 0, some items free, the column bottoms 0 or not, the heap; the item any, previous its true one mostly,
# siblings 0 or not (0x100 too: a dword).
# usage: window7059f0.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ColumnBottom,NextItem,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, heap, log_region, put, set_return, vtable  # noqa: E402
from window import MISC, OBJECT98, RANGES, VTABLE, notify_blobs, notify_env  # noqa: E402

CALLS = {'ColumnBottom': 0x6636f6, 'NextItem': 0x663a93, 'NthItem': 0x663ba2, 'SetNthItemData': 0x664b68, 'SetRowHeight': 0x664ba8,
         'FindItemData': 0x664d2a, 'RefreshRows': 0x663324, 'ResetItems': 0x663085, 'RootEnd': 0x663cb8, 'FitBar': 0x664495,
         'FreeItem': 0x664605}
B = Bench(RANGES + [(0x6636f6, 0x663718), (0x663a93, 0x663c11), (0x664b68, 0x664bf0), (0x664d2a, 0x664db7), (0x663324, 0x663554),
                    (0x4f0240, 0x4f0250), (0x65d547, 0x65d56b), (0x65d56b, 0x65d57c), (0x65c672, 0x65c707),
                    (0x663085, 0x66316b), (0x663cb8, 0x663d56), (0x664495, 0x6644ea), (0x66b47d, 0x66b508),
                    (0x664605, 0x6647cb), (0x47db20, 0x47db80)])
WINDOW, COLUMNS, LINKS, OUT = RAM, RAM + 0x200, RAM + 0x400, RAM + 0x800
LIST, LINKS2, COLS2, CELLS, ROWS2, ROWWINS = RAM + 0x9000, RAM + 0x9420, RAM + 0x9e00, RAM + 0x9f00, RAM + 0x9c00, RAM + 0xb000
BAR, OBJS, BUFS = RAM + 0xd000, RAM + 0x1000, RAM + 0x1200


def none(r):
    return r.choice([-1] * 8 + [-2, -5])


def items(r, window, regions, stride=4, base=LINKS):
    """the forest, its items stride dwords apart at base: returns the item indices"""
    n = r.randint(1, 12)
    shift = 1 if r.random() < 0.1 else 0
    idx = [stride * k + shift for k in r.sample(range(n), n)]   # (shuffled: a child, a sibling or a parent at index 0 too)
    links = [r.getrandbits(32) for _ in range(stride * n + max(8, stride))]
    parent = [None] + [r.choice([None, r.randrange(k)]) for k in range(1, n)]
    kids = {p: [k for k in range(n) if parent[k] == p] for p in [None] + list(range(n))}
    for k in range(n):
        links[idx[k] + 2] = idx[kids[k][0]] if kids[k] else none(r)
        links[idx[k] + 3] = idx[parent[k]] if parent[k] is not None else none(r)
        sibs = kids[parent[k]]
        at = sibs.index(k)
        links[idx[k]] = idx[sibs[at + 1]] if at + 1 < len(sibs) else none(r)
    regions[base] = struct.pack('<%dI' % len(links), *[v & 0xffffffff for v in links])
    put(window, 0x144, 'I', base)
    return idx


PAD = 8


def walk(arr, shown, first, count, nrows, ncols, bottoms, cells):
    """RefreshRows' reads of the links (arr[i + PAD] is links[i]) in Python: False when one leaves the array, loops on or takes a cell
    for an object that is none of the cells"""
    def get(i):
        if not -PAD <= i < len(arr) - PAD:
            raise IndexError
        return arr[i + PAD]

    def nxt(i):
        if get(i + 2) >= 0:
            return get(i + 2)
        if get(i) >= 0:
            return get(i)
        if get(i + 3) < 0:
            return -2
        n = get(i + 3)
        for _ in range(64):
            if not (get(n) < 0 and get(n + 3) >= 0):
                return get(n)
            n = get(n + 3)
        raise IndexError

    try:
        item, i = shown, 0
        while 0 <= first < count and i < first and item != -1:
            item, i = nxt(item), i + 1
        if item >= 0:
            p, steps = get(item + 3), 0
            while p >= 0:
                p, steps = get(p + 3), steps + 1
                if steps > 64:
                    return False
        for row in range(nrows):
            if row + first < count:
                get(item + 1)
                get(item + 8)
                for c in range(ncols):
                    if bottoms[c] != 1 and get(item + 4 + c) & 0xffffffff not in cells:
                        return False
                item = nxt(item)
    except IndexError:
        return False
    return True


def refresh_case(r):
    """RefreshRows (0x663324): see the top"""
    log = log_region()
    regions = {LOG: log}
    wins = notify_env(r, regions, log)
    regions[VTABLE] = vtable({5: 1, 3: 2, 0: 1, 7: 1, 9: 1})
    notify_blobs(log, 0, 0x3c00)
    regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
    for k in (7, 9):
        if r.random() < 0.3:
            set_return(log, k, r.getrandbits(32))
    window = bytearray(r.bytes(0x180))
    regions[LIST] = window
    window[0x48] = (window[0x48] & 0xfe) | r.choice([0, 1, 1])
    put(window, 0x60, 'I', r.choice(wins + [0]))
    nrows, ncols = r.choice([0, 1, 2, 3, 4]), r.choice([0, 1, 1, 2, 3])
    while nrows * ncols > 8:
        nrows -= 1
    put(window, 0x172, 'h', r.choice([nrows] * 6 + [-1]))
    put(window, 0x164, 'h', r.choice([ncols] * 6 + [-1]))
    # the rows: a window each with its columns' windows (its first child, then that one's siblings), shown or not
    rows, at = [], ROWWINS
    for _ in range(nrows):
        group = [at + 0x120 * k for k in range(max(ncols, 1) + r.choice([0, 0, 1]))]
        at += 0x120 * len(group)
        for k, w in enumerate(group):
            buf = bytearray(r.bytes(0x120))
            buf[4] = 0 if r.random() < 0.9 else 1
            buf[0x48] = (buf[0x48] & 0xfe) | r.choice([0, 1])
            put(buf, 0, 'I', VTABLE)
            put(buf, 0x5c, 'I', group[1] if k == 0 and len(group) > 1 else 0)
            put(buf, 0x54, 'I', group[k + 1] if 0 < k < len(group) - 1 else 0)
            put(buf, 0x60, 'I', r.choice(wins + [LIST, 0]) if k == 0 else group[0])
            put(buf, 0x98, 'I', r.choice([OBJECT98, OBJECT98, 0]))
            put(buf, 0x9c, 'I', r.choice([MISC + 0x40, MISC + 0x40, 0]))
            put(buf, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
            regions[w] = buf
        rows.append(group[0])
    regions[ROWS2] = struct.pack('<%dI' % (nrows + 1), *rows, r.getrandbits(32))
    put(window, 0x148, 'I', ROWS2 if r.random() < 0.95 else 0)
    bottoms = [r.choice([1, 1, 0, 2, -1]) for _ in range(4)]
    regions[COLS2] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', b) for b in bottoms)
    put(window, 0x16c, 'I', COLS2)
    cells = [CELLS + 8 * j for j in range(8)]
    regions[CELLS] = b''.join(struct.pack('<II', r.getrandbits(32), r.choice([0, r.getrandbits(32)])) for _ in cells)
    # the items, nine dwords apart: four links, four cells (objects of CELLS), a dword whose address the rows are sent
    idx = items(r, window, regions, stride=9, base=LINKS2)
    raw = regions.pop(LINKS2)
    links = list(struct.unpack('<%di' % (len(raw) // 4), raw))
    for i in idx:
        for c in range(4):
            links[i + 4 + c] = r.choice(cells)
    arr = [none(r) for _ in range(PAD)] + links   # (PAD dwords before the array: the reads of a missing item's links)
    regions[LINKS2 - 4 * PAD] = struct.pack('<%di' % len(arr), *arr)
    n = len(idx)
    shown = r.choice(idx + [idx[0], -1])
    first = r.choice([0, 0, 1, 2, -1, r.randrange(n)])
    count = r.choice([n, n, first + nrows, r.randrange(-1, n + 3), 0x7fffffff])
    for k in range(nrows, -2, -1):
        if walk(arr, shown, first, count, nrows, max(ncols, 0), bottoms, cells):
            break
        count = first + k
    put(window, 0x150, 'i', count)
    put(window, 0x154, 'i', first)
    put(window, 0x160, 'i', shown)
    return Call(CALLS['RefreshRows'], [LIST], regions, conv='t', ret='v')


def free_case(r, window, regions):
    """FreeItem (0x664605): see the top"""
    ncols = r.choice([0, 1, 2, 3])
    put(window, 0x164, 'h', ncols)
    regions[HEAP] = heap()
    regions[COLUMNS] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', r.choice([0, 0, 1, -1])) for _ in range(4))
    put(window, 0x16c, 'I', COLUMNS)
    bufs = bytearray(r.bytes(0x20 * 8))
    for i in range(8):
        bufs[0x20 * i + 2] = r.choice([0, 1, 2, 5, 0xff])
    regions[BUFS] = bufs
    objs = bytearray(r.bytes(0x10 * 16))
    for j in range(16):
        put(objs, 0x10 * j + 4, 'I', r.choice([0, BUFS + 0x20 * r.randrange(8) + 3]))
    regions[OBJS] = objs
    idx = items(r, window, regions, stride=4 + ncols)
    links = list(struct.unpack('<%di' % (len(regions[LINKS]) // 4), regions.pop(LINKS)))
    cells = r.sample(range(16), 16)
    for i in idx:
        for c in range(ncols):
            links[i + 4 + c] = OBJS + 0x10 * cells.pop() if cells and r.random() < 0.8 else 0
        if r.random() < 0.15:   # (free)
            links[i + 3] = -2
    regions[LINKS - 0x20] = r.bytes(0x20) + struct.pack('<%di' % len(links), *links)   # (room for the dword before the array)
    item = r.choice(idx)
    before = [j for j in idx if links[j] == item]
    roots = [j for j in idx if links[j + 3] < 0 and not any(links[k] == j for k in idx)]
    put(window, 0x150, 'i', r.choice([len(idx), len(idx), 1, r.randrange(len(idx) + 1)]))
    put(window, 0x160, 'i', r.choice(roots[:1] * 4 + idx + [-1]))
    previous = r.choice((before[:1] or [-1]) * 4 + [-1, r.choice(idx)])
    return Call(CALLS['FreeItem'], [WINDOW, r.getrandbits(32), item, previous & 0xffffffff, r.choice([0, 1, 1, 0x100])], regions,
                conv='t', ret='v')


def bar_case(r):
    """FitBar (0x664495): see the top"""
    log = log_region()
    regions = {LOG: log}
    wins = notify_env(r, regions, log)
    regions[VTABLE] = vtable({5: 1, 3: 2, 0: 1, 7: 1})
    bar = bytearray(r.bytes(0x150))
    bar[4] = 0 if r.random() < 0.9 else 1
    bar[0x48] = (bar[0x48] & 0xfe) | r.choice([0, 1])
    put(bar, 0, 'I', VTABLE)
    put(bar, 0x5c, 'I', 0)
    put(bar, 0x60, 'I', r.choice(wins + [0]))
    put(bar, 0x98, 'I', r.choice([OBJECT98, OBJECT98, 0]))
    put(bar, 0x9c, 'I', r.choice([MISC + 0x40, MISC + 0x40, 0]))
    put(bar, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
    put(bar, 0x138, 'ii', r.choice([0, 0, r.randrange(-3, 9)]), r.choice([0, r.randrange(-3, 9)]))
    put(bar, 0x144, 'i', r.choice([1, -1, 2, -3, 0]))
    regions[BAR] = bar
    rows = r.choice([0, 1, 5, 8, -1, 0x7fffffff, -0x80000000])
    count = r.choice([rows, rows + 1, rows - 1, 0, 3, 12, -0x80000000, 0x7fffffff]) & 0xffffffff
    return Call(CALLS['FitBar'], [r.choice([BAR] * 9 + [0]), count, rows & 0xffffffff, r.getrandbits(32)], regions, ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'RefreshRows':
        return refresh_case(r)
    if op == 'FitBar':
        return bar_case(r)
    window = bytearray(r.bytes(0x180))
    regions = {WINDOW: window}
    if op == 'SetRowHeight':   # (the inner rectangle's top and bottom near each other or any; the height small, negative or 0: a fault in both)
        if r.random() < 0.7:
            top = r.randrange(-50, 300)
            put(window, 0x13e, 'hh', top, top + r.randrange(-10, 400))
        height = r.choice([1, 2, 7, 12, 16, 20, -3, -1, 0x7fff, 0x8000, r.getrandbits(16), 0]) & 0xffff
        return Call(CALLS[op], [WINDOW, height | r.choice([0, r.getrandbits(16)]) << 16], regions, conv='t', ret='v')
    if op == 'ColumnBottom':
        regions[COLUMNS - 0x20] = r.bytes(0x20 + 12 * 12)
        put(window, 0x16c, 'I', COLUMNS)
        column = r.choice(list(range(12)) + [-1, -2]) & 0xffff
        return Call(CALLS[op], [WINDOW, column | r.choice([0, r.getrandbits(16)]) << 16], regions, conv='t', ret='a')
    if op == 'ResetItems':   # (the columns +0x164 any small, -4 a fault in both; the dword count small, negative too)
        regions[LINKS - 0x40] = r.bytes(0x40 + 4 * 80)
        put(window, 0x144, 'I', LINKS)
        put(window, 0x164, 'h', r.choice([0, 1, 2, 3, 5, -1, -3, -5, -8, -4]))
        put(window, 0x158, 'i', r.choice([0, 1, 3, 4, 7, 8, 9, 20, 33, 48, -1, -9, r.randrange(64)]))
        return Call(CALLS[op], [WINDOW], regions, conv='t', ret='v')
    if op == 'FreeItem':
        return free_case(r, window, regions)
    idx = items(r, window, regions)
    if op == 'RootEnd':   # (some items free: their parent -2; the outputs' old values random)
        links = bytearray(regions[LINKS])
        for i in idx:
            if r.random() < 0.25:
                put(links, 4 * (i + 3), 'i', -2)
        regions[LINKS] = links
        regions[OUT] = r.bytes(8)
        put(window, 0x160, 'i', r.choice(idx + [idx[0], -1, -2]))
        if r.random() < 0.1:
            put(window, 0x144, 'I', 0)
        return Call(CALLS[op], [WINDOW, OUT, OUT + 4], regions, conv='t', ret='v')
    if op == 'FindItemData':   # (the data of a few items a value looked for)
        links = bytearray(regions[LINKS])
        datas = [0, 7, 0xffffffff, r.getrandbits(32)]
        for i in idx:
            put(links, 4 * (i + 1), 'I', r.choice(datas))
        regions[LINKS] = links
        put(window, 0x150, 'i', r.choice([len(idx), len(idx), 0, 1, r.randrange(len(idx) + 1), len(idx) + 2, -1]))
        put(window, 0x160, 'i', r.choice(idx + [idx[0], -1]))
        return Call(CALLS[op], [WINDOW, r.choice(datas + [5])], regions, conv='t', ret='a')
    if op == 'NextItem':
        regions[OUT] = r.bytes(4)
        return Call(CALLS[op], [WINDOW, r.choice(idx), r.choice([OUT, OUT, 0])], regions, conv='t', ret='a')
    put(window, 0x150, 'i', r.choice([0, 1, len(idx), len(idx) + 3, r.randrange(len(idx) + 1), -1]))
    put(window, 0x160, 'i', r.choice(idx + [idx[0], -1]))
    if r.random() < 0.1:
        put(window, 0x144, 'I', 0)
    n = r.choice([-1, 0, 1, 2, 3, r.randrange(len(idx) + 2), len(idx) - 1, len(idx), 0x7fffffff, -0x80000000])
    if op == 'SetNthItemData':
        return Call(CALLS[op], [WINDOW, n & 0xffffffff, r.getrandbits(32)], regions, conv='t', ret='w')
    return Call(CALLS[op], [WINDOW, n & 0xffffffff], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

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
# siblings 0 or not (0x100 too: a dword). Select (0x66525c): the queues, 0xfaa5d4 set or not, a shown or hidden parent (or none), the
# highlight a recorder (its event's rectangle hashed) or none, the rows / count / first small or negative, the row in and around them,
# the selection the row's, none or any; +0x48's flag 0x100 random. FindText (0x664db7): items() of 4 + 0-3 columns (-1 now and then),
# their cells 16 wide strings of six texts (some without data), the column bottoms mostly 0, the column 1-4 or past either end.
# RemoveItem / Clear / ScrollTo (0x6647cb, 0x6649d5, 0x665739): list_env() (FreeItem's items with notify_env's windows, the queues, a
# highlight, the bar child 1 or none, no row windows), the n / position around the count, the selection it or not, now and then no items.
# Grow (0x66316b): items() of 4 + 0-3 columns (or a negative count), some free, the dword count +0x158 theirs or less, now and then no
# array; n 0-20 or -1; the heap (its first allocation failing now and then).
# SortsBefore (0x663edc): the sort column +0x168 0-3 (or -1 / -2: before the column array), +0x16a 0, 1 or any, the column bottoms 1 or not;
# the texts four dwords (wide strings of a few short texts, or numbers around each other), the cells four counted strings (their text
# or none) or 0 or numbers.
# CopyCells (0x663dfa): 0-4 columns (now and then -1), the bottoms mostly 0; the texts wide strings of 0-40 characters, 0 or (bottom not
# 0) any dword; the cells random; the heap, failing now and then. Each new string's allocator byte is a stack byte the original never
# sets (sic), different in the two runs: allocator_masks() leaves those bytes of the heap out (the strings' first byte; the listing reads
# no string's allocator: CopyCells only stores the new string, StdWString_CtorChars writes the byte and nothing after it reads one).
# SetRowSize / SetRow (0x663718, 0x663864): see row_case(). SetHighlight (0x662eee): see highlight_case().
# ShowRows (0x664fde): see show_case(). SendToRows (0x6650d4): RefreshRows' list, rows and items; the message 0x1e mostly (else 3,
# 0x15, 7 or one with the top bit), a and b any (the row windows' handlers fncmp's logger or none).
# NthChild (0x663c11): NthItem's forest, the n likewise, the parent -1, -2, an item's place or past them.
# Sort (0x6628f4): sort_case(): a top list of 1-10 items 4 + 1-3 columns apart in a shuffled order (each item's parent -1, now and
# then -2 or -5; the last's next -1), with children of their own (their first child +2) left out of the list; the count about the
# list's length, the first shown its head; the sort column 0 to the last (now and then -1: the parent dword), the mode +0x16a 0 or 1
# (now and then 2), the column bottoms 1 (numbers: a few values, ties included, or the extremes) or 0 / 2 (counted wide strings of
# short texts from ALPHA, ties and prefixes included, or none); the columns 0 now and then.
# Cell (0x663d56): RefreshRows' list and rows (now and then one 0 or one without children), the row 0 to the rows shown (only 0 without the row array), the
# column a word 0 to past the columns (upper half random).
# NthItemCell (0x664bf0): NthItem's items eight dwords apart, their cells strings of CELLS (text or none); 0-4 columns (or -1), the
# bottoms 0 mostly, 1 or -1; the column 0 to past them.
# PostColumnAt (0x665494): the selection +0x14c -1, 0, small or any, 0-4 columns (or -1) of edges small or any, x on them, between or
# any (upper half random); the parent +0x60 a window (its byte +4 mostly clear) or 0, window.py's queues.
# ScrollFromBar (0x664ea3): RefreshRows' list, its children (+0x5c) a bar of id 1 (a recorder vtable: slot 8 answers the first row
# shown RefreshRows' set-up chose; the list's +0x154 then another value mostly, or the same), one of another id before it, or none;
# window.py's queues and [0xfaa5d4] set or not, the list's byte +4 mostly clear.
# NthItemData (0x664c71): SetNthItemData's set-up, the out a dword or 0. OnKey (0x665b51): ScrollTo's, flag 0x200 of +0x48 mostly set,
# the keys Home / End / Up / Down or others. PostSelection / NotifySort (0x6656fa, 0x6658b9), SetSortColumn / SetSortOrder (0x66590f,
# 0x66594d: the word the current one a third of the time, -1, small or any, the dword's upper half random): notify_case() below.
# ColumnSpan (0x664cb9): column_case() below.
# usage: window7059f0.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ColumnBottom,NextItem,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, Mask, blob_bytes, heap, log_region, put, recorder, set_return, vtable  # noqa: E402
from window import MISC, OBJECT98, RANGES, VTABLE, notify_blobs, notify_env, queues, shown_env  # noqa: E402

CALLS = {'ColumnBottom': 0x6636f6, 'NextItem': 0x663a93, 'NthItem': 0x663ba2, 'SetNthItemData': 0x664b68, 'SetRowHeight': 0x664ba8,
         'FindItemData': 0x664d2a, 'RefreshRows': 0x663324, 'ResetItems': 0x663085, 'RootEnd': 0x663cb8, 'FitBar': 0x664495,
         'FreeItem': 0x664605, 'Select': 0x66525c, 'FindText': 0x664db7, 'RemoveItem': 0x6647cb, 'Clear': 0x6649d5,
         'ScrollTo': 0x665739, 'Grow': 0x66316b, 'SortsBefore': 0x663edc, 'CopyCells': 0x663dfa,
         'AddItem': 0x66403b, 'SetRowSize': 0x663718, 'SetRow': 0x663864,
         'SetHighlight': 0x662eee, 'ShowRows': 0x664fde, 'SendToRows': 0x6650d4, 'NthChild': 0x663c11, 'Sort': 0x6628f4,
         'ScrollFromBar': 0x664ea3, 'Cell': 0x663d56,
         'PostColumnAt': 0x665494, 'NthItemCell': 0x664bf0, 'NthItemData': 0x664c71, 'OnKey': 0x665b51, 'PostSelection': 0x6656fa,
         'NotifySort': 0x6658b9, 'ColumnSpan': 0x664cb9, 'SetSortColumn': 0x66590f, 'SetSortOrder': 0x66594d}
LIST_RANGES = (RANGES + [(0x6636f6, 0x663718), (0x663a93, 0x663c11), (0x664b68, 0x664bf0), (0x664d2a, 0x664db7), (0x663324, 0x663554),
                    (0x4f0240, 0x4f0250), (0x65d547, 0x65d56b), (0x65d56b, 0x65d57c), (0x65c672, 0x65c707),
                    (0x663085, 0x66316b), (0x663cb8, 0x663d56), (0x664495, 0x6644ea), (0x66b47d, 0x66b508),
                    (0x664605, 0x6647cb), (0x47db20, 0x47db80), (0x6647cb, 0x664b68), (0x665739, 0x6658b9), (0x66316b, 0x663324), (0x67fff0, 0x680060), (0x6800f0, 0x6802f8), (0x663edc, 0x66403b), (0x67c290, 0x67c2e2), (0x663dfa, 0x663edc), (0x4f0170, 0x4f0237), (0x405280, 0x405390),
                    (0x4f23f0, 0x4f2450), (0x66403b, 0x664495),
                    (0x66525c, 0x665494), (0x659d43, 0x659d90), (0x664db7, 0x664ea3), (0x4f7870, 0x4f7910), (0x665f30, 0x665f71),
                    (0x67acd0, 0x67ad00), (0x45b380, 0x45b5f0), (0x45b680, 0x45b9b0), (0x67bb90, 0x67bd98), (0x67d250, 0x67d2c0),
                    (0x67d3a0, 0x67d400), (0x6918f0, 0x691a30), (0x691ba0, 0x691ce0),
                    (0x663718, 0x663864), (0x663864, 0x6639cb), (0x65d865, 0x65d8e7), (0x659e3e, 0x659e83), (0x65cf5e, 0x65cfd4),
                    (0x65c9bd, 0x65cd95), (0x661700, 0x66176a), (0x65f16b, 0x65f246), (0x662eee, 0x663085), (0x664fde, 0x6650d2), (0x6650d4, 0x6651ee),
                    (0x663c11, 0x663cb8), (0x6628f4, 0x662eee), (0x4f76b0, 0x4f7718), (0x664ea3, 0x664f6d), (0x663d56, 0x663dfa), (0x665494, 0x66552c), (0x664bf0, 0x664c71),
                    (0x664c71, 0x664cb9), (0x665b51, 0x665bfb), (0x6656fa, 0x665739), (0x6658b9, 0x66590f), (0x664cb9, 0x664d2a), (0x66590f, 0x66594d), (0x66594d, 0x6659ad)])
B = Bench(LIST_RANGES)
WINDOW, COLUMNS, LINKS, OUT = RAM, RAM + 0x200, RAM + 0x400, RAM + 0x800
LIST, LINKS2, COLS2, CELLS, ROWS2, ROWWINS = RAM + 0x9000, RAM + 0x9420, RAM + 0x9e00, RAM + 0x9f00, RAM + 0x9c00, RAM + 0xb000
BAR, OBJS, BUFS = RAM + 0xd000, RAM + 0x1000, RAM + 0x1200
HILITE, HVT, OWNER = RAM + 0xe000, RAM + 0xe100, RAM + 0xe400
WSTRS, WCHARS = RAM + 0x1000, RAM + 0x1400
ROWLIST, ROWARR, ROWCOLS, ROWHL, ROWHLVT, ROWWIN = RAM + 0x10000, RAM + 0x10200, RAM + 0x10400, RAM + 0x10600, RAM + 0x10640, RAM + 0x10800
HL_A, HL_B, HL_VT, HL_PARENTS = RAM + 0x11000, RAM + 0x11040, RAM + 0x11080, RAM + 0x11200
ALPHA = [0x41, 0x42, 0x100, 0xffff]
SORTW, SORTCOLS, SORTLINKS, SORTSTRS, SORTCHARS = RAM + 0x12000, RAM + 0x12200, RAM + 0x12400, RAM + 0x12c00, RAM + 0x13000
BARS = RAM + 0x14000
PARENT = RAM + 0x600


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


def row_windows(r, regions, wins, nrows, ncols, owner, base=ROWWINS):
    """the rows: a window each with its columns' windows (its first child, then that one's siblings), shown or not, from base on; the
    row's parent one of wins, the list owner or none. Returns the rows."""
    rows, at = [], base
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
            put(buf, 0x60, 'I', r.choice(wins + [owner, 0]) if k == 0 else group[0])
            put(buf, 0x98, 'I', r.choice([OBJECT98, OBJECT98, 0]))
            put(buf, 0x9c, 'I', r.choice([MISC + 0x40, MISC + 0x40, 0]))
            put(buf, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
            regions[w] = buf
        rows.append(group[0])
    return rows


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
    rows = row_windows(r, regions, wins, nrows, ncols, LIST)
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


def select_case(r):
    """Select (0x66525c): see the top"""
    log = log_region()
    regions = {LOG: log}
    queues(r, regions)
    regions[0xfaa5d4] = struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)]))
    window = bytearray(r.bytes(0x180))
    regions[WINDOW] = window
    owner = bytearray(r.bytes(0x80))
    owner[0x48] = (owner[0x48] & 0xfe) | r.choice([0, 1, 1])
    owner[4], window[4] = (0 if r.random() < 0.85 else 1 for _ in range(2))   # (byte +4 set: no posts to it)
    put(owner, 0x60, 'I', 0)
    regions[OWNER] = owner
    window[0x48] = (window[0x48] & 0xfe) | r.choice([0, 1, 1])
    put(window, 0x60, 'I', r.choice([OWNER, OWNER, 0]))
    put(window, 0x174, 'I', r.choice([HILITE, HILITE, 0]))
    regions[HILITE] = struct.pack('<I', HVT)
    regions[HVT] = vtable({5: 1, 3: 2})
    blob_bytes(log, 3, 1, 0, (0x14, 0xfff))
    rows, count, first = r.choice([0, 1, 3, 8, -1]), r.choice([0, 1, 2, 5, 9, -1]), r.choice([0, 0, 1, 4, -2, r.getrandbits(32)])
    put(window, 0x172, 'h', rows)
    put(window, 0x150, 'i', count)
    put(window, 0x154, 'I', first & 0xffffffff)
    row = r.choice([-1, -5, 0, 1, 2, rows - 1, rows, count - 1, count, 0x7fffffff, r.randrange(10)])
    selected = (row + first) & 0xffffffff if 0 <= row < min(rows, count) else 0xffffffff
    put(window, 0x14c, 'I', r.choice([selected, selected, 0xffffffff, 0xffffffff, r.randrange(10), r.getrandbits(32)]))
    put(window, 0x170, 'h', r.choice([0, 1, 12, 20, -3, r.getrandbits(16) - 0x8000]))
    return Call(CALLS['Select'], [WINDOW, row & 0xffffffff], regions, conv='t', ret='v')


def text_case(r, window, regions):
    """FindText (0x664db7): see the top"""
    ncols = r.choice([0, 1, 1, 2, 2, 3, 3])
    put(window, 0x164, 'h', r.choice([ncols] * 9 + [-1]))
    regions[COLUMNS - 0x18] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', r.choice([0, 0, 0, 0, 1, 2, -1])) for _ in range(6))   # (two before)
    put(window, 0x16c, 'I', COLUMNS if r.random() < 0.95 else 0)
    texts = [tuple(r.choice(ALPHA) for _ in range(r.choice([0, 1, 2, 3]))) for _ in range(6)]
    chars, strs = bytearray(0x20 * 16), bytearray(r.bytes(0x10 * 16))
    for j in range(16):
        t = r.choice(texts)
        struct.pack_into('<%dH' % len(t), chars, 0x20 * j, *t)
        put(strs, 0x10 * j + 4, 'II', WCHARS + 0x20 * j if t or r.random() < 0.5 else 0, len(t))
    regions[WSTRS], regions[WCHARS] = strs, chars
    idx = items(r, window, regions, stride=4 + max(ncols, 1))
    links = bytearray(regions[LINKS])
    for i in idx:
        for c in range(max(ncols, 1)):
            put(links, 4 * (i + 4 + c), 'I', WSTRS + 0x10 * r.randrange(16))
    regions[LINKS] = links
    put(window, 0x150, 'i', r.choice([len(idx)] * 4 + [0, 1, r.randrange(len(idx) + 1), len(idx) + 2]))
    put(window, 0x160, 'i', r.choice(idx + [idx[0], -1]))
    text = r.choice(texts + [(0x42, 0x41, 0x41, 0x41)])
    regions[WCHARS + 0x200] = struct.pack('<%dH' % (len(text) + 1), *text, 0)
    column = r.choice([0, 1, 2, 3, 4, -1, 0x7fffffff, -0x80000000])
    return Call(CALLS['FindText'], [WINDOW, WCHARS + 0x200, column & 0xffffffff], regions, conv='t', ret='a')


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


def allocator_masks(fail, lengths, first=None):
    """The heap offsets of the allocator bytes of the strings made from texts of these lengths (None: no string) in turn, the heap
    failing its allocation fail (fncmp's _alloc: blocks 16-aligned from +0x10, a countdown in the low bits), after an allocation of
    first bytes when given: a Mask, or none"""
    nxt, offsets = HEAP + 0x10 + fail, []

    def alloc(size):
        nonlocal nxt
        countdown, p = nxt & 15, nxt & ~15
        if countdown == 1:
            nxt = p
            return 0
        nxt = p + ((size + 15) & ~15) + (countdown - 1 if countdown else 0)
        return p

    if first is not None:
        alloc(first)
    for n in lengths:
        if n is None:
            continue
        p = alloc(0x10)
        if p:
            offsets.append(p - HEAP)
            if n:
                alloc(((n | 31) + 2) * 2)
    return [Mask(HEAP, offsets, "the new strings' allocator byte: a stack byte the original never sets")] if offsets else []


LIST3, COLS3, LINKS3, OBJS3, BUFS3, BAR3, OTHER3 = RAM + 0xf000, RAM + 0xf200, RAM + 0xf400, RAM + 0xf800, RAM + 0xf900, RAM + 0xfa00, RAM + 0xfc00


def list_env(r):
    """RemoveItem / Clear / ScrollTo's list: notify_env's windows (the list's parent one of them or none), the queues, 0xfaa5d4 set or not, the
    highlight a recorder (slot 3 as id 23, its event's rectangle hashed) or none, the bar (child 1: a recorder vtable, slot 7) first or after
    another child or none, no row windows; free_case's items at LINKS3 (4 + 0-3 columns, cells distinct objects with counted buffers or 0, some
    free), the heap. Returns (window, regions, items)."""
    log = log_region()
    regions = {LOG: log}
    wins = notify_env(r, regions, log)
    regions[VTABLE] = vtable({5: 1, 3: 2, 0: 1, 7: 1})
    regions[0xfaa5d4] = struct.pack('<I', r.choice([0, 0, 1]))
    window = bytearray(r.bytes(0x180))
    regions[LIST3] = window
    window[4] = 0 if r.random() < 0.85 else 1
    window[0x48] = (window[0x48] & 0xfe) | r.choice([0, 1, 1])
    put(window, 0x60, 'I', r.choice(wins + [0]))
    put(window, 0x148, 'I', 0)
    put(window, 0x174, 'I', r.choice([HILITE, HILITE, 0]))
    regions[HILITE] = struct.pack('<I', HVT)
    regions[HVT] = put(vtable({5: 1}), 0xc, 'I', recorder(23, 2))
    blob_bytes(log, 23, 1, 0, (0x14, 0xfff))
    bar = bytearray(r.bytes(0x150))
    bar[4] = 0 if r.random() < 0.9 else 1
    bar[0x48] = (bar[0x48] & 0xfe) | r.choice([0, 1])
    put(bar, 0, 'I', VTABLE)
    put(bar, 0x54, 'I', 0)
    put(bar, 0x5c, 'I', 0)
    put(bar, 0x60, 'I', LIST3)
    put(bar, 0x8c, 'I', 1)
    put(bar, 0x98, 'I', r.choice([OBJECT98, OBJECT98, 0]))
    put(bar, 0x9c, 'I', r.choice([MISC + 0x40, MISC + 0x40, 0]))
    put(bar, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
    put(bar, 0x138, 'ii', r.choice([0, 0, r.randrange(-3, 9)]), r.choice([0, r.randrange(-3, 9)]))
    put(bar, 0x144, 'i', r.choice([1, -1, 2, -3, 0]))
    regions[BAR3] = bar
    other = put(put(bytearray(r.bytes(0x90)), 0x8c, 'I', r.choice([0, 2, 3])), 0x54, 'I', BAR3)
    regions[OTHER3] = put(other, 0, 'I', VTABLE)
    put(window, 0x5c, 'I', r.choice([BAR3] * 6 + [OTHER3] * 3 + [0]))
    ncols = r.choice([0, 1, 2, 3])
    put(window, 0x164, 'h', ncols)
    regions[HEAP] = heap()
    regions[COLS3] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', r.choice([0, 0, 1, -1])) for _ in range(4))
    put(window, 0x16c, 'I', COLS3)
    bufs = bytearray(r.bytes(0x20 * 8))
    for i in range(8):
        bufs[0x20 * i + 2] = r.choice([0, 1, 2, 5, 0xff])
    regions[BUFS3] = bufs
    objs = bytearray(r.bytes(0x10 * 16))
    for j in range(16):
        put(objs, 0x10 * j + 4, 'I', r.choice([0, BUFS3 + 0x20 * r.randrange(8) + 3]))
    regions[OBJS3] = objs
    idx = items(r, window, regions, stride=4 + ncols, base=LINKS3)
    links = list(struct.unpack('<%di' % (len(regions[LINKS3]) // 4), regions.pop(LINKS3)))
    cells = r.sample(range(16), 16)
    for i in idx:
        for c in range(ncols):
            links[i + 4 + c] = OBJS3 + 0x10 * cells.pop() if cells and r.random() < 0.8 else 0
        if r.random() < 0.15:   # (free)
            links[i + 3] = -2
    # (the walks past the last item read the room before the array, the data dwords and the unused ones as links: all made negative)
    used = {j for i in idx for j in range(i, i + 4 + ncols)}
    for i in idx:
        links[i + 1] = r.getrandbits(31) - 0x80000000
    links = [v if j in used else none(r) for j, v in enumerate(links)]
    regions[LINKS3 - 0x20] = struct.pack('<8i', *[none(r) for _ in range(8)]) + struct.pack('<%di' % len(links), *links)
    put(window, 0x158, 'i', r.choice([len(links), len(links), (4 + ncols) * len(idx), 0]))   # (the array's dwords: ResetItems' count)
    roots = [j for j in idx if links[j + 3] < 0 and not any(links[k] == j for k in idx)]
    n = len(idx)
    put(window, 0x150, 'i', r.choice([n, n, n, 1, 0, r.randrange(n + 1)]))
    put(window, 0x160, 'i', r.choice(roots[:1] * 4 + idx + [-1]))
    rows, first = r.choice([0, 1, 3, 8, -1]), r.choice([0, 0, 0, 1, 4, -2])
    put(window, 0x172, 'h', rows)
    put(window, 0x154, 'i', first)
    put(window, 0x170, 'h', r.choice([1, 12, 20, -3]))
    return window, regions, idx


def list_case(r, op):
    """RemoveItem (0x6647cb), Clear (0x6649d5), ScrollTo (0x665739): list_env(), the n / position around the count, the selection it or not"""
    window, regions, idx = list_env(r)
    count = struct.unpack_from('<i', window, 0x150)[0]
    if r.random() < 0.05:
        put(window, 0x144, 'I', 0)
    n = r.choice([-1, 0, 0, 1, 2, count - 1, count - 1, count, r.randrange(len(idx) + 1), 0x7fffffff])
    put(window, 0x14c, 'i', r.choice([n, n, -1, count - 1, r.randrange(10)]))
    if op == 'OnKey':   # (flag 0x200 of +0x48 either way; the keys Home, End, Up and Down or others, the upper half random now and then)
        window[0x49] = (window[0x49] & 0xfd) | r.choice([2, 2, 2, 0])
        key = r.choice([0x2300, 0x2400, 0x2600, 0x2800] * 3 + [0x2500, 0x2700, 0, r.getrandbits(16)])
        return Call(CALLS[op], [LIST3, key | r.choice([0, 0, r.getrandbits(16)]) << 16, r.getrandbits(32)], regions, conv='t', ret='v')
    if op == 'Clear':   # (the count mostly at most the items the walk from the first shown one reaches: past them it reads cells as links)
        links = struct.unpack('<%di' % (len(regions[LINKS3 - 0x20]) // 4), regions[LINKS3 - 0x20])[8:]
        item, reach = struct.unpack_from('<i', window, 0x160)[0], 0
        while item >= 0 and reach < 64:   # (NextItem)
            reach += 1
            if links[item + 2] >= 0 or links[item] >= 0:
                item = links[item + 2] if links[item + 2] >= 0 else links[item]
            elif links[item + 3] < 0:
                item = -2
            else:
                item = links[item + 3]
                for _ in range(64):
                    if not (links[item] < 0 and links[item + 3] >= 0):
                        break
                    item = links[item + 3]
                item = links[item]
        if r.random() < 0.9:
            put(window, 0x150, 'i', min(count, reach))
        return Call(CALLS[op], [LIST3], regions, conv='t', ret='v')
    return Call(CALLS[op], [LIST3, n & 0xffffffff], regions, conv='t', ret='w' if op == 'RemoveItem' else 'v')



def notify_case(r, op):
    """PostSelection (0x6656fa) / NotifySort (0x6658b9) / SetSortColumn / SetSortOrder: the selection +0x14c and the words +0x168 / +0x16a small, -1 or any, the parent
    +0x60 a window (byte +4 mostly clear, its handler +0x114 fncmp's logger or 0) or none; window.py's queues, the hook [0xfaa5fc]"""
    regions = {LOG: log_region()}
    w = bytearray(r.bytes(0x180))
    put(w, 0x14c, 'i', r.choice([-1, 0, 1, 5, r.getrandbits(31), -r.randrange(1, 1 << 31)]))
    put(w, 0x168, 'h', r.choice([0, 1, -1, 5, 0x7fff, -0x8000, r.getrandbits(15)]))
    put(w, 0x16a, 'h', r.choice([1, 1, 0, 2, -1, r.getrandbits(15)]))
    put(w, 0x60, 'I', r.choice([PARENT, PARENT, PARENT, 0]))
    parent = bytearray(r.bytes(0x120))
    parent[4] = r.choice([0, 0, 0, 1])
    put(parent, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
    regions[WINDOW], regions[PARENT] = w, parent
    regions[0xfaa5fc] = put(bytearray(4), 0, 'I', r.choice([0, CDECL_LOG4]))
    queues(r, regions)
    if op in ('SetSortColumn', 'SetSortOrder'):
        current = struct.unpack_from('<h', w, 0x168 if op == 'SetSortColumn' else 0x16a)[0]
        value = r.choice([current] * 4 + [-1, -1, 0, 1, 2, r.getrandbits(16)]) & 0xffff
        return Call(CALLS[op], [WINDOW, value | r.choice([0, r.getrandbits(16)]) << 16], regions, conv='t', ret='a')
    return Call(CALLS[op], [WINDOW], regions, conv='t', ret='a')


def column_case(r):
    """ColumnSpan (0x664cb9): the columns +0x16c (12 bytes each, random, after 0x30 bytes of room) or none, their count word +0x164 0-4
    (now and then negative), the column -2 to past the count, the two word outs each set or 0"""
    w = bytearray(r.bytes(0x180))
    count = r.choice([0, 1, 2, 3, 4, -1])
    put(w, 0x164, 'h', count)
    put(w, 0x16c, 'I', r.choice([COLUMNS + 0x30] * 5 + [0]))
    column = r.choice([-2, -1, 0, 0, 1, 2, 3, 4, 5, r.getrandbits(32)]) & 0xffffffff
    regions = {WINDOW: w, COLUMNS: r.bytes(0x30 + 12 * 6), OUT: r.bytes(8)}
    return Call(CALLS['ColumnSpan'], [WINDOW, column, r.choice([OUT, OUT, OUT, 0]), r.choice([OUT + 4, OUT + 4, OUT + 4, 0])], regions,
                conv='t', ret='a')


ACHARS, ATEXTS = RAM + 0xfd00, RAM + 0xfe00


def add_case(r):
    """AddItem (0x66403b): list_env()'s windows, a new forest at LINKS3 (4 + 0-3 columns) whose last root goes on to the free items'
    chain (parent -2, no child; their cells 0) or ends; the cells of a number column (bottom 1) numbers, the others string objects (+4
    one of 8 short strings of ALPHA, now and then none) or 0; the dword count theirs or the used ones' (the array grown: the heap's first
    block) or 0, now and then no array; the sort column and direction, flag 0x10 (sorted) set or not; the texts like the cells (strings
    for the string columns, mostly set) or none; the parent -1, a position or past the count; refresh or not. The new strings' allocator
    bytes masked (allocator_masks)."""
    window, regions, _ = list_env(r)
    ncols = r.choice([0, 1, 2, 3, 3])
    put(window, 0x164, 'h', ncols)
    stride = 4 + ncols
    bottoms = [r.choice([0, 0, 1, 1, -1]) for _ in range(4)]
    regions[COLS3] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', v) for v in bottoms)
    chars = bytearray(0x100)
    for j in range(8):
        t = [r.choice(ALPHA) for _ in range(r.choice([0, 1, 1, 2, 3]))]
        struct.pack_into('<%dH' % (len(t) + 1), chars, 0x20 * j, *t, 0)
    regions[ACHARS] = chars
    objs = bytearray(r.bytes(0x10 * 16))
    for j in range(16):
        put(objs, 0x10 * j + 4, 'I', r.choice([ACHARS + 0x20 * r.randrange(8)] * 9 + [0]))
    regions[OBJS3] = objs
    nums = [0, 1, -1, 5, 6, 0x7fffffff, -0x80000000]

    def cell(c, free=False):
        if free:
            return 0
        if bottoms[c] == 1:
            return r.choice(nums)
        return r.choice([OBJS3 + 0x10 * r.randrange(16)] * 4 + [0])

    used, nfree = r.choice([0, 1, 1, 2, 3, 5, 8]), r.choice([0, 0, 1, 2, 4])
    if used + nfree == 0:
        nfree = 1
    total = used + nfree
    pos = [stride * k for k in r.sample(range(total), total)]
    idx, fidx = pos[:used], pos[used:]
    parent = [None] + [r.choice([None, None, r.randrange(k)]) for k in range(1, used)] if used else []
    kids = {p: [k for k in range(used) if parent[k] == p] for p in [None] + list(range(used))}
    links = [none(r) for _ in range(stride * total)]
    tail = (fidx[0] if fidx and r.random() < 0.85 else none(r))
    for k in range(used):
        i = idx[k]
        sibs = kids[parent[k]]
        at = sibs.index(k)
        links[i] = idx[sibs[at + 1]] if at + 1 < len(sibs) else tail if parent[k] is None else none(r)   # (a chain's end mostly -1)
        links[i + 1] = r.getrandbits(31) - 0x80000000
        links[i + 2] = idx[kids[k][0]] if kids[k] else -1
        links[i + 3] = idx[parent[k]] if parent[k] is not None else -1
        for c in range(ncols):
            links[i + 4 + c] = cell(c)
    for j, i in enumerate(fidx):
        links[i:i + 4] = [fidx[j + 1] if j + 1 < len(fidx) else -1, 0, -1, -2]
        for c in range(ncols):
            links[i + 4 + c] = cell(c, r.random() < 0.9)
    regions[LINKS3 - 0x20] = struct.pack('<8i', *[none(r) for _ in range(8)]) + struct.pack('<%di' % len(links), *links)
    put(window, 0x144, 'I', LINKS3)
    dwords = r.choice([len(links)] * 4 + [stride * used] * 2 + [0])
    if r.random() < 0.05:   # (no array: the dword count 0, so it is made)
        put(window, 0x144, 'I', 0)
        dwords = 0
    put(window, 0x158, 'i', dwords)
    count = r.choice([used] * 6 + [r.randrange(used + 1)])
    put(window, 0x150, 'i', count)
    roots = kids[None]
    put(window, 0x160, 'i', idx[roots[0]] if roots and r.random() < 0.95 else -1)
    put(window, 0x168, 'hh', r.randrange(max(ncols, 1)), r.choice([0, 0, 1, 1, 0x100]))   # (0x100: only the upper byte set)
    window[0x48] = (window[0x48] & ~0x10) | r.choice([0, 0x10, 0x10])
    texts, lengths = [], []
    for c in range(4):
        j = r.randrange(8)
        if bottoms[c] == 1:
            texts.append(r.choice(nums + [r.choice(nums) + 1]) & 0xffffffff)
        else:
            texts.append(r.choice([ACHARS + 0x20 * j] * 19 + [0]))
        n = struct.unpack_from('<4H', chars, 0x20 * j).index(0) if texts[-1] else 0
        lengths.append(n if c < ncols and bottoms[c] == 0 and texts[-1] else None)
    regions[ATEXTS] = struct.pack('<4I', *texts)
    fail = r.choice([0] * 8 + [1, 2])
    regions[HEAP] = heap(fail=fail)
    grows = count * stride >= dwords and ncols != 0
    first = (dwords + (20 if dwords == 0 else 10) * stride) * 4 if grows else None
    p = r.choice([-1] * 4 + [r.randrange(count)] * 4 + [count, -2] if count else [-1, -1, 0, 1])
    args = [LIST3, r.choice([ATEXTS] * 19 + [0]), r.getrandbits(32), p & 0xffffffff, r.choice([0, 1])]
    return Call(CALLS['AddItem'], args, regions, conv='t', ret='a', masks=allocator_masks(fail, lengths, first) if args[1] else [])


def row_case(r, op):
    """SetRowSize (0x663718): a row window (or none) whose inner height (its rectangle's words less the insets) is small (0 now and then:
    a fault in both), the list's words +0x13c-+0x142 near each other, the row array +0x148 there or not (the heap), the highlight +0x174 a
    recorder (slot 3: its event's rectangle hashed through +0x14) or 0. SetRow (0x663864): windowtree.py's rect_env tree with no objects
    +0x98 / +0x9c (SetRect's and SetDepth's events would share their recorders), the row its root (or none), the columns +0x164 up to its
    root and children (now and then one more: a null window, a fault in both; or -1), their words any small, the array and the columns
    there or not, the index from -2 (before the array) to past the rows shown +0x172, the list's depth word +0xe0 small or near 0x100."""
    log = log_region()
    regions = {LOG: log, HEAP: heap()}
    lst = bytearray(r.bytes(0x180))
    regions[ROWLIST] = lst
    top = r.randrange(-40, 200)
    put(lst, 0x13c, 'hhhh', r.randrange(-40, 200), top, r.randrange(-40, 300), top + r.choice([0, 7, 20, 64, 300, -10, r.randrange(-100, 400)]))
    if op == 'SetRowSize':
        row = bytearray(r.bytes(0x120))
        t = r.randrange(-30, 100)
        put(row, 0xc, 'hhhh', r.randrange(-30, 100), t, r.randrange(-30, 100), t + r.choice([2, 5, 12, 16, 20, 33, -6, 3]))
        put(row, 0xf8, 'hhhh', *[r.choice([0, 0, 1, 2, -1]) for _ in range(4)])
        regions[ROWWIN] = row
        put(lst, 0x148, 'I', r.choice([0, 0, ROWARR]))
        regions[ROWARR] = r.bytes(8)
        regions[ROWHLVT] = vtable({3: 2})
        regions[ROWHL] = struct.pack('<I', ROWHLVT)
        put(lst, 0x174, 'I', r.choice([ROWHL, ROWHL, 0]))
        blob_bytes(log, 3, 1, 0, (0x14, 0xfff))
        return Call(CALLS[op], [ROWLIST, r.choice([ROWWIN] * 12 + [0])], regions, conv='t', ret='w')
    from windowtree import rect_env
    wins = rect_env(r, regions, log)
    for w in wins:
        put(regions[w], 0x98, 'II', 0, 0)
    chain = [wins[0]]
    c = struct.unpack_from('<I', regions[wins[0]], 0x5c)[0]
    while c:
        chain.append(c)
        c = struct.unpack_from('<I', regions[c], 0x54)[0]
    ncols = r.choice(list(range(len(chain) + 1)) * 4 + [len(chain) + 1, -1])
    put(lst, 0x164, 'h', ncols)
    regions[ROWCOLS] = b''.join(struct.pack('<Ihhhh', r.getrandbits(32), r.randrange(-30, 100), r.randrange(-50, 50), r.randrange(-30, 160),
                                            r.randrange(-50, 50)) for _ in range(8))
    put(lst, 0x16c, 'I', r.choice([ROWCOLS] * 12 + [0]))
    regions[ROWARR - 8] = r.bytes(8 + 4 * 8)
    put(lst, 0x148, 'I', r.choice([ROWARR] * 12 + [0]))
    rows = r.choice([0, 1, 2, 3, 5, 7, -1])
    put(lst, 0x170, 'hh', r.choice([1, 5, 12, 16, -3, 0, r.randrange(-100, 100)]), rows)
    put(lst, 0xe0, 'h', r.choice([0, 1, 5, 0xfc, 0xfd, 0xff, 0x100, -1, r.randrange(-0x8000, 0x8000)]))
    index = r.choice([-2, -1, 0, 0, 1, 2, rows - 1, rows, rows + 1, 7]) & 0xffffffff
    return Call(CALLS[op], [ROWLIST, index, r.choice([wins[0]] * 15 + [0])], regions, conv='t', ret='v')


def highlight_case(r):
    """SetHighlight (0x662eee): the list a window of its own (0x180 random bytes) with 0-2 parents (+0x60), each shown or not (flag 0 of
    +0x48), +0x14c -1 or not, the highlight +0x174 one recorder object or none; the new one that, another or none. The objects' words
    +0x10 / +0x12 small (WindowImpl_Release answers -1 now and then: the old one's slot 0 (1) then); slot 3's event hashed by its set
    words +4-+9 and, through its +0x14, the GUI rectangle's twelve bytes (the pointer itself is a stack address, the rest unset)."""
    log = log_region()
    regions = {LOG: log, HL_VT: vtable({0: 1, 3: 2, 5: 1})}
    for obj in (HL_A, HL_B):
        regions[obj] = put(put(bytearray(r.bytes(0x20)), 0, 'I', HL_VT), 0x10, 'hh', r.choice([0, 1, 2]), r.choice([0, 1, 1, 2, 3]))
    w = bytearray(r.bytes(0x180))
    w[0x48] = (w[0x48] & 0xfe) | r.choice([0, 1, 1])
    n = r.choice([0, 1, 2])
    put(w, 0x60, 'I', HL_PARENTS if n else 0)
    for i in range(n):
        p = bytearray(r.bytes(0x70))
        p[0x48] = (p[0x48] & 0xfe) | r.choice([0, 1, 1, 1])
        put(p, 0x60, 'I', HL_PARENTS + 0x100 * (i + 1) if i + 1 < n else 0)
        regions[HL_PARENTS + 0x100 * i] = p
    put(w, 0x14c, 'i', r.choice([-1, 0, 3, -1]))
    put(w, 0x174, 'I', r.choice([HL_A, HL_A, 0]))
    regions[WINDOW] = w
    blob_bytes(log, 3, 1, 0x3f0, (0x14, 0xfff))
    return Call(CALLS['SetHighlight'], [WINDOW, r.choice([HL_A, HL_B, HL_B, HL_B, 0])], regions, conv='t', ret='v')


def send_walk(regions):
    """SendToRows' reads of the items for 0x1e in Python (the first shown item +0x160, then one per row while not -1): False when one
    leaves the links"""
    window = regions[LIST]
    arr = list(struct.unpack('<%di' % (len(regions[LINKS2 - 4 * PAD]) // 4), regions[LINKS2 - 4 * PAD]))
    if not struct.unpack_from('<I', window, 0x148)[0]:
        return True

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
    item = struct.unpack_from('<i', window, 0x160)[0]
    try:
        for _ in range(struct.unpack_from('<h', window, 0x172)[0]):
            if item != -1:
                get(item + 1)
                item = nxt(item)
    except IndexError:
        return False
    return True


def show_case(r):
    """ShowRows (0x664fde): window.py's shown_env windows as the rows (some 0) of a list of its own (0x180 random bytes): the rows shown
    +0x172 0-5 (or -1), the array +0x148 there or not, the first shown +0x154 and count +0x150 about each other (the edges too), the
    highlight +0x174 a recorder (slot 5) or none, +0x14c -1 or not; shown 0, 1 or any (the upper half random)."""
    log = log_region()
    regions = {LOG: log}
    wins = shown_env(r, regions, log)
    w = bytearray(r.bytes(0x180))
    rows = r.choice([0, 1, 2, 3, 4, 5, -1])
    put(w, 0x172, 'h', rows)
    regions[ROWARR] = b''.join(struct.pack('<I', r.choice(wins + [0])) for _ in range(6))
    put(w, 0x148, 'I', r.choice([ROWARR] * 8 + [0]))
    first = r.choice([0, 0, 1, 3, -1, 0x7fffffff, r.getrandbits(32)])
    put(w, 0x150, 'II', r.choice([first + 2, first, first + 5, 0, 3, -1, r.getrandbits(32)]) & 0xffffffff, first & 0xffffffff)
    put(w, 0x14c, 'i', r.choice([-1, 0, 2]))
    regions[ROWHLVT] = vtable({5: 1})
    regions[ROWHL] = struct.pack('<I', ROWHLVT)
    put(w, 0x174, 'I', r.choice([ROWHL, ROWHL, 0]))
    regions[ROWLIST] = w
    shown = r.choice([0, 1, 1, 0x10000, r.getrandbits(16)]) | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0)
    return Call(CALLS['ShowRows'], [ROWLIST, shown], regions, conv='t', ret='v')


def sort_case(r):
    """Sort (see the header)"""
    window = bytearray(r.bytes(0x180))
    regions = {SORTW: window}
    ncols = r.choice([1, 2, 3])
    stride = 4 + ncols
    k = r.randint(1, 10)
    kids = r.randint(0, 3)
    n = k + kids
    idx = [stride * j for j in r.sample(range(n), n)]
    top, rest = idx[:k], idx[k:]
    links = [r.getrandbits(32) for _ in range(stride * n + 8)]
    for j, i in enumerate(top):
        links[i] = top[j + 1] if j + 1 < k else -1
        links[i + 2] = -1
        links[i + 3] = r.choice([-1] * 12 + [-2, -5])
    for i in rest:   # (children of top items)
        p = r.choice(top)
        links[i], links[i + 2], links[i + 3] = -1, -1, p
        links[p + 2] = i
    bottoms = [r.choice([1, 1, 0, 2]) for _ in range(4)]
    texts = [tuple(r.choice(ALPHA) for _ in range(r.choice([0, 1, 1, 2, 3]))) for _ in range(6)]
    texts[1] = texts[0] + texts[1][:1]   # (a prefix)
    chars = bytearray(0x200)
    for j, t in enumerate(texts):
        struct.pack_into('<%dH' % len(t), chars, 0x20 * j, *t)
    regions[SORTCHARS] = chars
    strs = bytearray(r.bytes(0x10 * 8))
    for j in range(8):
        t = r.randrange(6)
        data = r.choice([SORTCHARS + 0x20 * t] * 5 + [0])
        put(strs, 0x10 * j + 4, 'II', data, len(texts[t]) if data else 0)
    regions[SORTSTRS] = strs
    nums = r.choice([[0, 1, -1, 5], [0x7fffffff, -0x80000000, 0, 3], [2, 2, 7]])
    for i in idx:
        for c in range(ncols):
            links[i + 4 + c] = r.choice(nums) if bottoms[c] == 1 else SORTSTRS + 0x10 * r.randrange(8)
    regions[SORTLINKS - 0x40] = r.bytes(0x40) + struct.pack('<%dI' % len(links), *[v & 0xffffffff for v in links])
    put(window, 0x144, 'I', SORTLINKS)
    put(window, 0x150, 'i', r.choice([k, k, k, k - 1, k + 2, 0, 1, 2, 3]))
    put(window, 0x160, 'i', top[0])
    column = r.choice(list(range(ncols)) * 4 + [-1])   # (-1: the parents, as numbers: the column before the array's bottom 1)
    regions[SORTCOLS - 0xc] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', v) for v in [1 if column == -1 else r.choice([0, 1])] + bottoms)
    put(window, 0x168, 'hh', column, r.choice([0, 0, 0, 1, 1, 1, 2]))
    put(window, 0x16c, 'I', r.choice([SORTCOLS] * 12 + [0]))
    return Call(CALLS['Sort'], [SORTW, r.choice([0xffffffff, r.getrandbits(32)])], regions, conv='t', ret='v')


def bar_scroll_case(r):
    """ScrollFromBar (0x664ea3): see the top"""
    c = refresh_case(r)
    regions, log, window = c.regions, c.regions[LOG], c.regions[LIST]
    first = struct.unpack_from('<i', window, 0x154)[0]
    set_return(log, 8, first)
    n = r.choice([0, 1, 1, 1, 2])
    kids = [BARS + 0x100 * i for i in range(n)]
    for i, kid in enumerate(kids):
        buf = bytearray(r.bytes(0x100))
        put(buf, 0, 'I', VTABLE)
        put(buf, 0x54, 'I', kids[i + 1] if i + 1 < n else 0)
        put(buf, 0x8c, 'I', 1 if i == n - 1 and r.random() < 0.9 else r.choice([2, 3, r.getrandbits(32)]))
        regions[kid] = buf
    put(window, 0x5c, 'I', kids[0] if kids else 0)
    if r.random() < 0.85:
        put(window, 0x154, 'i', r.choice([first + 1, first - 1, r.getrandbits(31)]))
    window[4] = r.choice([0, 0, 0, 1])   # (going or not: posts to it refused)
    queues(r, regions)
    regions[0xfaa5d4] = struct.pack('<I', r.choice([0, 1]))
    return Call(CALLS['ScrollFromBar'], [LIST], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'Sort':
        return sort_case(r)
    if op == 'SetHighlight':
        return highlight_case(r)
    if op == 'ShowRows':
        return show_case(r)
    if op == 'SendToRows':
        c = refresh_case(r)
        message = r.choice([0x1e] * 5 + [3, 0x15, 7, 0x80000001])
        if message == 0x1e and not send_walk(c.regions):   # (an item read off the links: a wild read the emulator faults on, not the driver)
            message = 3
        return Call(CALLS[op], [LIST, message, r.getrandbits(32), r.getrandbits(32)], c.regions, conv='t', ret='v')
    if op in ('SetRowSize', 'SetRow'):
        return row_case(r, op)
    if op == 'AddItem':
        return add_case(r)
    if op in ('PostSelection', 'NotifySort', 'SetSortColumn', 'SetSortOrder'):
        return notify_case(r, op)
    if op == 'ColumnSpan':
        return column_case(r)
    if op in ('RemoveItem', 'Clear', 'ScrollTo', 'OnKey'):
        return list_case(r, op)
    if op == 'RefreshRows':
        return refresh_case(r)
    if op == 'ScrollFromBar':
        return bar_scroll_case(r)
    if op == 'PostColumnAt':
        window = bytearray(r.bytes(0x180))
        regions = {LIST: window, OWNER: put(bytearray(r.bytes(0x20)), 4, 'B', r.choice([0, 0, 0, 1]))}
        put(window, 0x14c, 'i', r.choice([-1, 0, 0, 2, r.randrange(-0x80000000, 0x80000000)]))
        ncols = r.choice([0, 1, 2, 3, 4, 4, -1])
        put(window, 0x164, 'h', ncols)
        edges = []
        for _ in range(4):
            left = r.choice([r.randrange(-20, 200), r.randrange(-0x8000, 0x8000)])
            edges.append((left, (r.choice([left + r.randrange(-2, 60), r.randrange(-0x8000, 0x8000)]) + 0x8000 & 0xffff) - 0x8000))
        regions[COLS2] = b''.join(struct.pack('<Ihhhh', r.getrandbits(32), left, r.getrandbits(15), right, r.getrandbits(15)) for left, right in edges)
        put(window, 0x16c, 'I', COLS2)
        put(window, 0x60, 'I', r.choice([OWNER, OWNER, 0]))
        queues(r, regions)
        left, right = r.choice(edges)
        x = r.choice([left, right, left + 1, right - 1, (left + right) // 2, r.getrandbits(16)]) & 0xffff
        return Call(CALLS[op], [LIST, x | r.getrandbits(16) << 16, r.getrandbits(32)], regions, conv='t', ret='v')
    if op == 'Cell':
        c = refresh_case(r)
        window = c.regions[LIST]
        nrows, ncols = struct.unpack_from('<h', window, 0x172)[0], struct.unpack_from('<h', window, 0x164)[0]
        arr = bytearray(c.regions[ROWS2])
        if nrows > 0 and r.random() < 0.2:
            put(arr, 4 * r.randrange(nrows), 'I', 0)
        for k in range(max(nrows, 0)):   # (now and then a row without children: its walk reaches 0 early)
            w = struct.unpack_from('<I', arr, 4 * k)[0]
            if w in c.regions and r.random() < 0.25:
                put(c.regions[w], 0x5c, 'I', 0)
        c.regions[ROWS2] = arr
        has = struct.unpack_from('<I', window, 0x148)[0] != 0
        row = r.choice([r.randrange(max(nrows, 0) + 1), 0, nrows if nrows > 0 else 0]) if has else 0
        column = r.randrange(max(ncols, 0) + 2) | r.choice([0, r.getrandbits(16)]) << 16
        return Call(CALLS[op], [LIST, row, column], c.regions, conv='t', ret='a')
    if op == 'FitBar':
        return bar_case(r)
    if op == 'Select':
        return select_case(r)
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
    if op == 'FindText':
        return text_case(r, window, regions)
    if op == 'CopyCells':
        ncols = r.choice([0, 1, 2, 3, 4, 4, -1])
        put(window, 0x164, 'h', ncols)
        bottoms = [r.choice([0, 0, 0, 1, 2]) for _ in range(4)]
        regions[COLUMNS] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', v) for v in bottoms)
        put(window, 0x16c, 'I', COLUMNS)
        chars, texts, lengths = bytearray(0x400), [], []
        for c in range(4):
            n = r.choice([0, 1, 5, 31, 32, r.randrange(41)])
            struct.pack_into('<%dH' % (n + 1), chars, 0x100 * c, *[r.randrange(1, 0x10000) for _ in range(n)], 0)
            text = r.choice([WCHARS + 0x100 * c] * 4 + [0]) if bottoms[c] == 0 else r.getrandbits(32)
            texts.append(text)
            lengths.append(n if c < ncols and bottoms[c] == 0 and text else None)
        regions[WCHARS] = chars
        regions[OUT] = struct.pack('<4I', *texts)
        regions[OUT + 0x40] = r.bytes(0x10)
        fail = r.choice([0] * 8 + [1, 2])
        regions[HEAP] = heap(fail=fail)
        return Call(CALLS[op], [WINDOW, OUT, OUT + 0x40], regions, conv='t', ret='v', masks=allocator_masks(fail, lengths))
    if op == 'SortsBefore':
        bottoms = [r.choice([1, 1, 0, 2, -1]) for _ in range(6)]
        regions[COLUMNS - 0x18] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', v) for v in bottoms)
        put(window, 0x16c, 'I', COLUMNS)
        column = r.choice([0, 1, 2, 3, 0, 1, 2, 3, -1, -2])
        put(window, 0x168, 'hh', column, r.choice([0, 0, 1, 1, 0x100, -0x8000, r.getrandbits(16) - 0x8000]))   # (0x100: only the upper byte set)
        texts = [tuple(r.choice(ALPHA + [0x7fff, 0x8000]) for _ in range(r.choice([0, 1, 2, 3]))) for _ in range(5)]
        chars = bytearray(0x200)
        for j, t in enumerate(texts):
            struct.pack_into('<%dH' % len(t), chars, 0x20 * j, *t)
        regions[WCHARS] = chars
        strs = bytearray(r.bytes(0x10 * 8))
        for j in range(8):
            put(strs, 0x10 * j + 4, 'I', r.choice([WCHARS + 0x20 * r.randrange(5)] * 3 + [0]))
        regions[WSTRS] = strs
        nums = [0, 1, -1, 5, 0x7fffffff, -0x80000000]
        a = [WCHARS + 0x20 * r.randrange(5) for _ in range(6)]
        b = [r.choice([WSTRS + 0x10 * r.randrange(8)] * 6 + [0]) for _ in range(6)]
        if bottoms[column + 2] == 1:   # (numbers: around each other, or the strings' addresses now and then)
            x = r.choice(nums)
            a[column + 2] = r.choice([x, x + 1, x - 1, r.choice(nums), a[column + 2]]) & 0xffffffff
            b[column + 2] = r.choice([x, x, r.choice(nums), b[column + 2]]) & 0xffffffff
        regions[OUT - 8] = struct.pack('<6I', *a)
        regions[OUT + 0x40 - 8] = struct.pack('<6I', *b)
        return Call(CALLS[op], [WINDOW, OUT, OUT + 0x40], regions, conv='t', ret='a')
    if op == 'Grow':   # (items() of 4 + 0-3 columns (now and then -1 or -3), some free; the dword count theirs or less; the heap failing now and then)
        ncols = r.choice([0, 1, 2, 3, 3, -1, -3])
        put(window, 0x164, 'h', ncols)
        idx = items(r, window, regions, stride=4 + max(ncols, 0))
        links = bytearray(regions.pop(LINKS))
        for i in idx:
            if r.random() < 0.3:
                put(links, 4 * (i + 3), 'i', -2)
        regions[LINKS - 0x20] = r.bytes(0x20) + links
        put(window, 0x158, 'i', r.choice([len(links) // 4] * 3 + [4 * len(idx), r.randrange(len(links) // 4)]))
        put(window, 0x160, 'i', r.choice(idx + [idx[0], -1]))
        if r.random() < 0.1:
            put(window, 0x144, 'I', 0)
        regions[HEAP] = heap(fail=r.choice([0] * 9 + [1]))
        return Call(CALLS[op], [WINDOW, r.choice([1, 2, 3, 10, 20, 20, 0, -1]) & 0xffffffff], regions, conv='t', ret='a')
    if op == 'NthItemCell':
        idx = items(r, window, regions, stride=8)
        links = bytearray(regions[LINKS])
        cells = [CELLS + 8 * j for j in range(8)]
        regions[CELLS] = b''.join(struct.pack('<II', r.getrandbits(32), r.choice([0, r.getrandbits(32)])) for _ in cells)
        for i in idx:
            for c in range(4):
                put(links, 4 * (i + 4 + c), 'I', r.choice(cells))
        regions[LINKS] = links
        regions[COLS2] = b''.join(put(bytearray(r.bytes(0xc)), 0xa, 'h', r.choice([0, 0, 0, 1, -1])) for _ in range(4))
        put(window, 0x16c, 'I', COLS2)
        put(window, 0x164, 'h', r.choice([0, 1, 2, 3, 4, 4, -1]))
        put(window, 0x150, 'i', r.choice([0, 1, len(idx), len(idx) + 3, r.randrange(len(idx) + 1), -1]))
        put(window, 0x160, 'i', r.choice(idx + [idx[0], -1]))
        if r.random() < 0.1:
            put(window, 0x144, 'I', 0)
        regions[OUT] = r.bytes(4)
        n = r.choice([-1, 0, 1, 2, r.randrange(len(idx) + 2), len(idx) - 1, len(idx)])
        return Call(CALLS[op], [WINDOW, n & 0xffffffff, r.choice([0, 1, 2, 3, 4]), OUT], regions, conv='t', ret='w')
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
    put(window, 0x160, 'i', r.choice(idx + [idx[0], -1, -2]))   # (-2: a head that is no item but not none, read as one)
    if r.random() < 0.1:
        put(window, 0x144, 'I', 0)
    n = r.choice([-1, 0, 1, 2, 3, r.randrange(len(idx) + 2), len(idx) - 1, len(idx), 0x7fffffff, -0x80000000])
    if op == 'SetNthItemData':
        return Call(CALLS[op], [WINDOW, n & 0xffffffff, r.getrandbits(32)], regions, conv='t', ret='w')
    if op == 'NthItemData':
        regions[OUT] = r.bytes(4)
        return Call(CALLS[op], [WINDOW, n & 0xffffffff, r.choice([OUT, OUT, 0])], regions, conv='t', ret='a')
    if op == 'NthChild':
        parent = r.choice([-1, -1, -2, 0, 0, 1, 2, r.randrange(len(idx) + 2)])
        return Call(CALLS[op], [WINDOW, n & 0xffffffff, parent & 0xffffffff], regions, conv='t', ret='a')
    return Call(CALLS[op], [WINDOW, n & 0xffffffff], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

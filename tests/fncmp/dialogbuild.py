# The GUI builders that dress a window and its children (list rows' initialisers in src/game/ui/listrows, dialogs' builders in
# src/game/ui/dialogs) against the originals.
# The list rows, chain(): listrows.py's init_case() (rendererpart.py's surroundings of a text part's construction, the heap, the fonts of
# group 0-2, the GUI's size words, the renderer), the window under test ROW and up to six children (first +0x5c, then +0x54) of random
# bytes, each with its rectangle +0x2c small, its impl +0x9c none (a new window) or an old one (released or deleted), its object +0x98
# none or a recorder, +0x4c none, an object or a font, shown (+0x48 bit 0, up its parents) or not, its parent ROW, its vtable a recorder
# (slots 5 and 7); the text parts' unported slots (3, their layout, but for TextPart_Ctor's own, and 9, their text) recorders in every
# kind (their events: stack pointers, logged as a hash of none of their bytes, so which bytes an event carries is windownotify.py's to
# check, not this bench's); the GUI's registered windows (listrows.py's register()) with the name looked up among them or not; the font
# dwords the builders read 0-12, 7 mostly. The heap fails one of the parts now and then (never one of a part's own allocations: those
# fault in both). Each window's fields after the call are compared.
# The dialogs, scripted(): listrows.py's init_case() surroundings of the parts (no row), windowscript.py's of a script's build
# (windowfactory.py's tree, queues, timers and focus; the parent window WINDOW2 as windowscript.py's Run gets it, its rectangles 100-300
# wide, the frame [0xf82884]'s window +4), a heap large enough for the parts' pixels (failing now and then), the dialog's global none
# (mostly) or made, the globals it writes random, and the script at the builder's address generated: 0-2 plain children (kinds 1-13 with
# windowfactory.py's flags, small rectangles, no opcodes of their own), then the dialog (kind 1, up to 160 wide) whose children are the
# texts it looks for (kind 1, inside it: other kinds' rectangles or edges outside give parts more pixels than the heap holds), in any
# order, among 0-2 others, now and then one of them missing (a fault in both). The new windows' fields are compared through the heap.
# HelpBg_Build (0x492180, src/game/frontend/helpbg), helpbg(): the dialogs' surroundings without the script, the GUI root [0xfaa5d8]
# windowfactory.py's (or none: a fault in both), font 7's vtable a recorder whose slot 1 gives its height (0-40 mostly, edges too), the
# GUI's registered windows of listrows.py's register() with "f_helpbg" among them or not, the background's bytes random.
# usage: dialogbuild.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=List7cc94c_InitRow,HelpBg_Build,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import listrows  # noqa: E402
import window  # noqa: E402
import windowfactory  # noqa: E402
import windowscript  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, put, recorder, set_return, vtable  # noqa: E402

CALLS = {'List7cc94c_InitRow': 0x4c03e0, 'List7cca90_InitRow': 0x4c1c30, 'List7cc200_InitRow': 0x4ad340, 'List7cb264_InitRow': 0x4a03d0,
         'Dialog7cc4c4_Create': 0x4b9a70, 'HelpBg_Build': 0x492180}
# each list row's initialiser: (the children it dresses, the GUI name it looks up or None, the font dword it reads or None)
SHAPES = {'List7cc94c_InitRow': (5, None, None), 'List7cca90_InitRow': (4, None, None), 'List7cc200_InitRow': (2, 'i_boxtick', 0x755134),
          'List7cb264_InitRow': (5, None, 0x752934)}
# each dialog builder: (its global, the script's address, the texts' ids, the other globals it writes)
DIALOGS = {'Dialog7cc4c4_Create': (0x7cc4c4, 0x7581a0, (0x16c11, 0x16c12), (0x7cc4e4,))}
ALL_RANGES = sorted(set(window.RANGES + listrows.PART_RANGES + listrows.REG_RANGES + windowscript.RANGES + [
    (0x48f6a0, 0x491aac), (0x65c388, 0x65c55a), (0x65d56b, 0x65d58c), (0x65d773, 0x65d7cc), (0x6616b0, 0x661710), (0x6617f0, 0x661820),
    (0x65e526, 0x65e549), (0x4c03e0, 0x4c0653), (0x4c1c30, 0x4c1e33), (0x4ad340, 0x4ad46d), (0x4a03d0, 0x4a06a4), (0x4b9a70, 0x4b9bf6), (0x492180, 0x4922d0),
    (0x65e6d5, 0x65e705), (0x65f16b, 0x65f246), (0x65f6d1, 0x65fba8), (0x65b8dc, 0x65bb10), (0x65bb3f, 0x65bc24), (0x65dcaf, 0x65dd18),
    (0x680050, 0x68005b)]))
B = Bench(ALL_RANGES)
ROW = listrows.ROW
KIDS, KIDVT = RAM + 0xb000, RAM + 0xbc00
KID = 0x140
FRAME = RAM + 0xc000
HELPBG, FONTVT = RAM + 0xc100, RAM + 0xc200
PART_KINDS = (0x6ff320, 0x6ff350, 0x6ff380, 0x6ff3b0, 0x6ff3e0, 0x6ff410, 0x6ff440, 0x6ff470, 0x6ff4a0, 0x6ff4d0, 0x6ff500, 0x6ff530)


def kid(r, parent):
    """a child window: random bytes, a recorder vtable, its rectangle small, +0x9c / +0x98 / +0x4c / shown as the module comment says"""
    w = bytearray(r.bytes(KID))
    put(w, 0, 'I', KIDVT)
    left, top = r.randrange(-5, 30), r.randrange(-5, 30)
    put(w, 0x30, '4h', left, top, left + r.randrange(0, 40), top + r.randrange(0, 40))
    put(w, 0x9c, 'I', listrows.OLD if r.random() < 0.15 else 0)
    put(w, 0x98, 'I', r.choice([0, 0, 0, listrows.OBJ98]))
    put(w, 0x4c, 'I', r.choice([0, 0, listrows.OBJ4C]))
    put(w, 0x48, 'I', (r.getrandbits(32) & ~1) | r.choice([0, 1, 1]))
    put(w, 0x60, 'I', parent)
    return w


def chain(r, op, kids, name=None, font=None):
    """the shared surroundings (above): ROW with `kids` children, `name` registered or not, the font dword `font`"""
    regions = dict(listrows.init_case(r, 'HilightList_InitRow').regions)
    regions[HEAP] = heap(0x6000, r.choice([0] * 8 + [1]))
    put(regions[ROW], 0, 'I', KIDVT)
    for k in range(kids):
        w = kid(r, ROW)
        put(w, 0x54, 'I', KIDS + KID * (k + 1) if k + 1 < kids else r.choice([0, 0, KIDS + KID * k]))
        regions[KIDS + KID * k] = w
    put(regions[ROW], 0x5c, 'I', KIDS if kids else 0)
    regions[KIDVT] = vtable({5: 1, 7: 1})
    regions[listrows.VT98] = vtable({0: 1, 3: 2, 5: 1})   # (an old object +0x98 released: WindowImpl_Release's slot 5)
    regions[listrows.VTOLD] = vtable({3: 2, 5: 1})   # (an old impl +0x9c kept, as beside a new object +0x98, is told too)
    for vt in PART_KINDS:
        if vt != 0x6ff320:
            regions[vt + 0xc] = struct.pack('<I', recorder(3, 2))
        regions[vt + 0x24] = struct.pack('<I', recorder(9, 1))
    if name:
        listrows.register(r, regions, name)
    if font:
        regions[font] = struct.pack('<I', r.choice([7] * 4 + list(range(13))))
    return regions


def small_rect(r, w, size=40):
    left, top = r.randrange(0, 40), r.randrange(0, 40)
    w += [left, top, left + r.randrange(0, size), top + r.randrange(0, size)]


def child(r, w, ident, script=(5,), size=40, kind=None):
    """opcode 0: a child of a kind 1-13 (1 mostly), windowfactory.py's flags, `ident`, a small rectangle, then its script"""
    kind = kind or r.choice([1] * 6 + list(range(2, 14)))
    w.append(0)
    w.append(kind)
    windowscript.dword(r, w, windowfactory.no_handlers(kind, r.choice(windowscript.FLAGS)))
    windowscript.dword(r, w, ident)
    small_rect(r, w, size)
    w += list(script)


def scripted(r, op):
    """the dialogs' surroundings (above); returns the regions"""
    address, at, texts, written = DIALOGS[op]
    regions = dict(listrows.init_case(r, 'HilightList_InitRow').regions)
    for k in (ROW, listrows.OLD, listrows.VTOLD, listrows.OBJ98, listrows.VT98, listrows.OBJ4C, listrows.PARENTS, listrows.PARENTS + 0x80):
        regions.pop(k)
    log = regions[LOG]
    regions[window.VTABLE] = vtable({5: 1})
    args = windowfactory.ctor_args(r, regions)
    windowfactory.timer_env(r, regions, log)
    regions[HEAP] = heap(0x80000, r.choice([0] * 16 + [r.randrange(1, 16)]))   # (the parts' pixels: their windows' width times height)
    regions[0xfaa5ec] = struct.pack('<I', r.choice([args[0], 0, 0]))
    regions[0xfaa610] = struct.pack('<I', 0)
    regions[0x6ff320 + 0x24] = struct.pack('<I', recorder(9, 1))
    win = regions[window.WINDOW2] = bytearray(regions[window.WINDOW2])   # (windowscript.py's Run's parent)
    set_return(log, 3, r.choice([4, 4, 7, 7, 1, 2, 11]))
    put(win, 0, 'I', window.VTABLE)
    put(win, 8, 'I', 0x705920)
    put(win, 0x2c, 'I', 0x705920)
    for edge in (0xc, 0x30):   # (its rectangles small: its children's parts' pixels from them)
        left, top = r.randrange(-40, 40), r.randrange(-40, 40)
        put(win, edge, '4h', left, top, left + r.randrange(100, 300), top + r.randrange(100, 300))
    put(win, 0x44, 'I', r.choice([window.WINDOW2 + 8, 0]))
    put(win, 0x48, 'I', r.choice([1, 0x401, 0x400, 0]) | (r.getrandbits(32) & 0x0ffff800 if r.random() < 0.3 else 0))
    put(win, 0x5c, 'I', 0)
    put(win, 0x60, 'I', r.choice([args[0], 0]))
    put(win, 0x98, 'I', 0)
    put(win, 0x9c, 'I', 0)
    regions[FRAME] = put(bytearray(r.bytes(8)), 4, 'I', window.WINDOW2)
    regions[0xf82884] = struct.pack('<I', FRAME)
    regions[address] = struct.pack('<I', r.choice([0] * 9 + [r.getrandbits(32) | 1]))
    for k in written:
        regions[k] = r.bytes(4)
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    inner = []
    ids = list(texts) + [r.choice([0, 1, texts[0] + 0x10, r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2]))]
    if r.random() < 0.05:
        ids.remove(r.choice(texts))
    r.shuffle(ids)
    for ident in ids:
        child(r, inner, ident, kind=1 if ident in texts else None)
    child(r, w, r.choice([0, 1, r.getrandbits(32)]), inner + [5], 120, 1)
    w += [5] * 4
    regions[at] = struct.pack('<%dH' % len(w), *w)
    return regions


def helpbg(r):
    """HelpBg_Build's surroundings (above); returns the regions"""
    regions = dict(listrows.init_case(r, 'HilightList_InitRow').regions)
    for k in (ROW, listrows.OLD, listrows.VTOLD, listrows.PARENTS, listrows.PARENTS + 0x80):
        regions.pop(k)
    log = regions[LOG]
    regions[window.VTABLE] = vtable({5: 1})
    windowfactory.ctor_args(r, regions)
    root = struct.unpack_from('<I', regions[0xfaa5d8])[0]
    if root in (0, window.WINDOW2) and r.random() < 0.95:   # (the root WINDOW2 with no children, as scripted()'s; none now and then)
        regions[0xfaa5d8] = put(bytearray(regions[0xfaa5d8]), 0, 'I', window.WINDOW2)
    win = regions[window.WINDOW2] = bytearray(regions[window.WINDOW2])
    put(win, 0, 'I', window.VTABLE)
    put(win, 0x48, 'I', r.choice([1, 0x401, 0x400, 0]))
    put(win, 0x5c, 'I', 0)
    put(win, 0x60, 'I', 0)
    windowfactory.timer_env(r, regions, log)
    regions[HEAP] = heap(0x14000, r.choice([0] * 16 + [r.randrange(1, 8)]))
    regions[0x6ff320 + 0x24] = struct.pack('<I', recorder(9, 1))
    lang = regions[0x78b530] = bytearray(regions[0x78b530])
    group = struct.unpack_from('<I', lang, 4)[0]
    put(lang, 0x1c + (7 + 13 * group) * 0x18, 'I', FONTVT)
    regions[FONTVT] = vtable({1: 0})
    height = r.choice([r.randrange(0, 24)] * 30 +[0, 0x7fff, 0x8000, 0xffff, 0xfff0, r.getrandbits(16)])   # (big ones: pixels past the heap, a fault in both)
    set_return(log, 1, height | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0))
    listrows.register(r, regions, 'f_helpbg')
    regions[HELPBG] = r.bytes(0x20)
    return regions


def case(r):
    op = r.deal(OPS)
    if op == 'HelpBg_Build':
        return Call(CALLS[op], [HELPBG], helpbg(r), conv='t', ret='v')
    if op in DIALOGS:
        return Call(CALLS[op], [], scripted(r, op), ret='v')
    kids, name, font = SHAPES[op]
    return Call(CALLS[op], [ROW], chain(r, op, kids, name, font), ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

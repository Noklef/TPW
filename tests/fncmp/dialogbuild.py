# The GUI builders that dress a window and its children (list rows' initialisers in src/game/ui/listrows, dialogs' builders) against the
# originals. The shared surroundings, chain(): listrows.py's init_case() (rendererpart.py's surroundings of a text part's construction, the
# heap, the fonts of group 0-2, the GUI's size words, the renderer), the window under test ROW and up to six children (first +0x5c, then
# +0x54) of random bytes, each with its rectangle +0x2c small, its impl +0x9c none (a new window) or an old one (released or deleted), its
# object +0x98 none or a recorder, +0x4c none, an object or a font, shown (+0x48 bit 0, up its parents) or not, its parent ROW, its vtable
# a recorder (slots 5 and 7); the text parts' unported slots (3, their layout, but for TextPart_Ctor's own, and 9, their text) recorders in
# every kind (their events: stack pointers, logged as a hash of none of their bytes); the GUI's registered windows (listrows.py's
# register()) with the name looked up among them or not; the font dwords the builders read 0-12, 7 mostly. The heap fails one of the parts
# now and then (never one of a part's own allocations: those fault in both). Each window's fields after the call are compared.
# usage: dialogbuild.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=List7cc94c_InitRow,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import listrows  # noqa: E402
import window  # noqa: E402
import windowfactory  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, put, recorder, set_return, vtable  # noqa: E402

CALLS = {'List7cc94c_InitRow': 0x4c03e0, 'List7cca90_InitRow': 0x4c1c30, 'List7cc200_InitRow': 0x4ad340, 'List7cb264_InitRow': 0x4a03d0}
# each builder: (the children it dresses, the GUI name it looks up or None, the font dword it reads or None)
SHAPES = {'List7cc94c_InitRow': (5, None, None), 'List7cca90_InitRow': (4, None, None), 'List7cc200_InitRow': (2, 'i_boxtick', 0x755134),
          'List7cb264_InitRow': (5, None, 0x752934), 'HelpBg_Build': (0, 'f_helpbg', None)}
B = Bench(sorted(set(window.RANGES + listrows.PART_RANGES + listrows.REG_RANGES + [
    (0x48f6a0, 0x491aac), (0x65c388, 0x65c55a), (0x65d56b, 0x65d58c), (0x65d773, 0x65d7cc), (0x6616b0, 0x661710), (0x6617f0, 0x661820),
    (0x65e526, 0x65e549), (0x4c03e0, 0x4c0653), (0x4c1c30, 0x4c1e33), (0x4ad340, 0x4ad46d), (0x4a03d0, 0x4a06a4),
    (0x492180, 0x4922d0), (0x65e6d5, 0x65e72f), (0x65f16b, 0x65f256)] + windowfactory.FACTORY)))
ROW = listrows.ROW
KIDS, KIDVT = RAM + 0xb000, RAM + 0xbc00
KID = 0x140
HELP, FONTVT = RAM + 0xc000, RAM + 0xc100
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


def created(r, op):
    """the builders that make their window (CreateChild on the GUI root [0xfaa5d8]): chain()'s surroundings without ROW, listrows.py's
    factory surroundings (window.py's tree and queues, windowfactory.py's timers, the heap, the GUI root one of the tree's, now and then
    none: a fault in both), font 7's height (its slot 1, the GUI's size word as chain()'s) any word, small mostly"""
    regions = chain(r, op, 0, *SHAPES[op][1:])
    regions.pop(ROW)
    log = regions[LOG]
    wins = window.tree(r, regions)
    window.queues(r, regions)
    regions[window.VTABLE] = vtable({5: 1})
    windowfactory.timer_env(r, regions, log)
    regions[HEAP] = heap(0x6000, r.choice([0] * 10 + [1, 2]))
    regions[0xfaa540] = struct.pack('<II', r.randrange(-10, 120) & 0xffffffff, r.randrange(-10, 40) & 0xffffffff)
    regions[0xfaa5d8] = struct.pack('<5I', r.choice([wins[0]] * 9 + [0]), r.getrandbits(32), r.choice(wins + [0, 0]), r.getrandbits(32),
                                    r.choice([0] * 6 + [1]))
    regions[0xfaa5ec] = struct.pack('<I', r.choice([wins[0], 0, 0]))
    lang = regions[0x78b530]
    put(lang, 0x1c + (7 + 13 * lang[4]) * 0x18, 'I', FONTVT)
    regions[FONTVT] = vtable({})
    set_return(log, 1, r.choice([r.randrange(0, 40)] * 4 + [0x7fff, 0x8000, 0xffff, r.getrandbits(16)]) | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0))
    return regions


def help_case(r, op):
    """HelpBg_Build: created()'s surroundings, the help background HELP of random bytes, "f_helpbg" registered or not"""
    regions = created(r, op)
    regions[HELP] = r.bytes(0x20)
    return Call(CALLS[op], [HELP], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'HelpBg_Build':
        return help_case(r, op)
    kids, name, font = SHAPES[op]
    return Call(CALLS[op], [ROW], chain(r, op, kids, name, font), ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

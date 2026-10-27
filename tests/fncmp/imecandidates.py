# The IME's candidate list's opener (src/game/ui/imecandidates: 0x480b00, thiscall on the host) against the original. dialogbuild.py's
# scripted() surroundings (the parent WINDOW2, the argument) with the script at 0x74f9e0 generated: 0-2 plain children, then the window
# (kind 1, 0-160 wide and high, its left and top small or about the limits 0x7f8 / 0x5f8 less its size) whose children are the list
# 0x7b79e3 (kind 7: one column, 0 or 2 now and then (the rows have no children: a fault in both); its inner rectangle 0-170 high, the rows
# the "hilight" list's: font 7's height (its slot 1, 20-80) * 0x600 / [0xfaa5c0] (about 1536) + 6) among 0-2 decoys; the list missing
# now and then (a fault in both). The window kinds' slots 5 and 7 and the parts' slots 3, 5 and 9 recorders (game code the driver cannot
# run natively); the GUI's registered windows ("hilight" among others or not); the GUI root [0xfaa5d8] the parent mostly. The host: its
# window +4 ANCHOR, whose rectangle +8 (insets 0) puts the new window where it already is, below, above or beside it (a move or a resize
# tells the new window, whose handler 0x480d10 is unported, stored raw and below g_game: the driver cannot run it; so the move left past
# 0x7f8 is not compared); its IME +0x1c with no current entry (+0x54 -1) or one cached already with no strings: the strings are never
# added (an item added tells the new window too), so the loop over them is not compared.
# usage: imecandidates.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Open)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7ca150  # noqa: E402
import dialogbuild  # noqa: E402
import panelparts  # noqa: E402
import window  # noqa: E402
import windowfactory  # noqa: E402
import windowscript  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, global_heap, heap, put, recorder, set_return, vtable  # noqa: E402

CALLS = {'Open': 0x480b00}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(sorted(set(dialog7ca150.ALL_RANGES + [(0x480b00, 0x480d05), (0x480d30, 0x480dc3), (0x480dd0, 0x480e6a), (0x47a9b0, 0x47ab92),
                                                (0x47a960, 0x47a975), (0x659409, 0x65959a), (0x659e83, 0x65a1d3), (0x65a3e9, 0x65a4a3),
                                                (0x66c01a, 0x66c7dc), (0x659730, 0x659760), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0),
                                                (0x65d547, 0x65d56a), (0x66bff7, 0x66c01b), (0x665bfb, 0x665c45)])),   # (Gui_TopDepth's: guidepth.py's)
          hooks=panelparts.animatormesh.HOOKS)
FONTVT = dialog7ca150.FONTVT
HOST, ANCHOR, ENTRIES = RAM + 0x58000, RAM + 0x58400, RAM + 0x58600
LIST = 0x7b79e3


def list_script(r):
    """the list (kind 7, id 0x7b79e3 mostly) and its children, as dialogbuild.py's child() writes it into w"""
    n = 1 if r.random() < 0.95 else r.choice([0, 2])   # (rows of no children: a second column's rectangle set on none, a fault in both)
    inner = [0xb, n]
    x = r.randrange(0, 10)
    for _ in range(n):
        width = r.randrange(5, 40)
        inner += [x, x + width]
        x += width
    left, top = r.randrange(0, 20), r.randrange(0, 30)
    inner += [0xa, left, top, left + r.randrange(20, 200), top + r.randrange(0, 170)]
    w = []
    dialogbuild.child(r, w, LIST if r.random() < 0.97 else r.choice([0, LIST + 1]), inner + [5], 120, 7)
    return w


def case(r):
    op = r.deal(OPS)
    dialogbuild.DIALOGS[op] = (HOST + 0x3f0, 0x74f9e0, (LIST,), ())
    regions = dialogbuild.scripted(r, op)
    log = regions[LOG]
    regions[HEAP] = heap(0x80000, r.choice([0] * 40 + [r.randrange(1, 16)]))   # (an allocation failing: a fault in both, so rarely)
    kids = [list_script(r)] + [[] for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(kids)
    inner = []
    for k in kids:
        if k:
            inner += k
        else:
            dialogbuild.child(r, inner, r.choice([0, 1, r.getrandbits(32) | 0x100]))
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    # the window (kind 1): its rectangle written here (no flag 0x400: +8 is these words), its edges about the limits now and then
    way = r.choice(['below'] * 3 + ['above'] * 2 + ['beside'])
    width, height = r.randrange(0, 160), r.randrange(1 if way == 'beside' else 0, 160)   # (beside: its top above its height)
    if way == 'below':
        top = r.choice([r.randrange(0, 100), 0x5f8 - height - r.randrange(0, 3)])
    else:
        top = r.randrange(0, 100) if way == 'above' else r.randrange(0, height)
    left = r.choice([r.randrange(0, 100), 0x7f8 - width - r.randrange(0, 3)])
    w += [0, 1]
    windowscript.dword(r, w, windowfactory.no_handlers(1, r.choice(windowscript.FLAGS)))
    windowscript.dword(r, w, r.choice([0, 1, r.getrandbits(32)]))
    w += [left, top, left + width, top + height] + inner + [5]
    w += [5] * 4
    regions[0x74f9e0] = struct.pack('<%dH' % len(w), *w)
    for vt in dialogbuild.KIND_VTABLES:
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
        regions[vt + 0x1c] = struct.pack('<I', recorder(29, 1))
    for vt in dialogbuild.PART_KINDS:
        regions[vt + 0xc] = struct.pack('<I', recorder(3, 2))
        regions[vt + 0x14] = struct.pack('<I', recorder(5, 1))
        regions[vt + 0x24] = struct.pack('<I', recorder(9, 1))
    # the rows' height: font 7's (its slot 1) over the divisor [0xfaa5c0]
    regions[0xfaa5c0] = struct.pack('<h', r.choice([1536, 768, 0x600 + r.randrange(-3, 4)]))
    lang = regions[0x78b530] = bytearray(regions[0x78b530])
    group = struct.unpack_from('<I', lang, 4)[0]
    put(lang, 0x1c + (7 + 13 * group) * 0x18, 'I', FONTVT)
    regions[FONTVT] = vtable({1: 0})
    set_return(log, 1, r.randrange(20, 80))
    regions.update(global_heap())
    regions[panelparts.MESH_VTABLE + 0xc] = struct.pack('<I', recorder(3, 2))
    regions[panelparts.MESH_VTABLE + 0x14] = struct.pack('<I', recorder(5, 1))
    dialog7ca150.registry(r, regions, ['hilight'])
    root = bytearray(regions[0xfaa5d8])   # (the GUI root, whose top depth over the window is asked: WINDOW2, the parent, mostly)
    put(root, 0, 'I', window.WINDOW2 if r.random() < 0.95 else 0)
    regions[0xfaa5d8] = root
    # the host: its window +4 the anchor, its IME +0x1c without a current entry or with one cached and no strings
    # (the anchor's edges such that the window stays where it is: below the anchor (its bottom the window's top, its left the window's),
    # above it (its bottom low enough, its top the window's bottom) or beside it (its top the window's, below the window's height; its
    # right the window's left); the others random)
    anchor = bytearray(r.bytes(0x120))
    a = [r.randrange(-0x100, 0x800) for _ in range(4)]   # (left, top, right, bottom)
    if way == 'below':
        a[0], a[3] = left, top
    elif way == 'above':
        a[0], a[1], a[3] = left, top + height, r.choice([0x5f9 - height + r.randrange(0, 3), r.randrange(0x5f9 - height, 0x800)])
    else:
        a[1], a[2], a[3] = top, left, r.choice([0x5f9 - height + r.randrange(0, 3), r.randrange(0x5f9 - height, 0x800)])
    put(anchor, 8, 'I4h', 0x705920, *a)
    put(anchor, 0xf8, '4h', 0, 0, 0, 0)
    regions[ANCHOR] = anchor
    host = bytearray(r.bytes(0x1c + 0x190))
    put(host, 4, 'I', ANCHOR)
    ime = 0x1c
    if r.random() < 0.6:
        put(host, ime + 0x54, 'i', -1)
        put(host, ime + 0x184, 'B', r.choice([0, 1, r.getrandbits(8)]))
    else:   # (the current entry, the first of the list +0x3c, cached already: the same first dword, the strings' vector empty)
        put(host, ime + 0x3c, 'II', ENTRIES, ENTRIES + 0x24)
        put(host, ime + 0x54, 'i', 0)
        key = r.getrandbits(32)
        regions[ENTRIES] = put(bytearray(r.bytes(0x24)), 0, 'I', key)
        put(host, ime + 0x160, 'I', key)
        put(host, ime + 0x174, 'II', 0, 0)
        put(host, ime + 0x184, 'B', 1)
    regions[HOST] = host
    return Call(CALLS[op], [HOST, window.WINDOW2, r.getrandbits(32)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

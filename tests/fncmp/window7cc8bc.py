# The window [0x7cc8bc]'s name list and the handler of [0x7cc8c4] / [0x7cc8c8] (src/game/ui/window7cc8bc: 0x4beb60-0x4bedd1 and the
# stdcall wrappers 0x481820-0x4818b2) against the originals. The window [0x7cc8bc] (now and then none) has the child 0xc3123 whose child
# 0xc3124 is window7059f0.py's list_env() list (each among 0-2 decoys, now and then missing: a fault in both); the mode [0x7cc8d0] the
# op's mostly, else the other or 2. Add / Add1 (and the wrappers): window7059f0.py's add_case() with at most one column (the callers'
# texts are one stack slot: a second column would read the return address), its texts' first the text, its data the data. Remove /
# SetData (and 1, and the wrappers): list_env()'s cells strings of window7059f0.py's ALPHA, 0-3 long (the buffers' counts kept), the
# first column's bottom 0 mostly; the text one of them or another; for SetData the list's rows mostly 1-3 row windows (window7059f0.py's
# row_windows(), the array +0x148 set, the columns' slot 9 a recorder), so the refresh is seen. Handler (0x4bed60): window.py's shown_env tree with its previous
# siblings +0x58 set as windowlinks.py's (Window_Unlink), the windows [0x7cc8cc] and [0x7cc8bc] of it (now and then none: a fault in
# both), [0x7cc8c4] the window or another, the window's default handler +0x118 fncmp's logger; the message 0x14, 0x10006 or another.
# Create (0x4bf460), create_case(): three scripted builds on the GUI root [0xfaa5d8] (WINDOW2 mostly; none now and then: a fault in
# both) with dialogbuild.py's scripted() surroundings, then a new window. The scripts generated: 0x758ae8 0-2 plain children, then the
# page (kind 1) whose child 0xc3123 (kind 1) has the lists 0xc3124 and 0xc312b (kind 7: one column, the inner rectangle under a row
# high mostly, else rows kept, moved and sent 0x12, which their handlers take (stored raw: the driver's kRawCalled)); the first's
# children the radio group 0xc3125 (scriptchain.py's radio_group(): the buttons 0xc3126-0xc3128; another id now and then: a fault in
# both), the second's the edit box 0xc312c (kind 5); 0x758cd8 / 0x758e18 0-2 plain children, then a kind 1 window whose child
# 0x1789 / 0x3003 is among kind 1 decoys (hidden at once, so the children's handlers are told: kind 1's default one). Each wanted child
# is among 0-2 decoys in any order and missing now and then (a fault in both). The kinds' slots 5 and 7 recorders, the edit box's slot 9
# too; the parts' slots 3, 4, 5 and 9 recorders; dialog7ca150.py's registry with "hilight"; the rows' font [0x758f04] 7 (its slot 1 the
# height, the divisor [0xfaa5c0] about 0x600), the edit box's [0x758f08] 0-12; the GUI manager [0x77c48c] a recorder (slot 13: told of
# the hidden windows); the globals written random; HelpBg_Place's object 0x7ca0c0 with a lone window (moved) or none; a heap of 0x80000 (failing rarely).
# usage: window7cc8bc.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Add,Remove,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7ca150  # noqa: E402
import dialog7cc5f4  # noqa: E402
import dialogbuild  # noqa: E402
import scriptchain  # noqa: E402
import window  # noqa: E402
import window7059f0 as w7  # noqa: E402
from fncmp import CDECL_LOG4, HEAP, LOG, RAM, Bench, Call, heap, log_region, put, recorder, set_return, vtable  # noqa: E402
from window import notify_blobs, shown_env  # noqa: E402

CALLS = {'Add': 0x4beb60, 'Remove': 0x4bebb0, 'SetData': 0x4bec00, 'Add1': 0x4bec60, 'Remove1': 0x4becb0, 'SetData1': 0x4bed00,
         'AddStd': 0x481820, 'RemoveStd': 0x481840, 'SetDataStd': 0x481850, 'Add1Std': 0x481870, 'Remove1Std': 0x481890,
         'SetData1Std': 0x4818a0, 'Handler': 0x4bed60, 'Create': 0x4bf460}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
CREATE_RANGES = dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + dialog7cc5f4.RANGES + [
    (0x4bf460, 0x4bf73a), (0x4bf740, 0x4bf8de), (0x4be730, 0x4be830), (0x492330, 0x4923c5),
    (0x4923d0, 0x49243e), (0x492530, 0x49255b), (0x65d4cb, 0x65d4f6), (0x6662b7, 0x666308), (0x66656c, 0x66671d)]
# (the create's: dialog7ca150.py's (the scripted builds, lists, rows, radio group), dialog7cc5f4.py's (the edit box), its own, the rows'
# makers and handlers, HelpBg_Place's)
B = Bench(dialog7cc5f4.merged(w7.LIST_RANGES + [(0x4beb60, 0x4bedd1), (0x481820, 0x4818b3), (0x65f538, 0x65f5b7), (0x65e59b, 0x65e60a),
                                                (0x65f66e, 0x65f6e1)] + CREATE_RANGES))
TOP, MIDDLE, DECOYS, TEXT = RAM + 0x30000, RAM + 0x30200, RAM + 0x30400, RAM + 0x30800
HELPWIN = RAM + 0x3fa00   # (Create's: HelpBg_Place's window)


def among(r, regions, want, base):
    """want (0x100 bytes from it on, its +0x54 set here) among 0-2 decoy windows at base (0x100 bytes each), linked by +0x54, now and then
    missing; returns the first or 0"""
    kids = [base + 0x100 * k for k in range(r.choice([0, 0, 1, 2]))]
    for k in kids:
        regions[k] = put(bytearray(r.bytes(0x100)), 0x8c, 'I', r.choice([1, 0xc3125, r.getrandbits(32)]))
    if r.random() < 0.98:
        kids.insert(r.randrange(len(kids) + 1), want)
    for i, k in enumerate(kids):
        put(regions[k], 0x54, 'I', kids[i + 1] if i + 1 < len(kids) else 0)
    return kids[0] if kids else 0


def top(r, regions, mode):
    """[0x7cc8bc] (mostly TOP) and the mode [0x7cc8d0] (mostly mode); TOP's child 0xc3123 MIDDLE, whose child 0xc3124 the list"""
    regions[0x7cc8bc] = struct.pack('<I', TOP if r.random() < 0.92 else 0)
    regions[0x7cc8d0] = struct.pack('<I', mode if r.random() < 0.85 else r.choice([1 - mode, 2]))
    put(regions[w7.LIST3], 0x8c, 'I', 0xc3124)
    regions[MIDDLE] = put(bytearray(r.bytes(0x100)), 0x8c, 'I', 0xc3123)
    put(regions[MIDDLE], 0x5c, 'I', among(r, regions, w7.LIST3, DECOYS))
    regions[TOP] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', among(r, regions, MIDDLE, DECOYS + 0x200))


def named(r, rows=False):
    """list_env() with its cells strings of ALPHA (the counted buffers' chars at +3 rewritten, their lengths +8 0-3) and the first
    column's bottom mostly 0, at least one column and each item's first cell an object (taken from the other columns' cells when none is
    unused: FindText reads it unchecked); returns (regions, the text: one of the strings or another)"""
    while True:
        window, regions, idx = w7.list_env(r)
        ncols = struct.unpack_from('<h', window, 0x164)[0]
        if ncols > 0:
            break
    links = list(struct.unpack('<%dI' % (len(regions[w7.LINKS3 - 0x20]) // 4), regions[w7.LINKS3 - 0x20]))
    used = {links[8 + i + 4 + c] for i in idx for c in range(ncols)}
    spare = [w7.OBJS3 + 0x10 * j for j in range(16) if w7.OBJS3 + 0x10 * j not in used]
    for i in idx:   # (an object from another column's cell when none is spare)
        for c in range(1, ncols):
            if spare:
                break
            for k in idx:
                if links[8 + k + 4 + c]:
                    spare.append(links[8 + k + 4 + c])
                    links[8 + k + 4 + c] = 0
                    break
        if links[8 + i + 4] == 0 and spare:
            links[8 + i + 4] = spare.pop()
    regions[w7.LINKS3 - 0x20] = struct.pack('<%dI' % len(links), *links)
    if r.random() < 0.9:   # (the count at most the items the walk from the first shown one reaches: past them it reads links as cells)
        put(window, 0x150, 'i', min(struct.unpack_from('<i', window, 0x150)[0], reach(links[8:], struct.unpack_from('<i', window, 0x160)[0])))
    texts = [tuple(r.choice(w7.ALPHA) for _ in range(r.choice([0, 1, 2, 3]))) for _ in range(8)]
    bufs, objs = bytearray(regions[w7.BUFS3]), bytearray(regions[w7.OBJS3])
    for k in range(8):
        t = texts[k]
        struct.pack_into('<%dH' % (len(t) + 1), bufs, 0x20 * k + 3, *t, 0)
    for j in range(16):
        p = struct.unpack_from('<I', objs, 0x10 * j + 4)[0]
        put(objs, 0x10 * j + 8, 'I', len(texts[(p - w7.BUFS3) // 0x20]) if p else 0)
    regions[w7.BUFS3], regions[w7.OBJS3] = bufs, objs
    cols = bytearray(regions[w7.COLS3])
    if r.random() < 0.8:
        put(cols, 0xa, 'h', 0)
    regions[w7.COLS3] = cols
    text = r.choice(texts + [(0x42, 0x41, 0x41, 0x41)])
    regions[TEXT] = struct.pack('<%dH' % (len(text) + 1), *text, 0)
    if rows and r.random() < 0.85:   # (SetData's refresh: 1-3 row windows as window7059f0.py's refresh_case's, their columns' slot 9 a recorder)
        nrows = r.choice([1, 2, 3])
        put(window, 0x172, 'h', nrows)
        wins = w7.row_windows(r, regions, [], nrows, ncols, w7.LIST3)
        regions[w7.ROWS2] = struct.pack('<%dI' % (nrows + 1), *wins, r.getrandbits(32))
        put(window, 0x148, 'I', w7.ROWS2)
        regions[w7.VTABLE] = vtable({5: 1, 3: 2, 0: 1, 7: 1, 9: 1})
        notify_blobs(regions[LOG], 0, 0x3c00)
    return regions, TEXT


def reach(links, item):
    """the items NextItem (0x663a93) steps through from item (as window7059f0.py's list_case for Clear)"""
    links = [v - (1 << 32) if v & 0x80000000 else v for v in links]
    n = 0
    while item >= 0 and n < 64:
        n += 1
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
    return n


def handler_case(r):
    log = log_region()
    regions = {LOG: log}
    wins = shown_env(r, regions, log)
    for w in wins:
        put(regions[w], 0x58, 'I', 0)
    for w in wins:   # (each child list's previous links, as windowlinks.py's)
        prev, c = 0, struct.unpack_from('<I', regions[w], 0x5c)[0]
        while c:
            put(regions[c], 0x58, 'I', prev if r.random() < 0.95 else 0)
            prev, c = c, struct.unpack_from('<I', regions[c], 0x54)[0]
    window = r.choice(wins)
    put(regions[window], 0x118, 'I', CDECL_LOG4)
    regions[0xfaa5ec] = struct.pack('<I', r.choice(wins + [0]))
    regions[0x7cc8cc] = struct.pack('<I', r.choice(wins) if r.random() < 0.97 else 0)
    regions[0x7cc8bc] = struct.pack('<I', r.choice(wins) if r.random() < 0.97 else 0)
    regions[0x7cc8c4] = struct.pack('<II', r.choice([window, window, r.choice(wins), 0]), r.getrandbits(32))
    message = r.choice([0x14] * 4 + [0x10006] * 4 + [0x10005, 0x15, 4, r.getrandbits(32)])
    return Call(CALLS['Handler'], [window, message, r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


dialogbuild.DIALOGS['Window7cc8bc_Create'] = (0x7cc8bc, 0x758ae8, (0xc3123,), ())   # (scripted()'s script, replaced below)
MANAGER = RAM + 0x3f800


def kids(r, w, wanted, decoys=(0, 2), kind=None):
    """wanted: (id, kind, script) children written into w in any order among 0-2 decoys (of `kind`, or kinds 1-13), one of them missing
    now and then (a fault in both: so rarely)"""
    items = list(wanted)
    if items and r.random() < 0.01:
        items.remove(r.choice(items))
    items += [None] * r.choice([0, 0, 1, 2])
    r.shuffle(items)
    for it in items:
        if it is None:
            dialogbuild.child(r, w, r.choice(list(decoys) + [r.getrandbits(32) | 0x100]), kind=kind)
        else:
            dialogbuild.child(r, w, it[0], it[2] + [5], 120, it[1])


def list_body(r, wanted):
    """a list's (kind 7) script: one column (0xb: the rows are single windows), its inner rectangle (0xa: under a row high mostly, so
    no row is kept; else the kept rows are moved and sent 0x12, which their handlers 0x4be730 / 0x4be7c0 (stored raw: the driver's
    kRawCalled) give the default handler), then the children `wanted`"""
    x = r.randrange(0, 10)
    inner = [0xb, 1, x, x + r.randrange(5, 60)]
    left, top = r.randrange(0, 20), r.randrange(0, 30)
    inner += [0xa, left, top, left + r.randrange(20, 200), top + r.choice([r.randrange(0, 20)] * 6 + [0, 1, r.randrange(20, 120)])]
    kids(r, inner, wanted)
    return inner


def top_script(r, regions, at, wanted, kind=None):
    """0-2 plain children, then a kind 1 window whose children are `wanted` among decoys of `kind` (above); packed at `at`"""
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    inner = []
    kids(r, inner, wanted, kind=kind)
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), inner + [5], 160, 1)
    w += [5] * 4
    regions[at] = struct.pack('<%dH' % len(w), *w)


def create_case(r):
    """Window7cc8bc_Create (0x4bf460): three scripted builds on the GUI root and a new window (see the module comment)"""
    regions = dialogbuild.scripted(r, 'Window7cc8bc_Create')
    regions.pop(0x7cc8bc)
    log = regions[LOG]
    if r.random() < 0.95:
        regions[0xfaa5d8] = put(bytearray(regions[0xfaa5d8]), 0, 'I', window.WINDOW2)
    regions[HEAP] = heap(0x80000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    scriptchain.kind_recorders(regions)
    regions[0x705a20 + 0x24] = struct.pack('<I', recorder(30, 1))   # (the edit box's slot 9, 0x6664fa: unported)
    for vt in dialogbuild.PART_KINDS:   # (the parts' unported slots 3, 4, 5 and 9)
        regions[vt + 0xc] = struct.pack('<I', recorder(3, 2))
        regions[vt + 0x10] = struct.pack('<I', recorder(4, 2))
        regions[vt + 0x14] = struct.pack('<I', recorder(5, 1))
        regions[vt + 0x24] = struct.pack('<I', recorder(9, 1))
    dialog7ca150.registry(r, regions, ['hilight'])
    # the rows' font [0x758f04] (7 mostly: its slot 1 the height) and the edit box's [0x758f08]
    regions[0x758f04] = struct.pack('<II', 7, r.choice([7] * 6 + list(range(13))))   # (another rows' font's slot 1 random: a fault)
    regions[0xfaa5c0] = struct.pack('<h', r.choice([1536, 768, 0x600 + r.randrange(-3, 4)]))
    lang = regions[0x78b530] = bytearray(regions[0x78b530])
    group = struct.unpack_from('<I', lang, 4)[0]
    put(lang, 0x1c + (7 + 13 * group) * 0x18, 'I', dialog7ca150.FONTVT)
    regions[dialog7ca150.FONTVT] = vtable({1: 0})
    set_return(log, 1, r.randrange(10, 60))
    # the page 0x758ae8: 0xc3123 whose children are the list 0xc3124 (the radio group 0xc3125 among its children) and the list 0xc312b
    # (the edit box 0xc312c among its children)
    group = []
    scriptchain.radio_group(r, group, 0xc3125 if r.random() < 0.97 else 0xc3129, (0xc3126, 0xc3127, 0xc3128))
    first = list_body(r, []) + group
    second = list_body(r, [(0xc312c, 5, [])])
    names = []
    kids(r, names, [(0xc3124, 7, first), (0xc312b, 7, second)], (0, 1, 0xc3122))
    top_script(r, regions, 0x758ae8, [(0xc3123, 1, names)])
    top_script(r, regions, 0x758cd8, [(0x1789, 1, [])], 1)   # (hidden at once: its children told, kind 1's default handler)
    top_script(r, regions, 0x758e18, [(0x3003, 1, [])], 1)
    regions[0x7cc8b4] = r.bytes(0x34)   # (0x7cc8b4-0x7cc8e7: the module's globals, written)
    for k in (0x7a192c, 0x7a194c, 0x7a162c, 0x7a0c9c, 0x7a1c54, 0x74d104):
        regions[k] = r.bytes(8 if k in (0x7a1c54, 0x74d104) else 4)
    # (HelpBg_Place's object: its window a lone one (moved: its rectangles, position and handler, fncmp's logger, seen) or none)
    helpwin = bytearray(r.bytes(0x120))
    helpwin[4] = 0
    put(helpwin, 8, 'I', 0x705920)
    put(helpwin, 0x2c, 'I', 0x705920)
    for off, v in ((0x44, HELPWIN + 8), (0x5c, 0), (0x98, 0), (0x9c, 0), (0x114, CDECL_LOG4)):
        put(helpwin, off, 'I', v)
    regions[HELPWIN] = helpwin
    regions[0x7ca0c0] = put(bytearray(r.bytes(0x20)), 0x18, 'I', r.choice([0, HELPWIN, HELPWIN]))
    regions[0x77c48c] = struct.pack('<I', MANAGER)   # (the manager told of the hidden windows: its slot 13 a recorder)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40)
    regions[MANAGER + 0x40] = vtable({13: 1})
    return Call(CALLS['Create'], [], regions, ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'Create':
        return create_case(r)
    if op == 'Handler':
        return handler_case(r)
    mode = 1 if '1' in op else 0
    conv = 's' if op.endswith('Std') else 'c'
    if op.startswith('Add'):
        while True:
            c = w7.add_case(r, columns=(0, 1, 1, 1))
            if c.args[1]:
                break
        regions = c.regions
        top(r, regions, mode)
        text = struct.unpack_from('<I', regions[w7.ATEXTS])[0]
        return Call(CALLS[op], [text, c.args[2]], regions, conv=conv, ret='v', masks=c.masks)
    regions, text = named(r, rows='SetData' in op)
    top(r, regions, mode)
    args = [text] if 'Remove' in op else [text, r.getrandbits(32)]
    return Call(CALLS[op], args, regions, conv=conv, ret='v')


if __name__ == '__main__':
    B.main(case)

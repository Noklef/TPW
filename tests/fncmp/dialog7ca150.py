# The objects' lists dialog's tabs (src/game/ui/dialog7ca150: 0x494ec0, 0x494c70, 0x494250, 0x495110) against the originals.
# list_dialog(): dialogbuild.py's scripted() surroundings (listrows.py's text parts, windowscript.py's build on WINDOW2, the heap, the
# timers, the queues), WINDOW2 the child 0x12c4b8 of the page [0x7ca150] (another id now and then: no page, a fault in both), its child
# 400 before the build a window whose handler is fncmp's logger (another id, or none, now and then); the tab's script at its address
# generated: 0-2 plain children, then the list (kind 7, up to 120 high) whose children are the header buttons 0x10 on (kind 2, one of them
# missing now and then: a fault in both) among 0-1 others, in any order; the kinds' slots 5 and 7 recorders (dialogbuild.py's
# windowat10()); the heap failing rarely (a fault in both); the rows' height [0x7ca158] 20-80 mostly (0 now and then: the font
# [0x750d54]'s slot 1 asked, font 7's vtable a recorder, the divisor [0xfaa5c0] about 1536), the list's inner rectangle (0xa) under 20
# high mostly, so no row is kept (the first one made by List7ca158_NewRow only), 20-170 high now and then: the rows kept are moved
# (Window7059f0_SetRow) and sent 0x12, which their handler 0x4959c0 (stored raw: the driver's kRawCalled jumps to its port) gives the
# default handler; the part kinds' unported slots 3, 5 and 9 recorders (listrows.py's child_case()); the GUI's registered windows: "hilight" and "!cbut"
# each among 0-2 others or not (their window a recorder whose slot 8 gives listrows.py's CUT, an object, or 0); the texts' record table
# about the tab's ids (panelparts.py's texts()); the buttons' font [0x750d58] 0-12, 7 mostly; the sort dword +-1 to +-the columns, 0
# or any; an empty park (the things' callbacks are unported: none called); the selected data 0 or any (the list is new and empty, so it
# is never found: the scroll is not reached).
# The open (0x495aa0), open_case(): two builds in order. focused()'s surroundings (the page's parent [0x7cb2ac] WINDOW2) with the page's
# script at 0x7508e0 generated: 0-2 plain children, then the page (kind 1) whose children are 0x12c4b8 (kind 1; another id now and then:
# a fault in both) and the hover label's 0x12c4be (kind 1: its new rectangle sends it 0x12, which other kinds' unported handlers take;
# missing now and then: no label) among 0-2 decoys in any order; 0x12c4b8's children the radio group 0x12c4b9 (scriptchain.py's
# radio_group(): the buttons 0x12c4ba-0x12c4bd; another id now and then: a fault in both) among 0-1 decoys (no child 400: the tab's 4
# would delete it through the window manager; the tab's cases above cover it); the tab then builds its list into 0x12c4b8 with the tab's
# surroundings above (the sort, registry, texts (the labels' 0x4e-0x51 too), rows), a heap of 0x80000 (failing rarely); the tab 0-3, -1
# ([0x7ca154] 0-3 mostly) or another (no tab, no label); the hover label's shared part (scriptchain.py's hover_part()); advisor.py's
# PostId surroundings for 0xbf (its record its own mostly; the stack message's unwritten bytes masked).
# usage: dialog7ca150.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Dialog7ca150_ShowTab0,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialogbuild  # noqa: E402
import listrows  # noqa: E402
import panelparts  # noqa: E402
import window  # noqa: E402
import window7059f0  # noqa: E402
import windowscript  # noqa: E402
import windowfactory  # noqa: E402
import radiogroup  # noqa: E402
import scriptchain  # noqa: E402
from fncmp import CDECL_LOG4, HEAP, LOG, Bench, Call, _cdecl_log, global_heap, heap, put, recorder, set_return, vtable  # noqa: E402
from msgtags import ADVISOR_RANGES  # noqa: E402

CALLS = {'Dialog7ca150_ShowTab0': 0x494ec0, 'Dialog7ca150_ShowTab1': 0x494c70, 'Dialog7ca150_ShowTab2': 0x494250,
         'Dialog7ca150_ShowTab3': 0x495110, 'Dialog7ca150_Open': 0x495aa0}
# each tab: (its script, the columns, the sort dword, the first header text)
TABS = {'Dialog7ca150_ShowTab0': (0x750ab8, 5, 0x750d5c, 0x52), 'Dialog7ca150_ShowTab1': (0x750ab8, 5, 0x750d60, 0x57),
        'Dialog7ca150_ShowTab2': (0x750ba0, 6, 0x750d64, 0x5c), 'Dialog7ca150_ShowTab3': (0x750ca0, 2, 0x750d68, 0x62)}
ALL_RANGES = sorted(set(dialogbuild.ALL_RANGES + panelparts.ALL_RANGES + window7059f0.LIST_RANGES + [
    (0x481bc0, 0x481f90), (0x481f80, 0x482040), (0x482040, 0x482110), (0x482100, 0x4821c0), (0x4821b0, 0x482280), (0x482270, 0x482350),
    (0x482340, 0x482410), (0x482400, 0x4824d0), (0x516130, 0x5161b0), (0x50b350, 0x50b360), (0x412e90, 0x412eea), (0x4dd4e0, 0x4dd500),
    (0x4e2710, 0x4e273a), (0x4cd300, 0x4cd331), (0x5da3c0, 0x5da3c1),
    (0x485a70, 0x485ab0), (0x485ac0, 0x485b50), (0x486660, 0x486790), (0x494250, 0x4952a0), (0x4944d0, 0x494690),
    (0x659bc9, 0x659c52), (0x6588b6, 0x6588ef), (0x65d070, 0x65d1c8), (0x668f5a, 0x668f9c), (0x668820, 0x6688b3), (0x668ad3, 0x668bd4),
    (0x6659af, 0x665a54), (0x65dd7b, 0x65e559), (0x65bcad, 0x65c035),
    (0x4959c0, 0x495a94), (0x65f66e, 0x65f6a7)]))   # (the kept rows' handler List7ca158_RowProc: sent 0x12, its default handler's answer)
OPEN_RANGES = radiogroup.RANGES + ADVISOR_RANGES + [
    (0x495aa0, 0x495bf6), (0x4a0810, 0x4a083d), (0x485d20, 0x485e30), (0x48d290, 0x48d36d), (0x48f690, 0x48f6b0), (0x492d20, 0x492d3b),
    (0x6589f9, 0x658abb), (0x658ccc, 0x658e04), (0x659710, 0x659740), (0x659d00, 0x659d53), (0x659d90, 0x659df5), (0x659e3e, 0x659e93),
    (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x65c388, 0x65c4c9), (0x65c5d5, 0x65c682), (0x65c7fe, 0x65c989), (0x65c9bd, 0x65cda5),
    (0x65d070, 0x65d1d8), (0x65d42e, 0x65d4b0), (0x65d773, 0x65d832), (0x65db25, 0x65dc00), (0x65e526, 0x65e559), (0x65e6d5, 0x65e73d),
    (0x65f5fe, 0x65f655), (0x65f66e, 0x65f6e1), (0x6616b0, 0x661780), (0x6617f0, 0x661820), (0x6697e3, 0x6698b4), (0x66bff7, 0x66c01b)]
# (the open's: radiogroup.py's, the advisor's (msgtags.py's), its own, Global7cb28c_Select's and hoverlabel.py's from 0x485d20 on)


def merged(ranges):
    out = []
    for lo, hi in sorted(set(ranges)):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged(ALL_RANGES + OPEN_RANGES), hooks={**panelparts.animatormesh.HOOKS, 0x67a7d0: _cdecl_log})
BASE = 0x32000000
PAGE, C400, NODES, FOUND, FVT, FONTVT = BASE, BASE + 0x100, BASE + 0x300, BASE + 0x400, BASE + 0x480, BASE + 0x500


def registry(r, regions, names):
    """the GUI's registered windows [0xfaa610]: each name a node (or not), among 0-2 others, in any order; each node's window FOUND, a
    recorder whose slot 8 gives listrows.py's CUT (an object of the +0x98 kind), OBJ98 or 0"""
    hashes = [listrows.hash_name(n) for n in names if r.random() < 0.85]
    hashes += [r.choice([listrows.hash_name('!frame'), r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(hashes)
    nodes = bytearray(r.bytes(0x18 * 5))
    for i, h in enumerate(hashes):
        put(nodes, 0x18 * i, 'I', FOUND)
        put(nodes, 0x18 * i + 0x10, 'I', h)
        put(nodes, 0x18 * i + 0x14, 'I', NODES + 0x18 * (i + 1) if i + 1 < len(hashes) else 0)
    regions[NODES] = nodes
    regions[0xfaa610] = struct.pack('<I', NODES if hashes else 0)
    regions[listrows.VT98] = vtable({0: 1, 3: 2, 5: 1})
    regions[FOUND] = struct.pack('<I', FVT) + r.bytes(0x1c)
    regions[FVT] = vtable({})
    set_return(regions[LOG], 8, r.choice([listrows.CUT] * 4 + [0, listrows.OBJ98]))
    regions[listrows.CUT] = put(bytearray(r.bytes(0x20)), 0, 'I', listrows.VT98)
    put(regions[listrows.CUT], 0x10, 'HH', r.choice([0, 1, 2]), r.choice([0, 1, 2]))
    regions[listrows.OBJ98] = put(bytearray(r.bytes(0x20)), 0, 'I', listrows.VT98)


def list_script(r, columns):
    """0-2 plain children, then the list (kind 7): its columns (opcode 0xb: the tab's count, 0 now and then: the rows unset, a fault in
    both), its inner rectangle (0xa: up to 120 high) or not, its header buttons (0xc: kind 2, ids 0x10 on) in any order, one missing now
    and then, among 0-1 other children"""
    n = columns if r.random() < 0.97 else 0
    inner = [0xb, n]
    x = r.randrange(0, 10)
    for _ in range(n):
        width = r.randrange(5, 40)
        inner += [x, x + width]
        x += width
    left, top = r.randrange(0, 20), r.randrange(0, 30)   # (under a row high mostly: no rows kept, so none moved, see list_dialog())
    inner += [0xa, left, top, left + r.randrange(20, 200), top + r.choice([r.randrange(0, 20)] * 6 + [0, 1, r.randrange(20, 170)])]
    heads = list(range(columns))
    if r.random() < 0.03:
        heads.remove(r.choice(heads))
    heads += [None] * r.choice([0, 0, 1])
    r.shuffle(heads)
    for i in heads:
        if i is None:
            dialogbuild.child(r, inner, r.choice([0, 0x400, r.getrandbits(32) | 0x100]))
        else:
            inner += [0xc, i]
            dialogbuild.small_rect(r, inner)
            inner.append(5)
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 2, r.getrandbits(32) | 0x100]))
    dialogbuild.child(r, w, r.choice([0, 0x1e497, r.getrandbits(32) | 0x100]), inner + [5], 120, 7)
    return w + [5] * 4


def list_dialog(r, op, page=None):
    """the tab's surroundings (above); returns the regions. page: for the open (open() below), a function of (r, regions) that lays out
    the page's script: focused()'s surroundings then, the tab building into the page's built child 0x12c4b8, and none of the page made
    here"""
    script, columns, sort, first = TABS[op]
    dialogbuild.DIALOGS[op] = (0x7ca14c, script, (0x10,), (0x7ca154,))
    regions = (dialogbuild.focused if page else dialogbuild.scripted)(r, op)
    log = regions[LOG]
    regions[HEAP] = heap(0x14000, r.choice([0] * 40 + [r.randrange(1, 8)]))   # (an allocation failing: a fault in both, so rarely)
    w = list_script(r, columns)
    regions[script] = struct.pack('<%dH' % len(w), *w)
    for vt in dialogbuild.KIND_VTABLES:   # (the built windows' slots 5 and 7: game code the driver cannot run natively)
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
        regions[vt + 0x1c] = struct.pack('<I', recorder(29, 1))
    # the page [0x7ca150]: WINDOW2 its child 0x12c4b8, WINDOW2's child 400 before the build
    if page:
        extra = page(r, regions)
    else:
        regions[PAGE] = put(bytearray(r.bytes(0x80)), 0x5c, 'I', window.WINDOW2)
        regions[0x7ca150] = struct.pack('<I', PAGE)
    win = regions[window.WINDOW2]
    if not page:
        put(win, 0x8c, 'I', 0x12c4b8 if r.random() < 0.97 else 0x12c4b9)
        put(win, 0x54, 'I', 0)
    if not page and r.random() < 0.8:
        c = bytearray(r.bytes(0x120))
        put(c, 0, 'I', window.VTABLE)
        c[4] = r.choice([0] * 8 + [1])
        put(c, 0x48, 'I', r.choice([1, 0, 0x401]))
        put(c, 0x54, 'IIII', 0, 0, 0, window.WINDOW2)
        put(c, 0x8c, 'I', 400 if r.random() < 0.85 else 401)
        put(c, 0x98, 'II', 0, 0)
        put(c, 0x114, 'II', CDECL_LOG4, CDECL_LOG4)
        regions[C400] = c
        put(win, 0x5c, 'I', C400)
    # the rows: their height cached (or font [0x750d54]'s), the part kinds' unported slots
    regions[0x7ca158] = struct.pack('<I', r.choice([r.randrange(20, 80)] * 12 + [0]))
    regions[0xfaa5c0] = struct.pack('<h', r.choice([1536, 768, 0x600 + r.randrange(-3, 4)]))
    regions[0x750d54] = struct.pack('<I', 7)
    lang = regions[0x78b530] = bytearray(regions[0x78b530])
    group = struct.unpack_from('<I', lang, 4)[0]
    put(lang, 0x1c + (7 + 13 * group) * 0x18, 'I', FONTVT)
    regions[FONTVT] = vtable({1: 0})
    set_return(log, 1, r.randrange(20, 80))
    for vt in dialogbuild.PART_KINDS:   # (the kept rows' parts too: their slots 3, 5 and 9 unported, as imecandidates.py's)
        regions[vt + 0xc] = struct.pack('<I', recorder(3, 2))
        regions[vt + 0x14] = struct.pack('<I', recorder(5, 1))
        regions[vt + 0x24] = struct.pack('<I', recorder(9, 1))
    regions.update(global_heap())   # (the bars' meshes: panelparts.py's surroundings())
    regions[panelparts.MESH_VTABLE + 0xc] = struct.pack('<I', recorder(3, 2))
    regions[panelparts.MESH_VTABLE + 0x14] = struct.pack('<I', recorder(5, 1))
    registry(r, regions, ['hilight', '!cbut'])
    panelparts.texts(r, regions, list(range(first, first + columns)) + (extra if page else []))
    regions[0x750d58] = struct.pack('<I', r.choice([7] * 6 + list(range(13))))
    by = r.choice(list(range(1, columns + 1)) * 2 + [0, r.getrandbits(31)]) * r.choice([1, -1])
    regions[sort] = struct.pack('<I', by & 0xffffffff)
    regions[0x7cf56c] = struct.pack('<I', 0)   # (an empty park)
    return regions


def open_page(r, regions):
    """the open's page (above): its script at 0x7508e0, [0x7ca150] random, the hover label's part, [0x7cb2a0] random"""
    regions[0x7ca150] = r.bytes(4)
    sub = []   # (0x12c4b8's children: the radio group, 0-1 decoys)
    kids = ['group'] + ['decoy'] * r.choice([0, 0, 1])
    r.shuffle(kids)
    for k in kids:
        if k == 'group':
            scriptchain.radio_group(r, sub, 0x12c4b9 if r.random() < 0.97 else 0x12c4b7, (0x12c4ba, 0x12c4bb, 0x12c4bc, 0x12c4bd))
        else:
            dialogbuild.child(r, sub, r.choice([0, 1, 401, r.getrandbits(32) | 0x100]))
    top = []
    kids = ['tabs'] + (['label'] if r.random() < 0.95 else []) + ['decoy'] * r.choice([0, 1, 2])
    r.shuffle(kids)
    for k in kids:
        if k == 'tabs':
            dialogbuild.child(r, top, 0x12c4b8 if r.random() < 0.97 else 0x12c4b9, sub + [5], 160, 1)
        elif k == 'label':
            dialogbuild.child(r, top, 0x12c4be, kind=1)   # (its new rectangle sends it 0x12: kind 1's default handler, which the driver runs)
        else:
            dialogbuild.child(r, top, r.choice([0, 1, 0x12c4bf, r.getrandbits(32) | 0x100]))
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    w += [5] * 4
    regions[0x7508e0] = struct.pack('<%dH' % len(w), *w)
    scriptchain.hover_part(r, regions)
    regions[0x7cb2a0] = r.bytes(4)
    return list(range(0x4e, 0x52))   # (the labels' texts)


def open_case(r):
    """Dialog7ca150_Open (0x495aa0): the tab 0-3, -1 ([0x7ca154] 0-3 mostly, 4 or any) or another; the tab's surroundings with the page
    open_page() lays out (any tab's for a tab outside 0-3); advisor.py's PostId surroundings for 0xbf"""
    tab = r.choice([0, 1, 2, 3] * 3 + [-1] * 4 + [4, r.randrange(5, 1 << 31)])
    state = r.choice([0, 1, 2, 3] * 3 + [4, r.getrandbits(32)])
    shown = state if tab == -1 else tab
    regions = list_dialog(r, list(TABS)[shown if 0 <= shown <= 3 else r.randrange(4)], open_page)
    regions[0x7ca154] = struct.pack('<I', state)
    regions[HEAP] = heap(0x80000, r.choice([0] * 40 + [r.randrange(1, 16)]))   # (two builds' windows and parts)
    masks = scriptchain.advisor_env(r, regions, 0xbf)
    return Call(0x495aa0, [tab & 0xffffffff, r.choice([0, 0, r.getrandbits(32)])], regions, ret='v', masks=masks)


def case(r):
    op = r.deal(OPS)
    if op == 'Dialog7ca150_Open':
        return open_case(r)
    regions = list_dialog(r, op)
    return Call(CALLS[op], [r.choice([0, 0, r.getrandbits(32)])], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The objects' lists dialog's tabs (src/game/ui/dialog7ca150: 0x494ec0, 0x494c70, 0x494250, 0x495110) against the originals.
# list_dialog(): dialogbuild.py's scripted() surroundings (listrows.py's text parts, windowscript.py's build on WINDOW2, the heap, the
# timers, the queues), WINDOW2 the child 0x12c4b8 of the page [0x7ca150] (another id now and then: no page, a fault in both), its child
# 400 before the build a window whose handler is fncmp's logger (another id, or none, now and then); the tab's script at its address
# generated: 0-2 plain children, then the list (kind 7, up to 120 high) whose children are the header buttons 0x10 on (kind 2, one of them
# missing now and then: a fault in both) among 0-1 others, in any order; the kinds' slots 5 and 7 recorders (dialogbuild.py's
# windowat10()); the heap failing rarely (a fault in both); the rows' height [0x7ca158] 20-80 mostly (0 now and then: the font
# [0x750d54]'s slot 1 asked, font 7's vtable a recorder, the divisor [0xfaa5c0] about 1536), the list's inner rectangle (0xa) under 20
# high, so no row is kept (the first one made by List7ca158_NewRow only): a row kept is moved (Window7059f0_SetRow), sent 0x12, and its
# handler 0x4959c0 is unported, stored raw, and lies in the driver's own code natively; the part kinds' unported slots recorders
# (listrows.py's child_case()); the GUI's registered windows: "hilight" and "!cbut"
# each among 0-2 others or not (their window a recorder whose slot 8 gives listrows.py's CUT, an object, or 0); the texts' record table
# about the tab's ids (panelparts.py's texts()); the buttons' font [0x750d58] 0-12, 7 mostly; the sort dword +-1 to +-the columns, 0
# or any; an empty park (the things' callbacks are unported: none called); the selected data 0 or any (the list is new and empty, so it
# is never found: the scroll is not reached).
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
from fncmp import CDECL_LOG4, HEAP, LOG, Bench, Call, global_heap, heap, put, recorder, set_return, vtable  # noqa: E402

CALLS = {'Dialog7ca150_ShowTab0': 0x494ec0, 'Dialog7ca150_ShowTab1': 0x494c70, 'Dialog7ca150_ShowTab2': 0x494250,
         'Dialog7ca150_ShowTab3': 0x495110}
# each tab: (its script, the columns, the sort dword, the first header text)
TABS = {'Dialog7ca150_ShowTab0': (0x750ab8, 5, 0x750d5c, 0x52), 'Dialog7ca150_ShowTab1': (0x750ab8, 5, 0x750d60, 0x57),
        'Dialog7ca150_ShowTab2': (0x750ba0, 6, 0x750d64, 0x5c), 'Dialog7ca150_ShowTab3': (0x750ca0, 2, 0x750d68, 0x62)}
ALL_RANGES = sorted(set(dialogbuild.ALL_RANGES + panelparts.ALL_RANGES + window7059f0.LIST_RANGES + [
    (0x481bc0, 0x481f90), (0x481f80, 0x482040), (0x482040, 0x482110), (0x482100, 0x4821c0), (0x4821b0, 0x482280), (0x482270, 0x482350),
    (0x482340, 0x482410), (0x482400, 0x4824d0), (0x516130, 0x5161b0), (0x50b350, 0x50b360), (0x412e90, 0x412eea), (0x4dd4e0, 0x4dd500),
    (0x4e2710, 0x4e273a), (0x4cd300, 0x4cd331), (0x5da3c0, 0x5da3c1),
    (0x485a70, 0x485ab0), (0x485ac0, 0x485b50), (0x486660, 0x486790), (0x494250, 0x4952a0), (0x4944d0, 0x494690),
    (0x659bc9, 0x659c52), (0x6588b6, 0x6588ef), (0x65d070, 0x65d1c8), (0x668f5a, 0x668f9c), (0x668820, 0x6688b3), (0x668ad3, 0x668bd4),
    (0x6659af, 0x665a54), (0x65dd7b, 0x65e559), (0x65bcad, 0x65c035)]))
B = Bench(ALL_RANGES, hooks=panelparts.animatormesh.HOOKS)
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
    left, top = r.randrange(0, 20), r.randrange(0, 30)   # (under a row high: no rows kept, so none moved, see list_dialog())
    inner += [0xa, left, top, left + r.randrange(20, 200), top + r.choice([r.randrange(0, 20)] * 6 + [0, 1])]
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


def list_dialog(r, op):
    """the tab's surroundings (above); returns the regions"""
    script, columns, sort, first = TABS[op]
    dialogbuild.DIALOGS[op] = (0x7ca14c, script, (0x10,), (0x7ca154,))
    regions = dialogbuild.scripted(r, op)
    log = regions[LOG]
    regions[HEAP] = heap(0x14000, r.choice([0] * 40 + [r.randrange(1, 8)]))   # (an allocation failing: a fault in both, so rarely)
    w = list_script(r, columns)
    regions[script] = struct.pack('<%dH' % len(w), *w)
    for vt in dialogbuild.KIND_VTABLES:   # (the built windows' slots 5 and 7: game code the driver cannot run natively)
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
        regions[vt + 0x1c] = struct.pack('<I', recorder(29, 1))
    # the page [0x7ca150]: WINDOW2 its child 0x12c4b8, WINDOW2's child 400 before the build
    regions[PAGE] = put(bytearray(r.bytes(0x80)), 0x5c, 'I', window.WINDOW2)
    regions[0x7ca150] = struct.pack('<I', PAGE)
    win = regions[window.WINDOW2]
    put(win, 0x8c, 'I', 0x12c4b8 if r.random() < 0.97 else 0x12c4b9)
    put(win, 0x54, 'I', 0)
    if r.random() < 0.8:
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
    for vt in (0x6ff3b0, 0x6ff410):
        regions[vt + 0xc] = struct.pack('<I', recorder(3, 2))
    regions.update(global_heap())   # (the bars' meshes: panelparts.py's surroundings())
    regions[panelparts.MESH_VTABLE + 0xc] = struct.pack('<I', recorder(3, 2))
    regions[panelparts.MESH_VTABLE + 0x14] = struct.pack('<I', recorder(5, 1))
    registry(r, regions, ['hilight', '!cbut'])
    panelparts.texts(r, regions, list(range(first, first + columns)))
    regions[0x750d58] = struct.pack('<I', r.choice([7] * 6 + list(range(13))))
    by = r.choice(list(range(1, columns + 1)) * 2 + [0, r.getrandbits(31)]) * r.choice([1, -1])
    regions[sort] = struct.pack('<I', by & 0xffffffff)
    regions[0x7cf56c] = struct.pack('<I', 0)   # (an empty park)
    return regions


def case(r):
    op = r.deal(OPS)
    regions = list_dialog(r, op)
    return Call(CALLS[op], [r.choice([0, 0, r.getrandbits(32)])], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

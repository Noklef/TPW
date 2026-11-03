# Dialog7cc1f8_Open (src/game/ui/dialog7cc1f8, 0x4acc70) against the original. Dialog7cc1f8_FillList 17, Charset_FormatWide 18 (its number argument logged by its 20 bytes), RadioGroup_Select 19, HoverLabel_Attach 21,
# Window7059f0_FindItemData 25 (its answer the case's) and Window7059f0_ScrollTo 26 recorders: their own benches have them. Open on an open window as dialog7ca308open.py's: its list 0x1f8 and group 0x1f9 among decoys
# (windows of random bytes), each missing now and then. A new one: dialog7ca308open.py's new window at the script 0x754cf8 with the children 0x200, 0x1ea (0x1eb, 0x1ec), the list 0x1f8 (3 columns, its buttons, the group
# 0x1f9; missing now and then), the label 0x1fe (nothing built when the script is empty); the money record's balance; advisor.py's PostId for 0xbc. The kinds 0-3, past them, -1 (the state 0-3 mostly); the selected word 0 or not.
# Window7059f0_BuildRows 27 a recorder too (the rows' handler 0x4acb70 is unported: no row is made; the row functions logged by address). The dialog [0x7cb2b8] now and then (dialogs.py's child 0x21 and 0x25's 0x27), with the
# script mostly empty then: the build's own Dialog7cb2b8_ResetChild21 not run. dialogbuild.long_open(): its calls run past the log's 40.
# usage: dialog7cc1f8open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import dialog7ca150  # noqa: E402
import dialog7ca308open as base  # noqa: E402
import dialogs  # noqa: E402
import dialogbuild  # noqa: E402
import panelparts  # noqa: E402
import scriptchain  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, blob_bytes, heap, log_region, put, recorder, set_return  # noqa: E402

OP = 'Dialog7cc1f8_Open'
dialog7ca150.TABS[OP] = (0x754cf8, 3, 0x755140, 0x7b)
TEXTS = [0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x8a]
B = Bench(dialog7ca150.merged(dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + dialogs.RANGES + [
    (0x668ad3, 0x668c99), (0x65f697, 0x65f6e1), (0x65e6d5, 0x65e73d), (0x6697e3, 0x6698b4), (0x4acc70, 0x4ad204), (0x65d7bc, 0x65d840), (0x5195d0, 0x519610), (0x6ad810, 0x6ad820), (0x4a25f0, 0x4a2700)]),
    hooks={**panelparts.animatormesh.HOOKS, 0x67a7d0: _cdecl_log, 0x4aaf70: _recorder(17, 2, cdecl=True), 0x6acd60: _recorder(18, 5, cdecl=True), 0x66a295: _recorder(19, 1), 0x485d20: _recorder(21, 3, cdecl=True),
           0x664d2a: _recorder(25, 1), 0x665739: _recorder(26, 1), 0x6639cb: _recorder(27, 2)})
WIN, KIDS, SOURCE = RAM + 0x60000, RAM + 0x60200, RAM + 0x61000
DIALOG, DIALOG_KIDS, DIALOG_BUTTONS = RAM + 0x62000, RAM + 0x62200, RAM + 0x63000


def kinds(r, state):
    return r.choice([0, 1, 2, 3] * 3 + [4, state, 0xffffffff, 0xffffffff, r.getrandbits(32)])


def selected(r, log):
    set_return(log, 25, r.choice([-1 & 0xffffffff, 0, 1, 7]))
    return r.choice([0, 0, 1, r.getrandbits(32), r.getrandbits(32) & 0xffff0000])


def open_window(r):
    log = log_region()
    regions = {LOG: log}
    state = r.choice([0, 1, 2, 3, r.getrandbits(32)])
    regions[0x7cc1f8] = struct.pack('<2I', WIN, state)
    ids = ([0x1f8] if r.random() < 0.9 else []) + [r.getrandbits(32) | 0x100 for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(ids)
    kids = base.linked(r, regions, KIDS, ids)
    inner = ([0x1f9] if r.random() < 0.9 else []) + [r.getrandbits(32) | 0x100 for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(inner)
    base.linked(r, regions, KIDS + 0x400, inner)
    if 0x1f8 in kids:
        put(regions[kids[0x1f8]], 0x5c, 'I', KIDS + 0x400 if inner else 0)
    regions[WIN] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS if ids else 0)
    regions[0x7cb2a0] = r.bytes(4)
    return Call(0x4acc70, [kinds(r, state), selected(r, log)], regions, ret='v')


def script(r):
    top = []
    kids = ['line', 'pair', 'label', 'list'] + ['decoy'] * r.choice([0, 0, 1])
    if r.random() < 0.05:
        kids.remove('list')
    r.shuffle(kids)
    for k in kids:
        if k == 'line':
            dialogbuild.child(r, top, 0x200, kind=1)
        elif k == 'label':
            dialogbuild.child(r, top, 0x1fe, kind=1)
        elif k == 'list':
            top += base.child_list(r, 0x1f8, 3, 0x1f9, (0x1fa, 0x1fb, 0x1fc, 0x1fd))
        elif k == 'pair':
            base.group_of(r, top, 0x1ea, (0x1eb, 0x1ec))
        else:
            dialogbuild.child(r, top, r.choice([0, 1, r.getrandbits(32) | 0x1000]))
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    return w + [5] * 4


def reset_dialog(r, regions):
    """the dialog [0x7cb2b8] (dialogs.py's reset_case(), its child 0x21 of the kinds that send no message)"""
    def kid(w, ident):
        w[0x134] = r.choice([3, 4])
        put(w, 0xa8, 'I', r.choice([0, 1]))
        put(w, 0x98, 'II', 0, 0)
        if ident == 0x25:
            put(w, 0x5c, 'I', dialogs.chain(r, regions, DIALOG_BUTTONS, 0x27, [0x26, 0x28, 1], lambda b, i: put(b, 0x98, 'II', 0, 0)))
    w = bytearray(r.bytes(0x120))
    put(w, 0x5c, 'I', dialogs.chain(r, regions, DIALOG_KIDS, 0x25, [0x21], kid))
    regions[DIALOG] = w
    put(regions[0x7cb2ac], 0xc, 'I', DIALOG)


def new_window(r):
    regions = dialog7ca150.list_dialog(r, OP, lambda r, regions: TEXTS)
    dialogbuild.long_open(regions[LOG])
    resets = r.random() < 0.2
    if resets:
        reset_dialog(r, regions)
    w = script(r) if r.random() < (0.3 if resets else 0.97) else [5] * 4
    regions[0x754cf8] = struct.pack('<%dH' % len(w), *w)
    log = regions[LOG]
    regions[HEAP] = heap(0x28000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    state = r.choice([0, 1, 2, 3, r.getrandbits(32)])
    regions[0x7cc1f8] = struct.pack('<2I', 0, state)
    regions[0x7cc1e0] = r.bytes(0xc)
    regions[0x7cc1f0] = r.bytes(4)
    masks = scriptchain.advisor_env(r, regions, 0xbc)
    world = bytearray(r.bytes(8))
    b = 0x100 + r.randrange(0x100)   # (the advisor's own word below it)
    put(world, 6, 'H', b)
    regions[advisor.WORLD + 4] = world
    regions[0x7cfb90 + 20 * b] = struct.pack('<I', SOURCE)
    regions[SOURCE] = put(bytearray(r.bytes(0x10)), 0xc, 'i', r.choice([0, 1, 5000, -1, -5000, -0x80000000, r.randrange(-1 << 31, 1 << 31)]))
    regions[0x755138] = struct.pack('<I', r.choice([7] * 6 + list(range(13))))
    regions[0x755140] = struct.pack('<I', 1)   # (the new list's column 0 ascending: any other changes it, the dialog's handler 0x4ac270 told, unported)
    blob_bytes(log, 18, 4, 0xfffff)
    regions[0x6ff2d8 + 0xc] = struct.pack('<I', recorder(3, 2))   # (the 0x1eb renderer part told: its slots 3 and 5 recorders, as the text parts')
    regions[0x6ff2d8 + 0x14] = struct.pack('<I', recorder(31, 1))
    for vt in dialogbuild.KIND_VTABLES:   # (the money line's slot 9)
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    regions[0x7cb2a0] = r.bytes(4)
    return Call(0x4acc70, [kinds(r, state), selected(r, log)], regions, ret='v', masks=masks)


def case(r):
    return r.deal([new_window, new_window, new_window, open_window])(r)


if __name__ == '__main__':
    B.main(case)

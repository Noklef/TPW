# The 0x7ca1c8 dialog's button selection (src/game/ui/dialog7ca1c8: 0x4981d0, 0x498a30, 0x4819f0) and its loop button (0x4987f0)
# against the originals. The dialog
# [0x7ca1c8] with its child 0x18 a radiogroup.py group (children of the ids 0x19-0x21 and a few others) among another child or two, now and
# then missing (a fault in both); [0x7ca1cc] set or 0 (the IfUp guard; IfUp's dialog now and then 0); flags with one or a few of bits 0-8
# set, none, or any. GameCursor_Set's window [0x7cb2ac] one (not the hovered one [0xfaa5e0]: its word +0x50 and 0x7cb2d8 take the cursor
# id) or now and then 0. The events are masked as radiogroup.py / window705a50.py say.
# UpdateLoopButton (0x4987f0): see loop_case() below.
# usage: dialog7ca1c8.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SelectButton,...,UpdateLoopButton)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, blob_bytes, log_region, put, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from radiogroup import GROUP, RANGES as GROUP_RANGES, group_env, masks, select_states  # noqa: E402
import listrows  # noqa: E402

CALLS = {'SelectButton': 0x4981d0, 'SelectButtonIfUp': 0x498a30, 'SelectButtonIfUpThis': 0x4819f0, 'UpdateLoopButton': 0x4987f0}
RANGES = [(0x4981d0, 0x49829c), (0x498a30, 0x498a50), (0x4819f0, 0x481a00), (0x4a2aa0, 0x4a2ac0), (0x65f11d, 0x65f14e)] + GROUP_RANGES
B = Bench(sorted(set(RANGES + listrows.REG_RANGES + [(0x4987f0, 0x4988b7), (0x444c00, 0x444c60), (0x445910, 0x445a80), (0x65d773, 0x65d832),
                                                     (0x6616b0, 0x661710), (0x6617f0, 0x661820), (0x65d7d0, 0x65d840)])))
LOOP, PANEL, BUTTON, BVT, EDIT, ALT = RAM + 0x20000, RAM + 0x20200, RAM + 0x20400, RAM + 0x20600, RAM + 0x20700, RAM + 0x20800
DIALOG, OTHER, CURSOR_WINDOW = RAM + 0x3000, RAM + 0x3200, RAM + 0x3400
IDS = list(range(0x19, 0x22)) + [0x18, 2]
PICKS = [(4, 0x1b), (1, 0x1f), (2, 0x20), (8, 0x19), (0x10, 0x1a), (0x20, 0x1c), (0x40, 0x21), (0x80, 0x1e), (0x100, 0x1d)]


def loop_case(r):
    """the dialog [0x7ca1c8] (not shown, no parent) with its child 0x18 first or second, that child's children the button 0x20 (its
    vtable a recorder: slot 5, one argument; its object +0x98 none or one released, +0x4c none or an object; shown or not up its parents)
    first or after another; the edit object [0x7ca1d0]'s byte +0x50 0, 1 or any; coaster.py's Tools surroundings (the current link);
    the GUI's registered windows of listrows.py's register() with "cb_trough" or "cb_loop" among them or not"""
    import coaster  # here: coaster.py imports this module for its RANGES at load
    regions = dict(coaster.current(r, 'Tools').regions)
    log = log_region()
    regions[LOG] = log
    listrows.register(r, regions, r.choice(['cb_trough', 'cb_loop']))
    regions[listrows.OBJ98] = put(bytearray(r.bytes(0x20)), 0, 'I', listrows.VT98)
    blob_bytes(log, 3, 1, 0)   # (the released object's slot 3 event: a stack pointer, logged as a hash of none of its bytes, as listrows.py's)
    regions[listrows.OBJ4C] = r.bytes(0x20)
    button = bytearray(r.bytes(0x120))
    put(button, 0, 'I', BVT)
    put(button, 0x8c, 'I', 0x20)
    put(button, 0x98, 'I', r.choice([0, 0, listrows.OBJ98, listrows.CUT]))
    put(button, 0x4c, 'I', r.choice([0, listrows.OBJ4C]))
    put(button, 0x48, 'I', (r.getrandbits(32) & ~1) | r.choice([0, 1, 1]))
    put(button, 0x60, 'I', PANEL)
    put(button, 0x54, 'I', 0)
    put(button, 0x5c, 'I', 0)
    other = put(put(bytearray(r.bytes(0x100)), 0x8c, 'I', r.choice([0x1f, 0x21, 0x18])), 0x54, 'I', BUTTON)
    put(other, 0x5c, 'I', 0)
    regions[BUTTON] = button
    regions[BVT] = vtable({5: 1})
    regions[ALT] = other
    panel = bytearray(r.bytes(0x100))
    put(panel, 0x8c, 'I', 0x18)
    put(panel, 0x5c, 'I', r.choice([BUTTON, ALT]))
    put(panel, 0x48, 'I', (r.getrandbits(32) & ~1) | r.choice([0, 1, 1]))
    put(panel, 0x60, 'I', LOOP)
    put(panel, 0x54, 'I', 0)
    regions[PANEL] = panel
    sibling = put(put(bytearray(r.bytes(0x100)), 0x8c, 'I', r.choice([3, 0x20])), 0x5c, 'I', 0)
    put(sibling, 0x54, 'I', PANEL)
    regions[PANEL + 0x100] = sibling
    regions[LOOP] = put(put(put(bytearray(r.bytes(0x100)), 0x5c, 'I', r.choice([PANEL, PANEL + 0x100])), 0x60, 'I', 0), 0x48, 'I', 0)
    regions[0x7ca1c8] = struct.pack('<I', LOOP)
    regions[EDIT] = put(bytearray(r.bytes(0x60)), 0x50, 'B', r.choice([0, 0, 1, 1, 1, r.getrandbits(8)]))
    regions[0x7ca1d0] = struct.pack('<I', EDIT)
    return Call(CALLS['UpdateLoopButton'], [], regions, ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'UpdateLoopButton':
        return loop_case(r)
    log = log_region()
    regions = {LOG: log}
    g = bytearray(r.bytes(0x160))
    g[0x48] = 0
    put(g, 0x8c, 'I', r.choice([0x18] * 12 + [0x17]))
    g, kids = group_env(r, regions, log, g, IDS)
    other = bytearray(r.bytes(0x90))
    put(other, 0x8c, 'I', r.choice([3, 0x19]))
    put(other, 0x54, 'I', 0)
    regions[OTHER] = other
    if r.random() < 0.5:   # the group first or second
        put(g, 0x54, 'I', OTHER)
        first = GROUP
    else:
        put(g, 0x54, 'I', 0)
        put(other, 0x54, 'I', GROUP)
        first = OTHER
    regions[DIALOG] = put(bytearray(r.bytes(0x60)), 0x5c, 'I', first)
    up = r.choice([1, 1, 1, 0]) if op != 'SelectButton' else 1
    regions[0x7ca1c8] = put(bytearray(8), 0, 'II', DIALOG if op == 'SelectButton' or r.random() < 0.9 else 0, up)
    regions[CURSOR_WINDOW] = bytearray(r.bytes(0x60))
    regions[0x7cb2ac] = put(bytearray(4), 0, 'I', CURSOR_WINDOW if r.random() < 0.85 else 0)
    regions[0xfaa5e0] = bytearray(4)
    regions[0x7cb2d8] = put(bytearray(4), 0, 'I', r.getrandbits(32))
    flags = r.choice([1 << r.randrange(9), (1 << r.randrange(9)) | (1 << r.randrange(9)), 0, r.getrandbits(9), r.getrandbits(32)])
    ident = next((i for bit, i in PICKS if flags & bit), 0xffffffff)
    selected = int.from_bytes(g[0x138:0x13c], 'little')
    masks(log, kids, select_states(kids, selected, ident))
    args = [0x7b51f0, flags] if op == 'SelectButtonIfUpThis' else [flags]
    return Call(CALLS[op], args, regions, conv='t' if op == 'SelectButtonIfUpThis' else 'c', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

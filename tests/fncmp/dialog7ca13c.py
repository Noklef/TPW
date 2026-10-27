# The guests' list dialog's opener (src/game/ui/dialog7ca13c: 0x493530) against the original. dialogbuild.py's focused() surroundings (the
# dialog [0x7ca13c] none mostly; the script's parent [0x7cb2ac]) with the script at 0x7506c8 generated here: 0-2 plain children, then the
# dialog (kind 1) whose children are the list 0x1e497 (kind 7; another id now and then: no list) among 0-2 decoys in any order; the list's
# columns (opcode 0xb: six, 0 now and then: the rows unset, a fault in both), its inner rectangle (0xa) 0-170 high (the rows 0x38 high: 0-3
# kept, List7ca140_InitRow's parts as listrows.py's init_case() makes their surroundings), its header buttons 0x10-0x15 (0xc: kind 2, one
# missing now and then: a fault in both) among 0-1 decoys in any order; the window kinds' slots 5 and 7 recorders (dialogbuild.py's
# windowat10()); the parts' (dialogbuild.py's PART_KINDS: the header buttons' text parts among them) slots 3, 5 and 9 recorders (game code
# the driver cannot run natively); the heap failing rarely (a fault in both); the rows' height [0x7ca140] cached or not (the font [0x7508b4]'s slot 1 asked:
# font 7's vtable a recorder); the GUI's registered windows ("hilight" among others or not: dialog7ca150.py's registry()); the texts' record
# table about 0x71-0x76 (panelparts.py's texts()); the buttons' font [0x7508b8] 0-12, 7 mostly; the sort dword [0x7508bc] 1 (any other
# changes the new list's column or order: Window7059f0_NotifySort's message to the dialog, whose handler 0x493230 is unported, stored
# raw and below g_game); an empty park (the things' callback 0x4937e0 is unported: none called); the selected data 0, 1 or any (the list is new and its
# rows empty: never found); the dword [0x7cb2a0] random (Global7cb28c_Select's); advisor.py's PostId surroundings for 0x11e (its record its own mostly; the stack message's unwritten bytes masked).
# usage: dialog7ca13c.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Open)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import dialog7ca150  # noqa: E402
import dialogbuild  # noqa: E402
import panelparts  # noqa: E402
from fncmp import HEAP, LOG, Bench, _cdecl_log, Call, Mask, global_heap, heap, put, recorder, set_return, vtable  # noqa: E402
from msgtags import ADVISOR_RANGES, TIMER_RANGES  # noqa: E402

CALLS = {'Open': 0x493530}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(sorted(set(dialog7ca150.ALL_RANGES + ADVISOR_RANGES + TIMER_RANGES + [(0x4a0810, 0x4a083d), (0x493530, 0x4937d5), (0x4938c0, 0x4939c7), (0x4939d0, 0x493c03)])),
          hooks={**panelparts.animatormesh.HOOKS, 0x67a7d0: _cdecl_log})
FONTVT = dialog7ca150.FONTVT
ID = 0x11e


def list_script(r):
    """the list (kind 7, id 0x1e497 mostly) and its children (above), as dialogbuild.py's child() writes it into w"""
    n = 6 if r.random() < 0.97 else 0
    inner = [0xb, n]
    x = r.randrange(0, 10)
    for _ in range(n):
        width = r.randrange(5, 40)
        inner += [x, x + width]
        x += width
    left, top = r.randrange(0, 20), r.randrange(0, 30)
    inner += [0xa, left, top, left + r.randrange(20, 200), top + r.choice([r.randrange(0, 170)] * 6 + [0, 0x37, 0x38])]
    heads = list(range(6))
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
    dialogbuild.child(r, w, 0x1e497 if r.random() < 0.97 else r.choice([0, 0x1e496]), inner + [5], 120, 7)
    return w


def case(r):
    op = r.deal(OPS)
    dialogbuild.DIALOGS[op] = (0x7ca13c, 0x7506c8, (0x1e497,), (0x7ca138, 0x7ca134))
    regions = dialogbuild.focused(r, op)
    log = regions[LOG]
    regions[HEAP] = heap(0x80000, r.choice([0] * 40 + [r.randrange(1, 16)]))   # (an allocation failing: a fault in both, so rarely)
    # the script: 0-2 plain children, then the dialog with the list among 0-2 decoys
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
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), inner + [5], 160, 1)
    w += [5] * 4
    regions[0x7506c8] = struct.pack('<%dH' % len(w), *w)
    for vt in dialogbuild.KIND_VTABLES:   # (the built windows' slots 5 and 7: game code the driver cannot run natively)
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
        regions[vt + 0x1c] = struct.pack('<I', recorder(29, 1))
    # the rows: their height cached or font [0x7508b4]'s asked
    regions[0x7ca140] = struct.pack('<I', r.choice([0x38] * 6 + [0, r.getrandbits(32)]))
    regions[0x7508b4] = struct.pack('<I', 7)
    lang = regions[0x78b530] = bytearray(regions[0x78b530])
    group = struct.unpack_from('<I', lang, 4)[0]
    put(lang, 0x1c + (7 + 13 * group) * 0x18, 'I', FONTVT)
    regions[FONTVT] = vtable({1: 0})
    set_return(log, 1, r.randrange(20, 80))
    for vt in dialogbuild.PART_KINDS:   # (the parts' slots 3 / 5 / 9: 0x6ff320's the header buttons' new text parts, ButtonText_Init's)
        regions[vt + 0xc] = struct.pack('<I', recorder(3, 2))
        regions[vt + 0x14] = struct.pack('<I', recorder(5, 1))
        regions[vt + 0x24] = struct.pack('<I', recorder(9, 1))
    regions.update(global_heap())   # (the bars' meshes: panelparts.py's surroundings())
    regions[panelparts.MESH_VTABLE + 0xc] = struct.pack('<I', recorder(3, 2))
    regions[panelparts.MESH_VTABLE + 0x14] = struct.pack('<I', recorder(5, 1))
    dialog7ca150.registry(r, regions, ['hilight'])
    panelparts.texts(r, regions, list(range(0x71, 0x77)))
    regions[0x7508b8] = struct.pack('<I', r.choice([7] * 6 + list(range(13))))
    regions[0x7508bc] = struct.pack('<I', 1)   # (the new list's column 0 ascending: any other changes it, the dialog's handler 0x493230 told)
    regions[0x7cf56c] = struct.pack('<I', 0)   # (an empty park)
    regions[0x7cb2a0] = r.bytes(4)   # (Global7cb28c_Select(2, 6)'s dword)
    # the advisor (advisor.py's PostId surroundings, the id 0x11e)
    adv = advisor.queue(r, 'PostId')
    for k, v in adv.regions.items():
        if k != LOG:
            regions[k] = v
    table = bytearray(regions[0x76e300])
    if r.random() < 0.7:   # (0x11e's record its own mostly)
        put(table, 0x38 * ID + 4, 'I', ID)
    regions[0x76e300] = table
    set_return(log, 20, r.choice([0, 1, 5, 100, -1, r.getrandbits(32)]))
    masks = [Mask(m.region, m.offsets + [0x16d0 + k for k in (2, 3, 0x17)], m.reason) for m in adv.masks]
    return Call(CALLS[op], [r.choice([0, 0, 1, r.getrandbits(32)])], regions, ret='v', masks=masks)


if __name__ == '__main__':
    B.main(case)

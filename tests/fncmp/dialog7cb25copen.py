# Dialog7cb25c_Open (src/game/ui/dialog7cb25c, 0x49fb30) against the original. Setting_ReadMode is the port (the advisor reads it too), crt_atexit fncmp._cdecl_log. Dialog7cb25c_FillList 17, RadioGroup_Select 19, Dialog7cb25c_SetTexts 20, HoverLabel_Attach 21,
# Money_LoanOffer 22 (fills the out +0x10 / +0x14 of each offer in turn, answers the out) and Window7059f0_BuildRows 27 recorders: their own benches have them. dialog7ca308open.py's new window with the script at 0x7525a0:
# 0-2 plain children, then the dialog with the label 0x12c4ca, the list 0x12c4bd (6 columns, its buttons, the group 0x12c4be), the figures 0x12c4c1 (eight parts; missing now and then), the script empty now and then
# (nothing built); dialog7ca308open.py's rings; the guard and the mode setting (2: nothing built), the dialog open now and then; the flag [0x7cb260]; advisor.py's PostId for 0xc3. dialogbuild.long_open(): its calls run past the log's 40.
# usage: dialog7cb25copen.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import dialog7ca150  # noqa: E402
import dialog7ca308open as base  # noqa: E402
import dialogbuild  # noqa: E402
import panelparts  # noqa: E402
import scriptchain  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, echo, fill, heap, put, recorder, set_return, strings  # noqa: E402

OP = 'Dialog7cb25c_Open'
dialog7ca150.TABS[OP] = (0x7525a0, 6, 0x752930, 0xac)
TEXTS = [0xaa, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0x165, 0x167, 0x168, 0x169]
B = Bench(dialog7ca150.merged(dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + [
    (0x49fb30, 0x4a02a1), (0x65d7bc, 0x65d840), (0x519510, 0x519550), (0x5195d0, 0x519610)]),
    hooks={**panelparts.animatormesh.HOOKS, 0x67a7d0: _cdecl_log, 0x49fa00: _recorder(17, 1, cdecl=True), 0x66a295: _recorder(19, 1), 0x49f860: _recorder(20, 0, cdecl=True),
           0x485d20: _recorder(21, 3, cdecl=True), 0x4d06b0: _recorder(22, 2), 0x6639cb: _recorder(27, 2)})
RINGS, SOURCE = 0x33100000, RAM + 0x61000


def script(r):
    top = []
    kids = ['label', 'list', 'figures'] + ['decoy'] * r.choice([0, 0, 1])
    if r.random() < 0.05:
        kids.remove('figures')
    if r.random() < 0.02:
        kids.remove('list')
    r.shuffle(kids)
    for k in kids:
        if k == 'label':
            dialogbuild.child(r, top, 0x12c4ca, kind=1)
        elif k == 'list':
            top += base.child_list(r, 0x12c4bd, 6, 0x12c4be, (0x12c4bf, 0x12c4c0))
        elif k == 'figures':
            base.group_of(r, top, 0x12c4c1, (0x12c4c2, 0x12c4c3, 0x12c4c4, 0x12c4c5, 0x12c4c6, 0x12c4c7, 0x12c4c8, 0x12c4c9))
        else:
            dialogbuild.child(r, top, r.choice([0, 1, r.getrandbits(32) | 0x1000]))
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    return w + [5] * 4


def case(r):
    regions = dialog7ca150.list_dialog(r, OP, lambda r, regions: TEXTS)
    w = script(r) if r.random() < 0.97 else [5] * 4
    regions[0x7525a0] = struct.pack('<%dH' % len(w), *w)
    log = regions[LOG]
    dialogbuild.long_open(log)
    regions[0x7cb290] = r.bytes(4)   # (Global7cb28c_Select(3, ...)'s dword)
    regions[HEAP] = heap(0x28000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    regions[0x7cb25c] = struct.pack('<II', r.choice([0] * 12 + [r.getrandbits(32)]), r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[0xfb34c2] = bytes([r.choice([0, 1, 1, 1, r.getrandbits(8)])])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([2] + [0, 1, 3, r.getrandbits(32)] * 4))
    masks = scriptchain.advisor_env(r, regions, 0xc3)
    world = bytearray(r.bytes(8))
    a, b = r.sample(range(0x100), 2)
    a, b = a + 0x100, b + 0x100   # (the advisor's own word below them)
    put(world, 0, 'H', a)
    put(world, 6, 'H', b)
    regions[advisor.WORLD + 4] = world
    regions[0x7cfb90 + 20 * a] = struct.pack('<I', RINGS)
    regions[0x7cfb90 + 20 * b] = struct.pack('<I', SOURCE)
    regions[RINGS + base.RING_LOW] = base.rings(r)
    regions[0x752938] = struct.pack('<I', r.choice([7] * 6 + list(range(13))))
    echo(log, 22, 0)
    strings(log, 22, 0, 1)
    fill(log, 22, 0, b''.join(struct.pack('<II', r.getrandbits(32), r.choice([0, 1, r.getrandbits(32)])) for _ in range(8)), offset=0x10, records=8)
    for vt in dialogbuild.KIND_VTABLES:   # (the label's slot 9)
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    regions[0x7cb2a0] = r.bytes(4)
    return Call(0x49fb30, [], regions, ret='v', masks=masks)


if __name__ == '__main__':
    B.main(case)

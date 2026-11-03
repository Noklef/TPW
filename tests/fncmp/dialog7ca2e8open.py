# Dialog7ca2e8_Open (src/game/ui/dialog7ca2e8, 0x49ac60) against the original. Setting_ReadMode is the port (the advisor reads it too), crt_atexit fncmp._cdecl_log, RadioGroup_Select 19 and Window705a50_SetState 20 recorders: their own benches have them. dialog7ca308open.py's new window with
# the script at 0x751ca8: 0-2 plain children, then the dialog with the graph 0x12272 (missing now and then: only the advice posted), in it the group 0x12273 (missing now and then) and the eight buttons, and the child 0x1227f; the script empty now and then (nothing built); [0x7ca2ec] and the
# flags [0x751fa0]; the guard and the mode setting, the dialog open now and then; advisor.py's PostId for 0xc5. dialogbuild.long_open(): the calls run past the log.
# usage: dialog7ca2e8open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7ca150  # noqa: E402
import dialog7ca308open as base  # noqa: E402
import dialogbuild  # noqa: E402
import panelparts  # noqa: E402
import scriptchain  # noqa: E402
from fncmp import HEAP, LOG, LOG_BYTES, Bench, Call, _cdecl_log, _recorder, heap, recorder  # noqa: E402

OP = 'Dialog7ca2e8_Open'
dialog7ca150.TABS[OP] = (0x751ca8, 6, 0x752930, 0xac)
TEXTS = [0x16a, 0x16b, 0x16c] + list(range(0xa2, 0xaa))
BUTTONS = (0x12277, 0x1227b, 0x12279, 0x12278, 0x1227c, 0x1227d, 0x1227a, 0x1227e)
B = Bench(dialog7ca150.merged(dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + [(0x49ac60, 0x49b51e), (0x65d7bc, 0x65d840)]),
          hooks={**panelparts.animatormesh.HOOKS, 0x67a7d0: _cdecl_log, 0x66a295: _recorder(19, 1), 0x668bec: _recorder(20, 2)})


def script(r):
    inner = []
    kids = list(BUTTONS) + (['group'] if r.random() < 0.9 else []) + ['decoy'] * r.choice([0, 0, 1])
    r.shuffle(kids)
    for k in kids:
        if k == 'group':
            base.group_of(r, inner, 0x12273, (0x12275, 0x12276, 0x12274))
        elif k == 'decoy':
            dialogbuild.child(r, inner, r.choice([0, 1, r.getrandbits(32) | 0x1000]))
        else:
            dialogbuild.child(r, inner, k, kind=1)
    top = []
    tops = ['graph', 'last'] if r.random() < 0.93 else ['last']
    r.shuffle(tops)
    for k in tops:
        if k == 'graph':
            dialogbuild.child(r, top, 0x12272, inner + [5], 160, 1)
        else:
            dialogbuild.child(r, top, 0x1227f, kind=1)
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    return w + [5] * 4


def case(r):
    regions = dialog7ca150.list_dialog(r, OP, lambda r, regions: TEXTS)
    w = script(r) if r.random() < 0.97 else [5] * 4
    regions[0x751ca8] = struct.pack('<%dH' % len(w), *w)
    log = regions[LOG]
    dialogbuild.long_open(log)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 20, 0b11)   # (Window705a50_SetState's two bytes)
    regions[0x7cb290] = r.bytes(4)   # (Global7cb28c_Select(3, ...)'s dword)
    regions[HEAP] = heap(0x28000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    regions[0x7ca2e8] = struct.pack('<II', r.choice([0] * 12 + [r.getrandbits(32)]), r.choice([0, 1, 2, 2, 3, 0xffffffff, r.getrandbits(32)]))
    regions[0x751fa0] = bytes([r.choice([0, 1, 0xff, 0x80, r.getrandbits(8), r.getrandbits(8), r.getrandbits(8)])])
    regions[0xfb34c3] = bytes([r.choice([0, 1, 1, 1, r.getrandbits(8)])])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([2] * 4 + [0, 1, 3, r.getrandbits(32)] * 3))
    masks = scriptchain.advisor_env(r, regions, 0xc5)
    for vt in dialogbuild.KIND_VTABLES:   # (the buttons' slot 9)
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    regions[0x6ff2d8 + 0xc] = struct.pack('<I', recorder(3, 2))   # (the graph's renderer part told: its slots 3 and 5 recorders, as the text parts')
    regions[0x6ff2d8 + 0x14] = struct.pack('<I', recorder(31, 1))
    return Call(0x49ac60, [], regions, ret='v', masks=masks)


if __name__ == '__main__':
    B.main(case)

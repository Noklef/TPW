# The game panel's level box opened (src/game/ui/panel7cc218: Panel7cc218_OpenLevelBox, 0x4ae560) against the original. dialog7ca308open.py's new window with the script at 0x755750 on +0x10: 0-2 plain children, the box with the label 0x1f4e and the list 0x1f4d (two columns);
# Window7059f0_BuildRows 27 and Panel7cc218_FillLevels 19 recorders (their own benches have them), the object +0x20's slot 5 recorder 31; +0x24 / +0xc set now and then. dialogbuild.long_open(): the calls run past the log's 40.
# usage: panel7cc218levelboxopen.py [CASES [SEED]] [--wild|--both] [--show N]
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
import window  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, heap, put, recorder, vtable  # noqa: E402

OP = 'Panel7cc218_OpenLevelBox'
dialog7ca150.TABS[OP] = (0x755750, 2, 0x752930, 0xac)
TEXTS = [0x1f]
B = Bench(dialog7ca150.merged(dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + [(0x4ae560, 0x4ae6fb), (0x65d7bc, 0x65d840), (0x48d860, 0x48d870), (0x492d20, 0x492d50), (0x65f16b, 0x65f246)]),
          hooks={**panelparts.animatormesh.HOOKS, 0x67a7d0: _cdecl_log, 0x6639cb: _recorder(27, 2), 0x4ae140: _recorder(19, 4, cdecl=True)})
SELF, MEMBER, VT, THING = RAM + 0x60000, RAM + 0x60100, RAM + 0x60200, RAM + 0x60300


def script(r):
    top = base.child_list(r, 0x1f4d, 2, 0x1f4f, (0x1f50, 0x1f51))
    if r.random() < 0.5:
        label = []
        dialogbuild.child(r, label, 0x1f4e, kind=1)
        top = label + top
    else:
        dialogbuild.child(r, top, 0x1f4e, kind=1)
    if r.random() < 0.3:
        dialogbuild.child(r, top, r.choice([0, 1, r.getrandbits(32) | 0x1000]))
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    return w + [5] * 4


def case(r):
    regions = dialog7ca150.list_dialog(r, OP, lambda r, regions: TEXTS)
    w = script(r)
    regions[0x755750] = struct.pack('<%dH' % len(w), *w)
    log = regions[LOG]
    dialogbuild.long_open(log)
    regions[HEAP] = heap(0x28000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    regions[VT] = put(vtable({}, size=0x20), 0x14, 'I', recorder(31, 1))
    regions[MEMBER] = struct.pack('<I', VT)
    me = bytearray(r.bytes(0x40))
    put(me, 0xc, 'I', r.choice([0] * 12 + [r.getrandbits(32)]))
    put(me, 0x10, 'I', window.WINDOW2)
    put(me, 0x20, 'I', MEMBER)
    put(me, 0x24, 'I', r.choice([0] * 12 + [r.getrandbits(32)]))
    regions[SELF] = me
    regions[THING] = r.bytes(0x20)
    regions[0x7c2658] = struct.pack('<I', THING)
    for vt in dialogbuild.KIND_VTABLES:   # (the label's slot 9)
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    return Call(0x4ae560, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

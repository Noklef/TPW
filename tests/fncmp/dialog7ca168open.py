# Dialog7ca168_Open (src/game/ui/dialog7ca168, 0x496620) against the original. Things_ForEachSelected 22 (cdecl; the callback by address), Things_AverageStaffStat1f8 23 (its double in st0) and
# Things_AverageStat1f8OfKind 24 (stdcall: its ECX not logged) recorders: their own benches have them, so the park need not exist. dialog7ca308open.py's new window with the script at 0x750e10: 0-2 plain
# children, then the dialog with the list 0x321 (5 columns, its buttons, the group 0x322 among decoys; missing now and then) and the panel 0x329 (0x32a, 0x32b, 0x32c, 0x32d), the script empty now and
# then (nothing built); dialog7cb2b8.py's timer pool; the tab 0-4, past them, -1 (the state 0-4 mostly); the selected data 0, one or any; Window7059f0_FindItemData 25 (its answer the case's) and Window7059f0_ScrollTo 26 recorders;
# advisor.py's PostId for 0x11d. dialogbuild.long_open(): its calls run past the log's 40.
# usage: dialog7ca168open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import dialog7ca150  # noqa: E402
import dialog7ca308open as base  # noqa: E402
import dialog7cb2b8  # noqa: E402
import dialogbuild  # noqa: E402
import msgtags  # noqa: E402
import panelparts  # noqa: E402
import scriptchain  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, heap, put, recorder, set_float_return, set_return  # noqa: E402

OP = 'Dialog7ca168_Open'
dialog7ca150.TABS[OP] = (0x750e10, 5, 0x75118c, 0x65)
TEXTS = list(range(0x65, 0x70)) + [0x6a]
NODE, POOL, CLOCK = RAM + 0x70000, RAM + 0x70100, RAM + 0x70200
B = Bench(dialog7ca150.merged(dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + [
    (0x496620, 0x496bbe), (0x496c10, 0x496d16), (0x496d20, 0x496e30), (0x65d7bc, 0x65d840), (0x519510, 0x519550), (0x67a830, 0x67a860),
    ] + msgtags.TIMER_RANGES),
    hooks={**panelparts.animatormesh.HOOKS, 0x67a7d0: _cdecl_log, 0x481bc0: _recorder(22, 2, cdecl=True), 0x4c7cc0: _recorder(23, 1), 0x4c9cc0: _recorder(24, 1, True),
           0x664d2a: _recorder(25, 1), 0x665739: _recorder(26, 1)})


def script(r):
    top = []
    kids = ['panel'] + (['list'] if r.random() < 0.95 else []) + ['decoy'] * r.choice([0, 0, 1])
    r.shuffle(kids)
    for k in kids:
        if k == 'list':
            top += base.child_list(r, 0x321, 5, 0x322, (0x323, 0x324, 0x325, 0x326, 0x327))
        elif k == 'panel':
            base.group_of(r, top, 0x329, (0x32a, 0x32b, 0x32c, 0x32d))
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
    regions[0x750e10] = struct.pack('<%dH' % len(w), *w)
    log = regions[LOG]
    dialogbuild.long_open(log)
    set_return(log, 25, r.choice([-1 & 0xffffffff, 0, 1, 7]))
    regions[HEAP] = heap(0x28000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    state = r.choice([0, 1, 2, 3, 4, 4, r.getrandbits(32)])
    tab = r.choice([0, 1, 2, 3, 4] * 3 + [5, 0xffffffff, 0xffffffff, r.getrandbits(32)])
    regions[0x7ca164] = put(bytearray(r.bytes(0xc)), 8, 'I', state)
    masks = scriptchain.advisor_env(r, regions, 0x11d)
    world = bytearray(r.bytes(4))
    a = 0x100 + r.randrange(0x100)   # (the advisor's own word below it)
    put(world, 0, 'H', a)
    regions[advisor.WORLD + 4] = world
    regions[0x7cfb90 + 20 * a] = struct.pack('<I', r.getrandbits(31) | 1)
    regions[0x751188] = struct.pack('<I', r.choice([7] * 6 + list(range(13))))
    regions[0x75118c] = struct.pack('<I', 1)   # (the new list's column 0 ascending: any other changes it, the dialog's handler 0x495da0 told, unported)
    set_float_return(log, 23, r.choice([0.0, 1.0, 50.0, 99.5, 100.0, 255.0, r.uniform(0, 255)]))
    set_return(log, 24, r.choice([0, 1, 50, 99, 100, 255, r.randrange(256), r.getrandbits(32)]))
    for vt in dialogbuild.KIND_VTABLES:   # (the label's slot 9)
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    dialog7cb2b8.timer_env(r, regions, log, bytearray(0x130), NODE, POOL, CLOCK)   # (the new window's own node 0: the pool's)
    regions[0x7cb2a0] = r.bytes(4)
    return Call(0x496620, [tab, r.choice([0, 0, 1, r.getrandbits(32)])], regions, ret='v', masks=masks)


if __name__ == '__main__':
    B.main(case)

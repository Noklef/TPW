# Dialog7cca70_Open (src/game/ui/dialog7cca70, 0x4c2110) against the original. Recorders, as Dialog7cca70Env's: 0x5bcd10 11 (agent B's), Window_SetFocus 13, BuildRows 27, SetColumnBottom 14, SetHighlight 15, ButtonText_Init 16, Charset_FormatWide 18 (its argument hashed), HoverLabel_Attach 19,
# Class702da0_Push 20; the session's constructor 10 (onlineinstance.py). dialog7ca308open.py's new window with the script at 0x7593e8: 0-2 plain children, then the dialog with the list 0x14beb, its children 0x10-0x14.
# usage: dialog7cca70.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7ca150  # noqa: E402
import dialogbuild  # noqa: E402
import panelparts  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, heap, set_return  # noqa: E402

OP = 'Dialog7cca70_Open'
dialog7ca150.TABS[OP] = (0x7593e8, 6, 0x752930, 0xac)
TEXTS = list(range(0x126, 0x12b))
B = Bench(dialog7ca150.merged(dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + [(0x4c2110, 0x4c234c), (0x48b390, 0x48b3fd)]),
          hooks={**panelparts.animatormesh.HOOKS, 0x5b50b0: _recorder(10, 0), 0x5bcd10: _recorder(11, 3), 0x65e59b: _recorder(13, 1), 0x6639cb: _recorder(27, 2), 0x6636b2: _recorder(14, 2), 0x662eee: _recorder(15, 1),
                 0x486660: _recorder(16, 7, cdecl=True), 0x6acd60: _recorder(18, 5, cdecl=True), 0x485d20: _recorder(19, 3, cdecl=True), 0x5d6110: _recorder(20, 2)})
GAME, QUEUE, ROOT, FRAME, SESSION = RAM + 0x61000, RAM + 0x61100, RAM + 0x61200, RAM + 0x61300, RAM + 0x61400


def script(r):
    inner = []
    ids = [0x10, 0x11, 0x12, 0x13, 0x14] + [r.getrandbits(32) | 0x1000 for _ in range(r.choice([0, 0, 1]))]
    r.shuffle(ids)
    for k in ids:
        dialogbuild.child(r, inner, k, kind=1)
    top = []
    dialogbuild.child(r, top, 0x14beb, inner + [5], 160, 1)
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), kind=r.choice([1] * 6 + [2, 3, 4] + list(range(6, 14))))   # (not 5: its constructor calls Window_SetFocus, here a recorder in the original and the port itself in the build)
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    return w + [5] * 4


def case(r):
    regions = dialog7ca150.list_dialog(r, OP, lambda r, regions: TEXTS)
    w = script(r)
    regions[0x7593e8] = struct.pack('<%dH' % len(w), *w)
    log = regions[LOG]
    dialogbuild.long_open(log)
    blob(log, 18, 4, 0b1111)
    regions[HEAP] = heap(0x28000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    # (the frame's window +4 is where the script is built: the same window as the other dialogs' [0x7cb2ac])
    parent = struct.unpack('<I', regions[0x7cb2ac][:4])[0]
    regions[ROOT] = r.bytes(4) + struct.pack('<I', parent)
    regions[0xf82884] = struct.pack('<I', ROOT)
    regions[0x7cca70] = r.bytes(4)
    regions[0x7cca8c] = r.bytes(4)
    regions[FRAME] = r.bytes(0x20)
    regions[0xf82888] = struct.pack('<I', FRAME)
    regions[SESSION] = r.bytes(0x400)
    regions[0xf7d88c] = struct.pack('<I', r.choice([0, SESSION, SESSION]))
    regions[GAME] = r.bytes(0x14) + struct.pack('<I', QUEUE)
    regions[0xf85614] = struct.pack('<I', GAME)
    set_return(log, 10, SESSION)
    return Call(0x4c2110, [r.getrandbits(32)], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

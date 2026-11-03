# Dialog7cc944_Open (src/game/ui/dialog7cc944, 0x4bff70) against the original. Recorders, as Dialog7cc944Env's: Window705a20_SetBuffer 11, SetCaret 12, Window_SetFocus 13, BuildRows 27, SetColumnBottom 14, SetHighlight 15, ButtonText_Init 16, AddItem 17 (its texts hashed), Charset_FormatWide 18,
# HoverLabel_Attach 19, Class702da0_Push 20; the session's constructor 10 (onlineinstance.py). dialog7ca308open.py's new window with the script at 0x758f40: 0-2 plain children, then the dialog with the edit 0xe05a and the list 0xe05b, its children 0x10-0x15; the dialog open now and then.
# usage: dialog7cc944.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7ca150  # noqa: E402
import dialogbuild  # noqa: E402
import panelparts  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, heap, recorder, set_return  # noqa: E402

OP = 'Dialog7cc944_Open'
dialog7ca150.TABS[OP] = (0x758f40, 6, 0x752930, 0xac)
TEXTS = list(range(0x12c, 0x132)) + [0x133]
B = Bench(dialog7ca150.merged(dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + [(0x4bff70, 0x4c02c9), (0x65d7bc, 0x65d840), (0x491170, 0x491220), (0x48b390, 0x48b3fd), (0x5b6480, 0x5b6490)]),
          hooks={**panelparts.animatormesh.HOOKS, 0x5b50b0: _recorder(10, 0), 0x6662b7: _recorder(11, 2), 0x66656c: _recorder(12, 1), 0x65e59b: _recorder(13, 1), 0x6639cb: _recorder(27, 2), 0x6636b2: _recorder(14, 2), 0x662eee: _recorder(15, 1),
                 0x486660: _recorder(16, 7, cdecl=True), 0x66403b: _recorder(17, 4), 0x6acd60: _recorder(18, 4, cdecl=True), 0x485d20: _recorder(19, 3, cdecl=True), 0x5d6110: _recorder(20, 2)})
GAME, QUEUE, ROOT = RAM + 0x61000, RAM + 0x61100, RAM + 0x61200


def plain(r):
    """a decoy's kind: not 5, whose constructor calls Window_SetFocus, here a recorder in the original and the port itself in the build"""
    return r.choice([1] * 6 + [2, 3, 4] + list(range(6, 14)))


def script(r):
    inner = []
    ids = [0x10, 0x11, 0x12, 0x13, 0x14, 0x15] + [r.getrandbits(32) | 0x1000 for _ in range(r.choice([0, 0, 1]))]
    r.shuffle(ids)
    for k in ids:
        dialogbuild.child(r, inner, k, kind=1)
    top = []
    tops = ['edit', 'list'] + ['decoy'] * r.choice([0, 0, 1])
    r.shuffle(tops)
    for k in tops:
        if k == 'edit':
            dialogbuild.child(r, top, 0xe05a, kind=1)
        elif k == 'list':
            dialogbuild.child(r, top, 0xe05b, inner + [5], 160, 1)
        else:
            dialogbuild.child(r, top, r.getrandbits(32) | 0x1000, kind=plain(r))
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), kind=plain(r))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    return w + [5] * 4


def case(r):
    regions = dialog7ca150.list_dialog(r, OP, lambda r, regions: TEXTS)
    w = script(r)
    regions[0x758f40] = struct.pack('<%dH' % len(w), *w)
    log = regions[LOG]
    dialogbuild.long_open(log)
    blob(log, 17, 0, 0b111111)
    regions[HEAP] = heap(0x28000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    # (the frame's window +4 is where the script is built: the same window as the other dialogs' [0x7cb2ac])
    parent = struct.unpack('<I', regions[0x7cb2ac][:4])[0]
    regions[ROOT] = r.bytes(4) + struct.pack('<I', parent)
    regions[0xf82884] = struct.pack('<I', ROOT)
    regions[0x7cc944] = struct.pack('<II', r.choice([0] * 12 + [r.getrandbits(32)]), r.getrandbits(32))
    regions[0xf7d88c] = struct.pack('<I', r.choice([0, RAM + 0x61300, RAM + 0x61300]))
    regions[RAM + 0x61300] = r.bytes(0x20)
    set_return(log, 10, RAM + 0x61300)
    regions[GAME] = r.bytes(0x14) + struct.pack('<I', QUEUE)
    regions[0xf85614] = struct.pack('<I', GAME)
    for vt in dialogbuild.KIND_VTABLES:   # (the edit's slot 9)
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    return Call(0x4bff70, [r.getrandbits(32)], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

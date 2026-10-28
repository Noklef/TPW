# The dialogs built hidden at start-up (src/game/ui/dialog7ca1bc: 0x497b20, dialog7ca1e0: 0x498b50, dialog7ca1c8: 0x498790) against the
# originals: dialogbuild.py's scripted() surroundings with the GUI root [0xfaa5d8] its parent window WINDOW2 and the template generated at
# the dialog's address: 0-2 plain children, then the dialog (kind 1, hidden: hiding a shown one would tell its handler, a game address the
# driver cannot run natively) with 0-3 children of its own; the window kinds' slots 5 recorders, the GUI manager [0x77c48c] told of hidden
# windows (slot 13 a recorder). The heap fails now and then (mostly in the build: a fault in both).
# usage: hiddendialogs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Dialog7ca1bc_Create,Dialog7ca1e0_Create,Dialog7ca1c8_Create)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialogbuild  # noqa: E402
from dialog7cc444 import KIND_VTABLES, merged  # noqa: E402
from fncmp import RAM, Bench, Call, recorder, vtable  # noqa: E402
from thingbrowser import hidden  # noqa: E402

CALLS = {'Dialog7ca1bc_Create': 0x497b20, 'Dialog7ca1e0_Create': 0x498b50, 'Dialog7ca1c8_Create': 0x498790}
TEMPLATES = {'Dialog7ca1bc_Create': (0x7ca1bc, 0x7514c0), 'Dialog7ca1e0_Create': (0x7ca1e0, 0x751720),
             'Dialog7ca1c8_Create': (0x7ca1c8, 0x751530)}
B = Bench(merged(dialogbuild.ALL_RANGES + [(0x65f538, 0x65f5b7), (0x497b20, 0x497b5e), (0x498b50, 0x498b8e), (0x498790, 0x4987ce)]))
MANAGER = RAM + 0x30c00
for _op, (_address, _template) in TEMPLATES.items():
    dialogbuild.DIALOGS[_op] = (_address, _template, (0x100,), ())   # (scripted()'s template, replaced below)


def case(r):
    op = r.deal(OPS)
    regions = dialogbuild.scripted(r, op)
    regions[0xfaa5d8] = struct.pack('<I', dialogbuild.window.WINDOW2) + regions[0xfaa5d8][4:]
    for vt in KIND_VTABLES:   # (the built windows' slots 5: game code the driver cannot run natively)
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
    regions[0x77c48c] = struct.pack('<I', MANAGER)   # (the manager told of a hidden window: its slot 13 a recorder)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40)
    regions[MANAGER + 0x40] = vtable({13: 1})
    w, inner = [], []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    for _ in range(r.choice([0, 1, 2, 3])):
        dialogbuild.child(r, inner, r.choice([0, 1, 0x18, r.getrandbits(32)]))
    hidden(r, w, r.choice([0, 1, r.getrandbits(32)]), inner + [5], 160)
    w += [5] * 4
    regions[TEMPLATES[op][1]] = struct.pack('<%dH' % len(w), *w)
    return Call(CALLS[op], [], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

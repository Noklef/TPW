# The window [0x7cc47c]'s builder (src/game/ui/dialog7cc47c: 0x4b8520) against the original. Create: dialogbuild.py's scripted()
# surroundings (the window [0x7cc47c] none mostly; the script's parent the frame [0xf82884]'s window +4, WINDOW2, now and then none) with
# the script at 0x757d88 generated here: 0-2 plain children, then a kind 1 window whose children are the labels 0x49d9 and 0x49da (kind 1
# mostly) among 0-2 decoys, now and then one of them missing (a fault in both); the window kinds' slots 5, 7 and 9 recorders and the text
# parts' unported slots (as dialog7cc5f4.py's). The heap fails now and then (in the script's build: a fault in both).
# usage: dialog7cc47c.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Create)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialog7cc5f4  # noqa: E402
import dialogbuild  # noqa: E402
from fncmp import HEAP, Bench, Call, heap, put, recorder  # noqa: E402

CALLS = {'Create': 0x4b8520}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(dialog7cc5f4.merged(dialog7cc5f4.RANGES + [(0x4b8520, 0x4b869a)]))
dialogbuild.DIALOGS['Dialog7cc47c_Create'] = (0x7cc47c, 0x757d88, (0x49d9,), ())   # (scripted()'s script, replaced below)


def case(r):
    r.deal(OPS)
    regions = dialogbuild.scripted(r, 'Dialog7cc47c_Create')
    regions[HEAP] = heap(0x80000, r.choice([0] * 40 + [r.randrange(1, 16)]))   # (a failure lands in the build: a fault in both)
    if r.random() < 0.05:
        regions[dialogbuild.FRAME] = put(bytearray(regions[dialogbuild.FRAME]), 4, 'I', 0)
    for vt in dialog7cc5f4.KIND_VTABLES:   # (the built windows' slots 5, 7 and 9: game code the driver cannot run natively)
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
        regions[vt + 0x1c] = struct.pack('<I', recorder(29, 1))
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    for vt, slots in dialog7cc5f4.PART_SLOTS.items():
        for k, n in slots.items():
            regions[vt + 4 * k] = struct.pack('<I', recorder(k, n))
    top = []
    dialog7cc5f4.children(r, top, (0x49d9, 0x49da), {0x49d9: r.choice([1, 1, 1, 2, 6, 9, 13]), 0x49da: r.choice([1, 1, 1, 2, 6, 9, 13])},
                          {})
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    w += [5] * 4
    regions[0x757d88] = struct.pack('<%dH' % len(w), *w)
    return Call(CALLS['Create'], [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

# The window [0x7cc444]'s builder (src/game/ui/dialog7cc444: 0x4b7b00) against the original. Create: dialogbuild.py's focused()
# surroundings (the window none mostly; the script's parent [0x7cb2ac]) with the script at 0x7579c8 generated here: 0-2 plain children,
# then the dialog (kind 1) whose children are 0xadb3 (kind 1, the hover label: the shared text part [0x7c2118] random bytes, its vtable a
# recorder) and 0xada1 (kind 1; its own children the seventeen texts and figures, kind 1 mostly, among 0-2 decoys), in any order among 0-2
# decoys, now and then one of them missing (a fault in both, or for 0xada1 nothing more); the window kinds' slots 5 and 7 recorders (game
# code the driver cannot run natively), the 0x6ff350 / 0x6ff3b0 text parts' slots 3 and 9 too (unported; slot 3's events hashed with the
# bytes each notification sets: window.py's notify_inner()); the language texts 0xba-0xc1
# (panelparts.py's texts()). The game record [0x7cf83c]'s analyser (its handle +0x1da720 into the table 0x7cfb90) with its CircBufU32s
# 0, 1, 2, 5 and 15: the cursor -1..143 (now and then 0-12, the wrap's edge), the entries 0, 1, 12, 13 or 144 mostly (now and then any),
# wrapped or not. The heap fails now and then (in the script's build, a fault in both: its countdown cannot reach the parts).
# usage: dialog7cc444.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Create)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialogbuild  # noqa: E402
import focus  # noqa: E402
import panelparts  # noqa: E402
import window  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, put, recorder, vtable  # noqa: E402

CALLS = {'Create': 0x4b7b00}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def merged(ranges):
    out = []
    for lo, hi in sorted(set(ranges)):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged(dialogbuild.ALL_RANGES + panelparts.ALL_RANGES + focus.MODES + focus.RANGES + [
    (0x4b7b00, 0x4b8469), (0x519510, 0x519550), (0x485d20, 0x485e30), (0x48d290, 0x48d36d), (0x48f690, 0x48f6b0), (0x492d20, 0x492d3b),
    (0x6589f9, 0x658abb), (0x658ccc, 0x658e04), (0x659710, 0x659740), (0x659d00, 0x659d53), (0x659d90, 0x659df5), (0x659e3e, 0x659e93),
    (0x65a9d0, 0x65aa00), (0x65aa20, 0x65aaa0), (0x65c388, 0x65c4c9), (0x65c5d5, 0x65c682), (0x65c7fe, 0x65c989), (0x65c9bd, 0x65cda5),
    (0x65d070, 0x65d1d8), (0x65d42e, 0x65d4b0), (0x65d773, 0x65d832), (0x65db25, 0x65dc00), (0x65e526, 0x65e559), (0x65e6d5, 0x65e73d),
    (0x65f5fe, 0x65f655), (0x65f66e, 0x65f6e1), (0x6616b0, 0x661780), (0x6617f0, 0x661820), (0x6697e3, 0x6698b4), (0x66bff7, 0x66c01b)]))
# (hoverlabel.py's ranges from 0x485d20 on: HoverLabel_Attach's)
KIND_VTABLES = (0x705980, 0x705a50, 0x705b20, 0x705950, 0x705a20, 0x705c58, 0x7059f0, 0x705ac0, 0x705c28, 0x705bf8, 0x705bc8, 0x705af0,
                0x705b98)   # (the factory's kinds 1-13)
SHOWN_KINDS = [1, 2, 6, 9, 13]
PANEL = (0xadae, 0xadaf, 0xada9, 0xada3, 0xada4, 0xada5, 0xadb0, 0xada8, 0xadac, 0xada2, 0xadad, 0xada7, 0xadaa, 0xada6, 0xadab, 0xadb1,
         0xadb2)
SHARED, SHAREDVT = 0x7c2118, RAM + 0x3e800
RECORD, ANALYSER = RAM + 0x3f000, RAM + 0x40000
BUFFERS = (0, 1, 2, 5, 15)
dialogbuild.DIALOGS['Dialog7cc444_Create'] = (0x7cc444, 0x7579c8, (0xada1,), ())   # (scripted()'s script, replaced below)


def children(r, w, ids, kinds, scripts, decoy_kinds=None):
    """the children `ids` (kind kinds.get(id), else any; script scripts.get(id), else none) shuffled among 0-2 decoys, one now and then
    missing"""
    ids = list(ids)
    if r.random() < 0.03:
        ids.remove(r.choice(ids))
    ids += [r.choice([0, 1, 0xada0, 0xadb4, r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(ids)
    for ident in ids:
        kind = kinds.get(ident) or (r.choice(decoy_kinds) if decoy_kinds else None)
        dialogbuild.child(r, w, ident, scripts.get(ident, (5,)), kind=kind)


def analyser(r, regions):
    handle = r.randrange(0, 8)
    regions[0x7cf83c] = struct.pack('<I', RECORD)
    regions[RECORD + 0x1da720] = struct.pack('<H', handle)
    regions[0x7cfb90 + 20 * handle] = struct.pack('<I', ANALYSER)
    for k in BUFFERS:
        buf = bytearray(r.bytes(0x250))
        cursor = r.choice([r.randrange(-1, 144), r.randrange(-1, 144), r.randrange(0, 13)])
        entries = r.choice([0, 1, 12, 13, 144, 144, 144, r.randrange(0, 145), r.getrandbits(32)])
        put(buf, 0x244, 'iIB', cursor, entries, r.choice([0, 1, 1, r.getrandbits(8)]))
        regions[ANALYSER + 0x1f100 + 0x250 * k] = buf


def case(r):
    r.deal(OPS)
    regions = dialogbuild.focused(r, 'Dialog7cc444_Create')
    regions[HEAP] = heap(0x80000, r.choice([0] * 40 + [r.randrange(1, 16)]))   # (a failure lands in the build: a fault in both)
    for vt in KIND_VTABLES:   # (the built windows' slots 5 and 7: game code the driver cannot run natively)
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
        regions[vt + 0x1c] = struct.pack('<I', recorder(29, 1))
    for vt in (0x6ff350, 0x6ff3b0):   # (those parts' layout and text: unported)
        regions[vt + 0xc] = struct.pack('<I', recorder(3, 2))
        regions[vt + 0x24] = struct.pack('<I', recorder(9, 1))
    window.notify_inner(regions[LOG])   # (their slot 3 told the alignment bytes, the font, the colour and the text: hashed)
    regions[SHARED] = put(put(bytearray(r.bytes(0x60)), 0, 'I', SHAREDVT), 0x10, 'hh', r.choice([0, 1, 2]), r.choice([0, 1, 2]))
    regions[SHAREDVT] = vtable({3: 2, 5: 1})
    panelparts.texts(r, regions, list(range(0xba, 0xc2)))
    analyser(r, regions)
    inner = []
    children(r, inner, PANEL, {k: r.choice([1] * 6 + SHOWN_KINDS) for k in PANEL}, {}, SHOWN_KINDS)
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    top = []
    children(r, top, (0xada1, 0xadb3), {0xada1: 1, 0xadb3: 1}, {0xada1: inner + [5]})
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    w += [5] * 4
    regions[0x7579c8] = struct.pack('<%dH' % len(w), *w)
    return Call(CALLS['Create'], [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

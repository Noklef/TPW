# The two-field entry dialog's builders (src/game/ui/dialog7cc5f4: 0x4bb070, 0x4ba200) against the originals. Create / CreateFrom:
# dialogbuild.py's scripted() surroundings (the dialog [0x7cc5f4] any: the builders do not look), the GUI root [0xfaa5d8] the script's
# parent WINDOW2 (Dialog_Create's frame made under it), the script (0x7585a8 / 0x7583e8) generated here: 0-2 plain children, then a kind 1
# window whose children are the fields 0x49 and 0x4b (kind 5, their own children the labels 0x4a / 0x4c among 0-2 decoys) and 0x4d
# (kind 1 mostly) among 0-2 decoys; the window kinds' slots 5 and 7 recorders (game code the driver cannot run natively) and slot 9 too
# (the text; 0x705a20's 0x6664fa unported); the GUI's registered windows (listrows.py's register()) with "hilight" among them or not (the
# fields' caret: CUT mostly, else none); the language texts 0xe9-0xee (panelparts.py's texts()); CreateFrom: the data's bytes random, the value any, the five
# key maps (keymap.py's) for KeyMaps_ResetAllAndSetFlag. The heap fails now and then (in the script's build: a fault in both).
# usage: dialog7cc5f4.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Create,CreateFrom)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialogbase  # noqa: E402
import dialogbuild  # noqa: E402
import keymap  # noqa: E402
import listrows  # noqa: E402
import panelparts  # noqa: E402
import window  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, put, recorder, set_return  # noqa: E402

CALLS = {'Create': 0x4bb070, 'CreateFrom': 0x4ba200}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
SCRIPTS = {'Create': 0x7585a8, 'CreateFrom': 0x7583e8}


def merged(ranges):
    out = []
    for lo, hi in sorted(set(ranges)):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


RANGES = (dialogbuild.ALL_RANGES + panelparts.ALL_RANGES + dialogbase.CREATE + [
    (0x4bb070, 0x4bb38f), (0x4ba200, 0x4ba52c), (0x486b70, 0x486b7d), (0x40c870, 0x40d06a), (0x492d20, 0x492d3b), (0x6662b7, 0x666308),
    (0x66656c, 0x66671d), (0x659d43, 0x659dbc), (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c01b), (0x65e6d5, 0x65e73d), (0x65c5d5, 0x65c682),
    (0x65f5b7, 0x65f655), (0x661525, 0x6615a3)])
B = Bench(merged(RANGES))
KIND_VTABLES = (0x705980, 0x705a50, 0x705b20, 0x705950, 0x705a20, 0x705c58, 0x7059f0, 0x705ac0, 0x705c28, 0x705bf8, 0x705bc8, 0x705af0,
                0x705b98)   # (the factory's kinds 1-13)
PART_SLOTS = {0x6ff380: {3: 2, 4: 2, 9: 1}, 0x6ff4d0: {4: 2, 9: 1}, 0x6ff530: {9: 1}}   # (0x4901d0, 0x491300, 0x48f830, 0x491ab0)
DATA, KEYS =RAM + 0x48000, RAM + 0x50000
for op in CALLS:
    dialogbuild.DIALOGS['Dialog7cc5f4_' + op] = (0x7cc5f4, SCRIPTS[op], (0x49,), ())   # (scripted()'s script, replaced below)


def children(r, w, ids, kinds, scripts):
    """the children `ids` (kind kinds[id]; script scripts.get(id), else none) shuffled among 0-2 decoys, one now and then missing"""
    ids = list(ids)
    if r.random() < 0.01:
        ids.remove(r.choice(ids))
    ids += [r.choice([0, 1, 0x48, 0x4e, r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(ids)
    for ident in ids:
        dialogbuild.child(r, w, ident, scripts.get(ident, (5,)), kind=kinds.get(ident))


def case(r):
    op = r.deal(OPS)
    regions = dialogbuild.scripted(r, 'Dialog7cc5f4_' + op)
    if r.random() < 0.95:
        regions[0xfaa5d8] = put(bytearray(regions[0xfaa5d8]), 0, 'I', window.WINDOW2)
    for vt in KIND_VTABLES:   # (the built windows' slots 5, 7 and 9: game code the driver cannot run natively)
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
        regions[vt + 0x1c] = struct.pack('<I', recorder(29, 1))
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    for vt, slots in PART_SLOTS.items():   # (those parts' unported slots)
        for k, n in slots.items():
            regions[vt + 4 * k] = struct.pack('<I', recorder(k, n))
    listrows.register(r, regions, 'hilight')
    set_return(regions[LOG], 8, r.choice([listrows.CUT] * 6 + [0]))   # (the caret: register()'s OBJ98 is not among scripted()'s regions)
    regions[HEAP] = heap(0x80000, r.choice([0] * 40 + [r.randrange(1, 16)]))   # (a failure lands in the build: a fault in both)
    panelparts.texts(r, regions, list(range(0xe9, 0xef)))
    labels = {}
    for field, label in ((0x49, 0x4a), (0x4b, 0x4c)):
        inner = []
        children(r, inner, (label,), {label: 1}, {})
        labels[field] = inner + [5]
    top = []
    children(r, top, (0x49, 0x4b, 0x4d), {0x49: 5, 0x4b: 5, 0x4d: r.choice([1, 1, 1, 2, 6, 9, 13])}, labels)
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    w += [5] * 4
    regions[SCRIPTS[op]] = struct.pack('<%dH' % len(w), *w)
    if op == 'Create':
        regions[0x7cc5fc] = r.bytes(4)
        return Call(CALLS[op], [r.getrandbits(32)], regions, ret='v')
    regions[DATA] = r.bytes(0x640)
    for k, g in enumerate(keymap.GLOBALS):
        keymap.keymap(r, KEYS + k * 0x2000 + 0x40, regions)
        regions[g] = struct.pack('<I', KEYS + k * 0x2000 + 0x40)
    regions[0x7c24d0] = r.bytes(0x10)
    return Call(CALLS[op], [r.getrandbits(32), DATA], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

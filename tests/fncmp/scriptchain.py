# The surroundings of the dialog opens that run several scripted builds in order (WindowScript_Build after WindowScript_Build, then
# Window_CreateChild), for their benches (dialog7ca150.py's Open, dialog7ca168.py, window7cc8bc.py), on dialogbuild.py's focused() or
# scripted() surroundings. Not a bench.
# radio_group(): a radio group (kind 8, 0x705ac0: nothing selected, +0x138 -1, from its constructor) written into a script, its buttons
# (kind 2, 0x705a50) in any order, one missing now and then (RadioGroup_Select then selects nothing), among 0-1 decoys.
# hover_part(): HoverLabel_Attach's shared text part [0x7c2118] random bytes, its vtable a recorder (slots 3 and 5; dialog7cc444.py's).
# kind_recorders(): the built windows' slots 5 and 7 (game code the driver cannot run natively) recorders (dialogbuild.py's windowat10()).
# advisor_env(): advisor.py's PostId surroundings for one id (its record its own mostly), merged into a case's regions and log; returns the
# masks of the stack message's unwritten bytes (advisor.py says why nothing reads them; the copy at +0x16d0 for an id other than 0x15f).
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import dialogbuild  # noqa: E402
from fncmp import LOG, RAM, Mask, put, recorder, set_return, vtable  # noqa: E402

SHARED, SHAREDVT = 0x7c2118, RAM + 0x3e880


def radio_group(r, w, ident, buttons, size=60):
    """the radio group `ident` (kind 8) with the buttons `buttons` (kind 2) as above, written into w"""
    ids = list(buttons)
    if r.random() < 0.05:
        ids.remove(r.choice(ids))
    ids += [r.choice([0, 1, r.getrandbits(32) | 0x100]) for _ in range(r.choice([0, 0, 1]))]
    r.shuffle(ids)
    inner = []
    for k in ids:
        dialogbuild.child(r, inner, k, kind=2 if k in buttons else None)
    dialogbuild.child(r, w, ident, inner + [5], size, 8)


def hover_part(r, regions):
    regions[SHARED] = put(put(bytearray(r.bytes(0x60)), 0, 'I', SHAREDVT), 0x10, 'hh', r.choice([0, 1, 2]), r.choice([0, 1, 2]))
    regions[SHAREDVT] = vtable({3: 2, 5: 1})


def kind_recorders(regions):
    for vt in dialogbuild.KIND_VTABLES:
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
        regions[vt + 0x1c] = struct.pack('<I', recorder(29, 1))


def advisor_env(r, regions, ident):
    """advisor.py's PostId surroundings for `ident` (above); returns the masks"""
    adv = advisor.queue(r, 'PostId')
    for k, v in adv.regions.items():
        if k != LOG:
            regions[k] = v
    table = bytearray(regions[0x76e300])
    if r.random() < 0.7:
        put(table, 0x38 * ident + 4, 'I', ident)
    regions[0x76e300] = table
    set_return(regions[LOG], 20, r.choice([0, 1, 5, 100, -1, r.getrandbits(32)]))
    extra = [0x16d0 + k for k in (2, 3, 0x17)] if ident != 0x15f else []
    return [Mask(m.region, sorted(set(m.offsets + extra)), m.reason) for m in adv.masks]

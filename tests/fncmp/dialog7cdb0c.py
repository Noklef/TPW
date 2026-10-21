# The dialog [0x7cdb0c]'s builder (src/game/ui/dialogs: 0x4c4fb0) against the original. Create: dialogbuild.py's scripted() surroundings
# (the dialog's global none mostly, the frame [0xf82884]'s window +4 WINDOW2 or now and then none, [0x7cdb10] random) with the script at
# 0x7596b0 generated here: 0-2 plain children, then the dialog (kind 1) whose children are 0x16007 (kind 1; its own children 0x16008-0x1600c,
# kind 1 mostly, only kinds without handlers of their own (SHOWN_KINDS below), among 0-2 decoys), 0x16003 and 0x16000 (any kind) and 0x16002 / 0x16004 / 0x16005 / 0x16001 (kind 2, the 0x705a50 kind
# whose pair +0x13c / +0x140 is set), in any order among 0-2 decoys, now and then one of them missing (a fault in both); the window kinds'
# slots 5 and 7 recorders (game code the driver cannot run natively), the 0x6ff350 text parts' slots 3 and 9 too (unported); the flag
# byte 0, 1 or any.
# usage: dialog7cdb0c.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Create)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialogbuild  # noqa: E402
import window705a50  # noqa: E402
from fncmp import Bench, Call, put, recorder  # noqa: E402

CALLS = {'Create': 0x4c4fb0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def merged(ranges):
    out = []
    for lo, hi in sorted(set(ranges)):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged(dialogbuild.ALL_RANGES + window705a50.RANGES + [(0x4c4fb0, 0x4c53e8), (0x65d9dd, 0x65da8d), (0x65e6d5, 0x65e705)]))
KIND_VTABLES = (0x705980, 0x705a50, 0x705b20, 0x705950, 0x705a20, 0x705c58, 0x7059f0, 0x705ac0, 0x705c28, 0x705bf8, 0x705bc8, 0x705af0,
                0x705b98)   # (the factory's kinds 1-13)
TEXTS = (0x16008, 0x16009, 0x1600a, 0x1600b, 0x1600c)
PAIRS = (0x16002, 0x16004, 0x16005, 0x16001)
SHOWN_KINDS = [1, 2, 6, 9, 13]   # (0x16007's children are told when it is hidden: not the kinds with handlers of their own (3's thumb,
# 4, 5, 8, 10, 11, 12: the driver cannot call those game addresses, and some are unported) nor 7 (its rows shown, game code unbenched here)
dialogbuild.DIALOGS['Dialog7cdb0c_Create'] = (0x7cdb0c, 0x7596b0, (0x16007,), (0x7cdb10,))


def children(r, w, ids, kinds, scripts, decoy_kinds=None):
    """the children `ids` (kind kinds.get(id), else any; script scripts.get(id), else none) shuffled among 0-2 decoys (of decoy_kinds, else
    any kind), one now and then missing"""
    ids = list(ids)
    if r.random() < 0.03:
        ids.remove(r.choice(ids))
    ids += [r.choice([0, 1, 0x16006, 0x1600d, r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(ids)
    for ident in ids:
        kind = kinds.get(ident) or (r.choice(decoy_kinds) if decoy_kinds else None)
        dialogbuild.child(r, w, ident, scripts.get(ident, (5,)), kind=kind)


def case(r):
    r.deal(OPS)
    regions = dialogbuild.scripted(r, 'Dialog7cdb0c_Create')
    if r.random() < 0.05:
        put(regions[dialogbuild.FRAME], 4, 'I', 0)
    for vt in KIND_VTABLES:   # (the built windows' slots 5 and 7: game code the driver cannot run natively)
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
        regions[vt + 0x1c] = struct.pack('<I', recorder(29, 1))
    regions[0x6ff350 + 0xc] = struct.pack('<I', recorder(3, 2))   # (the 0x6ff350 parts' layout and text: unported)
    regions[0x6ff350 + 0x24] = struct.pack('<I', recorder(9, 1))
    inner = []
    children(r, inner, TEXTS, {k: r.choice([1] * 6 + SHOWN_KINDS) for k in TEXTS}, {}, SHOWN_KINDS)
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    top = []
    children(r, top, (0x16007, 0x16003, 0x16000) + PAIRS, {0x16007: 1, **{k: 2 for k in PAIRS}}, {0x16007: inner + [5]})
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    w += [5] * 4
    regions[0x7596b0] = struct.pack('<%dH' % len(w), *w)
    flag = r.choice([0, 0, 0, 1, r.getrandbits(32)])
    return Call(CALLS['Create'], [flag], regions, ret='v')


if __name__ == '__main__':
    B.main(case)

# The progress dialog's functions (src/game/ui/dialog7cca94) against the originals.
# Show / ShowText (0x4c23f0, 0x4c25b0), with the dialog [0x7cca94] up (a third of the time): a window whose children (+0x5c, then +0x54)
# are the text 0x2241 (its vtable's slot 9 a recorder; now and then missing, a fault in both) among 0-2 decoys. Without it:
# dialogbuild.py's scripted() surroundings, its script at 0x759548 with the text 0x2241, the root [0xfaa5d8] the script's parent WINDOW2
# (now and then another window of the tree), and the bar's mesh as panelparts.py's surroundings build it (global_heap(), its vtable
# 0x6feeb8's slots 3 / 5 recorders; slot 3's event a stack pointer, unhashed), the window kinds' vtables' slots 5 and 7 recorders
# (game code the driver cannot run natively). ShowText: the language text of the index as panelparts.py's texts() makes it, [0x7cca9c] that index
# (when the dialog is up, half the time) or another. Show: the text pointer any.
# SetProgress (0x4c2600): the dialog up (its slot 7 a recorder) or none, the percent 0-100 mostly, or any value.
# OnMessage (0x4c23c0): a window whose default handler +0x118 is a cdecl recorder, the message 0x14 or another, the dialog any.
# usage: dialog7cca94.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Show,ShowText,SetProgress,OnMessage)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import animatormesh  # noqa: E402
import dialogbuild  # noqa: E402
import panelparts  # noqa: E402
import window  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, blob_bytes, global_heap, log_region, put, recorder, vtable  # noqa: E402

CALLS = {'Show': 0x4c23f0, 'ShowText': 0x4c25b0, 'SetProgress': 0x4c2600, 'OnMessage': 0x4c23c0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def merged(ranges):
    out = []
    for lo, hi in sorted(set(ranges)):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged(dialogbuild.ALL_RANGES + panelparts.ALL_RANGES + [
    (0x4c23c0, 0x4c2630), (0x65da8d, 0x65db25), (0x65c5d5, 0x65c672), (0x65f5b7, 0x65f643), (0x65f66e, 0x65f697), (0x661525, 0x6615a3)]),
    hooks=animatormesh.HOOKS)
UP, UPKIDS, UPVT = RAM + 0x3c000, RAM + 0x3c400, RAM + 0x3cc00
TEXT = 0x2241
KIND_VTABLES = (0x705980, 0x705a50, 0x705b20, 0x705950, 0x705a20, 0x705c58, 0x7059f0, 0x705ac0, 0x705c28, 0x705bf8, 0x705bc8, 0x705af0,
                0x705b98)   # (the factory's kinds 1-13)
dialogbuild.DIALOGS['Dialog7cca94_Show'] = (0x7cca94, 0x759548, (TEXT,), ())


def up(r, regions):
    """the dialog up: UP and its children, the text 0x2241 among decoys"""
    d = put(bytearray(r.bytes(0x140)), 0, 'I', UPVT)
    regions[UPVT] = vtable({7: 1, 9: 1})
    ids = ([TEXT] if r.random() < 0.95 else []) + [r.choice([0, TEXT + 1, 0x2240, r.getrandbits(32)]) for _ in range(r.choice([0, 0, 1, 2]))]
    r.shuffle(ids)
    for i, ident in enumerate(ids):
        k = put(bytearray(r.bytes(0x140)), 0, 'I', UPVT)
        put(k, 0x8c, 'I', ident)
        put(k, 0x5c, 'I', 0)
        put(k, 0x54, 'I', UPKIDS + 0x140 * (i + 1) if i + 1 < len(ids) else 0)
        regions[UPKIDS + 0x140 * i] = k
    put(d, 0x5c, 'I', UPKIDS if ids else 0)
    regions[UP] = d
    regions[0x7cca94] = struct.pack('<I', UP)


def built(r):
    """the surroundings of the dialog's build (above)"""
    regions = dialogbuild.scripted(r, 'Dialog7cca94_Show')
    regions[0x7cca94] = struct.pack('<I', 0)
    if r.random() < 0.95:
        regions[0xfaa5d8] = put(bytearray(regions[0xfaa5d8]), 0, 'I', window.WINDOW2)
    regions.update(global_heap())
    regions[panelparts.MESH_VTABLE + 0xc] = struct.pack('<I', recorder(26, 2))
    blob_bytes(regions[LOG], 26, 1, 0)   # (its event a stack pointer, logged as a hash of none of its bytes, as listrows.py's)
    regions[panelparts.MESH_VTABLE + 0x14] = struct.pack('<I', recorder(27, 1))
    for vt in KIND_VTABLES:   # (the built dialog's slots 5 and 7: game code the driver cannot run natively)
        regions[vt + 0x14] = struct.pack('<I', recorder(28, 1))
        regions[vt + 0x1c] = struct.pack('<I', recorder(29, 1))
    return regions


def case(r):
    op = r.deal(OPS)
    if op in ('Show', 'ShowText'):
        shown = r.random() < 0.35
        regions = {LOG: log_region()} if shown else built(r)
        if shown:
            up(r, regions)
        if op == 'Show':
            return Call(CALLS[op], [r.choice([0, RAM + 0x3d000, r.getrandbits(32)])], regions, ret='v')
        index = r.choice([r.randrange(0, 0x40), r.randrange(0, 0x400)])
        regions.setdefault(0x78b530, b'')   # (the language object: random bytes where the dialog is up, texts() sets what it reads)
        panelparts.texts(r, regions, [index])
        same = shown and r.random() < 0.5
        regions[0x7cca9c] = struct.pack('<I', index if same else r.choice([index ^ 1, 0, r.getrandbits(32)]))
        return Call(CALLS[op], [index], regions, ret='v')
    regions = {LOG: log_region()}
    if op == 'SetProgress':
        if r.random() < 0.8:
            up(r, regions)
        else:
            regions[0x7cca94] = struct.pack('<I', 0)
        percent = r.choice([r.randrange(0, 101)] * 4 + [r.randrange(-200, 0), 0x200000, 0x1fffff, 0x7fffffff, 0x80000000, r.getrandbits(32)])
        return Call(CALLS[op], [percent & 0xffffffff], regions, ret='v')
    w = put(bytearray(r.bytes(0x140)), 0x118, 'I', recorder(5, 4, cdecl=True))
    regions[RAM] = w
    regions[0x7cca94] = r.bytes(4)
    message = r.choice([0x14, 0x14, 0x13, 0x15, 0x17, 4, r.getrandbits(32)])
    return Call(CALLS[op], [RAM, message, r.getrandbits(32), r.getrandbits(32)], regions)


if __name__ == '__main__':
    B.main(case)

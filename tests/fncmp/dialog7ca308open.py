# Dialog7ca308_Open (src/game/ui/dialog7ca308, 0x49bdd0) against the original. Dialog7ca308_FillList 17 (cdecl), Charset_FormatWide 18 (cdecl; its number
# argument on the stack logged by its 20 bytes), RadioGroup_Select 19 and HoverLabel_Attach 21 (cdecl) recorders: their own benches have them.
# Open on an open window: its list 0x248e and group 0x248f among decoys (windows of random bytes, their ids and links set; list or group missing now and then),
# the kinds 0-4, past them, the one it has, -1.
# A new one: dialog7ca150.py's list_dialog() surroundings with the page below at the script 0x751fa8: 0-2 plain children, then the dialog with the children 0x2496,
# the list 0x248e (2 columns, its buttons, the group 0x248f among decoys), the bar 0x2481 (0x2483, 0x2482), the figures 0x2485 (eight parts), each of the
# three parts missing now and then (the script empty: nothing built); the rings of [0x7cf83c]'s table's +0x1da720 (empty, positive, negative with the flag or not, count 0 or negative); advisor.py's PostId for 0xbd.
# usage: dialog7ca308open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import advisor  # noqa: E402
import dialog7ca150  # noqa: E402
import dialogbuild  # noqa: E402
import panelparts  # noqa: E402
import scriptchain  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, _recorder, blob_bytes, heap, log_region, put, recorder, set_return  # noqa: E402

OP = 'Dialog7ca308_Open'
dialog7ca150.TABS[OP] = (0x751fa8, 2, 0x7523e4, 0x90)
RANGES = sorted(set(dialog7ca150.ALL_RANGES + dialog7ca150.OPEN_RANGES + [(0x49bbb0, 0x49bc80), (0x49bdd0, 0x49c820), (0x49c870, 0x49c875), (0x49c880, 0x49c98c), (0x65d7bc, 0x65d840),
                                                                          (0x519510, 0x519550), (0x5195d0, 0x519610), (0x6ad810, 0x6ad820), (0x660000, 0x660001)]))
B = Bench(dialog7ca150.merged(RANGES),
          hooks={**panelparts.animatormesh.HOOKS, 0x67a7d0: _cdecl_log, 0x49b5b0: _recorder(17, 2, cdecl=True), 0x6acd60: _recorder(18, 5, cdecl=True),
                 0x66a295: _recorder(19, 1), 0x485d20: _recorder(21, 3, cdecl=True)})
WIN, KIDS = RAM + 0x60000, RAM + 0x60200
RINGS, SOURCE = 0x33100000, RAM + 0x61000
RING_LOW = 0x1f5a0
TEXTS = list(range(0x8b, 0x90)) + [0x92, 0x165, 0x166, 0x167, 0x169, 0x90, 0x91]


def kinds(r, state):
    return r.choice([0, 1, 2, 3, 4] * 3 + [5, state, state, 0xffffffff, 0xffffffff, r.getrandbits(32)])


def linked(r, regions, at, ids, size=0x100):
    """windows of random bytes at `at`, one after the other, with those ids; returns the address of each"""
    out = {}
    for k, ident in enumerate(ids):
        w = bytearray(r.bytes(size))
        put(w, 0x8c, 'I', ident)
        put(w, 0x54, 'I', at + size * (k + 1) if k + 1 < len(ids) else 0)
        put(w, 0x5c, 'I', 0)
        regions[at + size * k] = w
        out[ident] = at + size * k
    return out


def open_window(r):
    """the window [0x7ca308] already open"""
    log = log_region()
    regions = {LOG: log}
    state = r.choice([0, 1, 2, 3, 4, 4, r.getrandbits(32)])
    kind = kinds(r, state)
    if kind in (state, 0xffffffff):   # (the new path: not this case's)
        kind = (state + 1) & 0xffffffff
    regions[0x7ca2f0] = put(put(bytearray(r.bytes(0x20)), 0x18, 'I', WIN), 0x1c, 'I', state)
    ids = [0x248e] + [r.getrandbits(32) | 0x100 for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(ids)
    kids = linked(r, regions, KIDS, ids)
    inner = [0x248f] + [r.getrandbits(32) | 0x100 for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(inner)
    subs = linked(r, regions, KIDS + 0x400, inner)
    put(regions[kids[0x248e]], 0x5c, 'I', KIDS + 0x400)
    regions[WIN] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS)
    regions[0x7cb2a0] = r.bytes(4)
    regions[0x7c24c8] = struct.pack('<2I', 0, 0)
    regions[0x78b530] = bytearray()
    panelparts.texts(r, regions, TEXTS)
    return Call(0x49bdd0, [kind], regions, ret='v')


def child_list(r, ident=0x248e, columns=2, group=0x248f, buttons=(0x2490, 0x2491, 0x2492, 0x2493, 0x2494)):
    inner = [0xb, columns]
    x = r.randrange(0, 10)
    for _ in range(columns):
        width = r.randrange(5, 40)
        inner += [x, x + width]
        x += width
    left, top = r.randrange(0, 20), r.randrange(0, 30)
    inner += [0xa, left, top, left + r.randrange(20, 200), top + r.choice([r.randrange(0, 20)] * 6 + [0, 1, r.randrange(20, 170)])]
    kids = [str(k) for k in range(columns)] + (['group'] if r.random() < 0.9 else []) + ['decoy'] * r.choice([0, 0, 1])
    r.shuffle(kids)
    for k in kids:
        if k == 'decoy':
            dialogbuild.child(r, inner, r.choice([0, 0x400, r.getrandbits(32) | 0x100]))
        elif k == 'group':
            scriptchain.radio_group(r, inner, group, buttons)
        else:
            inner += [0xc, int(k)]
            dialogbuild.small_rect(r, inner)
            inner.append(5)
    w = []
    dialogbuild.child(r, w, ident, inner + [5], 120, 7)
    return w


def group_of(r, w, ident, ids):
    inner = []
    ids = list(ids) + [r.choice([0, 1, r.getrandbits(32) | 0x100]) for _ in range(r.choice([0, 0, 1]))]
    r.shuffle(ids)
    for k in ids:
        dialogbuild.child(r, inner, k, kind=1)
    dialogbuild.child(r, w, ident, inner + [5], 160, 1)


def script(r):
    top = []
    kids = ['line', 'bar', 'figures', 'list', 'label'] + ['decoy'] * r.choice([0, 0, 1])
    for k in ('bar', 'figures', 'list'):
        if r.random() < 0.05:
            kids.remove(k)
    r.shuffle(kids)
    for k in kids:
        if k == 'line':
            dialogbuild.child(r, top, 0x2496, kind=1)
        elif k == 'label':
            dialogbuild.child(r, top, 0x2480, kind=1)
        elif k == 'list':
            top += child_list(r)
        elif k == 'bar':
            group_of(r, top, 0x2481, (0x2482, 0x2483))
        elif k == 'figures':
            group_of(r, top, 0x2485, (0x2486, 0x2487, 0x2488, 0x2489, 0x248a, 0x248b, 0x248c, 0x248d))
        else:
            dialogbuild.child(r, top, r.choice([0, 1, r.getrandbits(32) | 0x100]))
    w = []
    for _ in range(r.choice([0, 0, 1, 2])):
        dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]))
    dialogbuild.child(r, w, r.choice([0, 1, r.getrandbits(32)]), top + [5], 160, 1)
    return w + [5] * 4


def rings(r):
    block = bytearray(r.bytes(0x1fee0 - RING_LOW))
    for base in (0x1fc94, 0x1f7f4, 0x1f5a4):
        count = r.choice([0, 0, 1, 2, 5, 0x90, r.randrange(-3, 0)])
        position = r.choice([r.randrange(0, 0x90), r.randrange(-max(count, 1), 0)] + ([-1] if count > 0 else [0]))
        put(block, base - RING_LOW + 0x240, 'iiB', position, count, r.choice([0, 0, 1, r.getrandbits(8)]))
    return block


def new_window(r):
    state = r.choice([0, 1, 2, 3, 4, 4, r.getrandbits(32)])
    kind = kinds(r, state)
    window = r.choice([0, 0, 0, WIN])
    if window and kind not in (state, 0xffffffff):
        kind = r.choice([state, 0xffffffff])
    regions = dialog7ca150.list_dialog(r, OP, lambda r, regions: TEXTS)
    w = script(r) if r.random() < 0.97 else [5] * 4
    regions[0x751fa8] = struct.pack('<%dH' % len(w), *w)
    log = regions[LOG]
    regions[HEAP] = heap(0x28000, r.choice([0] * 40 + [r.randrange(1, 16)]))
    regions[0x7ca2f0] = put(put(bytearray(r.bytes(0x20)), 0x18, 'I', window), 0x1c, 'I', state)
    if window:
        regions[WIN] = r.bytes(0x20)
    masks = scriptchain.advisor_env(r, regions, 0xbd)
    # [0x7cf83c] (advisor.py's PostId): the words that pick the table's entries, which hold the rings and the money's source
    world = bytearray(r.bytes(8))
    a, b = r.sample(range(0x100), 2)
    a, b = a + 0x100, b + 0x100   # (the advisor's own word below them)
    put(world, 0, 'H', a)
    put(world, 6, 'H', b)
    regions[advisor.WORLD + 4] = world
    regions[0x7cfb90 + 20 * a] = struct.pack('<I', RINGS)
    regions[0x7cfb90 + 20 * b] = struct.pack('<I', SOURCE)
    money = r.choice([0, 1, 5000, -1, -5000, -0x80000000, r.randrange(-1 << 31, 1 << 31)])
    regions[SOURCE] = put(bytearray(r.bytes(0x10)), 0xc, 'i', money)
    regions[RINGS + RING_LOW] = rings(r)
    regions[0x7523e0] = struct.pack('<I', r.choice([7] * 6 + list(range(13))))
    regions[0x7523dc] = struct.pack('<I', 7)
    regions[0x7523e4] = struct.pack('<I', 1)   # (the new list's column 0 ascending: any other changes it, the dialog's handler 0x49b650 told, unported)
    blob_bytes(log, 18, 4, 0xfffff)
    for vt in dialogbuild.KIND_VTABLES:   # (the money line's slot 9)
        regions[vt + 0x24] = struct.pack('<I', recorder(30, 1))
    regions[0x7cb2a0] = r.bytes(4)
    return Call(0x49bdd0, [kind], regions, ret='v', masks=masks)


def case(r):
    return r.deal([new_window, new_window, new_window, open_window])(r)


if __name__ == '__main__':
    B.main(case)

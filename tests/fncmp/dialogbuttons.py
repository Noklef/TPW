# Two dialogs' buttons set (src/game/ui/dialogs: 0x4bd400, 0x4bd450, 0x49ab80) against the originals.
# SetC3128 / SetC3129: the window [0x7cc8bc] (now and then 0) with its child 0xc3123, that one's 0xc3124 and that one's 0xc3128 / 0xc3129
# among one or two others (each link now and then missing: a fault in both); the value 0 / 1 / 2 / any, the pushed byte's upper bytes random.
# ClearButtons: the window [0x7ca2e8] (very rarely 0: a fault in both) with its child 0x12272 holding the eight buttons 0x12277-0x1227e
# shuffled among a few others (now and then one missing: a fault in both once it is reached); the mode 3 half the time, else any.
# The buttons are 0x705a50 windows as in window705a50.py: their +0x98 objects one recorder (k 3), their +0x9c objects each its own (k 22 +
# i), masked as radiogroup.py does by the look each will have; SetC3129's button has a vtable of recorders (slot 5 of one argument).
# usage: dialogbuttons.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetC3128,SetC3129,ClearButtons)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, blob_bytes, log_region, put, recorder, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from radiogroup import masks  # noqa: E402
from window import queues  # noqa: E402
from window705a50 import EVENT_MASK, RANGES as BUTTON_RANGES, window  # noqa: E402

CALLS = {'SetC3128': 0x4bd400, 'SetC3129': 0x4bd450, 'ClearButtons': 0x49ab80}
RANGES = [(0x4bd400, 0x4bd443), (0x4bd450, 0x4bd492), (0x49ab80, 0x49ac54), (0x65d7bc, 0x65d832)] + BUTTON_RANGES
B = Bench(RANGES)
TOP, LINKS, OBJ98, VT98, VTC, OBJS, KIDS = RAM + 0x3000, RAM + 0x3200, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x1000, RAM + 0x4000
# (the links: 3 windows of 0x180 from 0x3200, 3 from 0x3800; the buttons up to 11 from 0x4000, their objects 0x40 apart from 0x1000)
KID = 0x180
CLEARED = {3: [0x12277, 0x1227b], None: [0x12279, 0x12278, 0x1227c, 0x1227d, 0x1227a, 0x1227e]}


def plain(r, regions, at, ident, sibling):
    w = put(bytearray(r.bytes(0x160)), 0x8c, 'I', ident)
    put(w, 0x54, 'I', sibling)
    regions[at] = w
    return w


def children(r, regions, parent, at, ids):
    """windows of `ids` at at, at + 0x180, ... linked as the parent's children (no children of their own); returns their addresses"""
    addrs = [at + KID * i for i in range(len(ids))]
    for i, ident in enumerate(ids):
        put(plain(r, regions, addrs[i], ident, addrs[i + 1] if i + 1 < len(ids) else 0), 0x5c, 'I', 0)
    put(parent, 0x5c, 'I', addrs[0] if ids else 0)
    return addrs


def buttons(r, regions, log, parent, ids):
    """0x705a50 windows of `ids` linked as the parent's children; returns their (id, bytes) in order"""
    kids = []
    for i, ident in enumerate(ids):
        w = window(r)
        put(w, 0x8c, 'I', ident)
        put(w, 0x54, 'I', KIDS + KID * (i + 1) if i + 1 < len(ids) else 0)
        put(w, 0x98, 'I', OBJ98 if r.random() < 0.8 else 0)
        put(w, 0x9c, 'I', OBJS + 0x40 * i if r.random() < 0.8 else 0)
        vt = vtable({}, size=8)
        put(vt, 0xc, 'I', recorder(22 + i, 2))
        regions[OBJS + 0x40 * i] = put(bytearray(r.bytes(0x20)), 0, 'I', OBJS + 0x40 * i + 0x20) + vt
        regions[KIDS + KID * i] = w
        kids.append((ident, w))
    put(parent, 0x5c, 'I', KIDS if ids else 0)
    regions[VT98] = vtable({3: 2}, size=8)
    regions[OBJ98] = put(bytearray(r.bytes(0x20)), 0, 'I', VT98)
    blob_bytes(log, 3, 1, EVENT_MASK)
    queues(r, regions)
    regions[0xfaa5d4] = put(bytearray(4), 0, 'I', r.choice([0, 1, 0x80000000]))
    return kids


def among(r, ident, others, missing=0.04):
    """ident among one or two of others, shuffled (now and then left out)"""
    ids = r.sample(others, r.choice([0, 1, 2])) + ([] if r.random() < missing else [ident])
    r.shuffle(ids)
    return ids


def first(kids, ident):
    return next((i for i, (k, _) in enumerate(kids) if k == ident), None)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    top = bytearray(r.bytes(0x160))
    regions[TOP] = top
    if op == 'ClearButtons':
        regions[0x7ca2e8] = put(bytearray(4), 0, 'I', TOP if r.random() < 0.98 else 0)
        addrs = children(r, regions, top, LINKS, among(r, 0x12272, [0x12271, 0x12273, 0x1227f]))
        mode = r.choice([3, 3, 3, 0, 1, 2, 4, r.getrandbits(32)])
        ids = list(range(0x12277, 0x1227f)) + r.sample([0x12276, 0x1227f, 0x12272, 5], r.choice([0, 1, 2]))
        if r.random() < 0.06:
            ids.remove(r.choice(ids))
        r.shuffle(ids)
        panel = next((a for a in addrs if int.from_bytes(regions[a][0x8c:0x90], 'little') == 0x12272), None)
        kids = buttons(r, regions, log, regions[panel] if panel else bytearray(0x160), ids)
        states = {}
        for ident in CLEARED[3 if mode == 3 else None]:
            i = first(kids, ident)
            if i is None:
                break
            states[i] = 0
        masks(log, kids, states)
        return Call(CALLS[op], [mode], regions, ret='v')
    regions[0x7cc8bc] = put(bytearray(4), 0, 'I', TOP if r.random() < 0.9 else 0)
    target = 0xc3128 if op == 'SetC3128' else 0xc3129
    a = children(r, regions, top, LINKS, among(r, 0xc3123, [0xc3124, 0xc3128, 1]))
    link = next((x for x in a if int.from_bytes(regions[x][0x8c:0x90], 'little') == 0xc3123), None)
    b = children(r, regions, regions[link] if link else bytearray(0x160), LINKS + 0x600, among(r, 0xc3124, [0xc3123, 0xc3129, 2]))
    link = next((x for x in b if int.from_bytes(regions[x][0x8c:0x90], 'little') == 0xc3124), None)
    kids = buttons(r, regions, log, regions[link] if link else bytearray(0x160), among(r, target, [0xc3128, 0xc3129, 0xc3127], 0.03))
    i = first(kids, target)
    value = r.choice([0, 1, 1, 2, r.getrandbits(8)])
    if op == 'SetC3128':
        masks(log, kids, {} if i is None else {i: 1 if value == 1 else 0})
    else:
        masks(log, kids, {})
        if i is not None:
            put(kids[i][1], 0, 'I', VTC)
            regions[VTC] = vtable({5: 1}, size=8)
    return Call(CALLS[op], [value | r.choice([0, r.getrandbits(24)]) << 8], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

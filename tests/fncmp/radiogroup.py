# The GUI library's 0x705ac0 window, a group whose selected child's state it sets (src/game/ui/radiogroup: 0x66a1b0, 0x66a295), against the
# originals. A group with 0-5 children (0x705a50 windows as in window705a50.py, ids from a few, repeats too), the selected id one of
# theirs, -1 or another. SetSelectedState: a state 0 / 1 / any (the pushed register's upper bytes random). Select: an id of the children's,
# the selected one, -1 or another. The children's +0x98 objects one recorder (k 3), their +0x9c objects each its own (k 22 + i), so that
# each logs its event masked by the look that child will have (window705a50.py says what is masked and why).
# usage: radiogroup.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetSelectedState,Select)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, blob_bytes, log_region, put, recorder, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from window import queues  # noqa: E402
from window705a50 import EVENT_MASK, INNER, NOTIFY_MASK, RANGES as BUTTON_RANGES, look, window  # noqa: E402

CALLS = {'SetSelectedState': 0x66a1b0, 'Select': 0x66a295}
RANGES = [(0x66a1b0, 0x66a1e8), (0x66a295, 0x66a31c), (0x65d7bc, 0x65d832)] + BUTTON_RANGES
B = Bench(RANGES)
GROUP, KIDS, OBJ98, VT98, OBJS = RAM, RAM + 0x1000, RAM + 0x200, RAM + 0x280, RAM + 0x400
KID = 0x180
IDS = [1, 2, 3, 0x19, 0x1f]


def group_env(r, regions, log, group_at=GROUP, group=None):
    """a group (at group_at, or the given bytes) with 0-5 children; returns the group's bytes and the children's (id, bytes) in order"""
    g = group if group is not None else bytearray(r.bytes(0x160))
    n = r.choice([0, 1, 2, 3, 4, 5])
    kids = []
    for i in range(n):
        w = window(r)
        ident = r.choice(IDS)
        put(w, 0x8c, 'I', ident)
        put(w, 0x54, 'I', KIDS + KID * (i + 1) if i + 1 < n else 0)
        put(w, 0x98, 'I', OBJ98 if r.random() < 0.8 else 0)
        put(w, 0x9c, 'I', OBJS + 0x40 * i if r.random() < 0.8 else 0)
        vt = vtable({}, size=8)
        put(vt, 0xc, 'I', recorder(22 + i, 2))
        regions[OBJS + 0x40 * i] = put(bytearray(r.bytes(0x20)), 0, 'I', OBJS + 0x40 * i + 0x20) + vt
        regions[KIDS + KID * i] = w
        kids.append((ident, w))
    put(g, 0x5c, 'I', KIDS if n else 0)
    put(g, 0x138, 'I', r.choice([k[0] for k in kids] + [0xffffffff, 7, r.getrandbits(32)]))
    regions[group_at] = g
    regions[VT98] = vtable({3: 2}, size=8)
    regions[OBJ98] = put(bytearray(r.bytes(0x20)), 0, 'I', VT98)
    blob_bytes(log, 3, 1, EVENT_MASK)
    queues(r, regions)
    regions[0xfaa5d4] = put(bytearray(4), 0, 'I', r.choice([0, 1, 0x80000000]))
    return g, kids


def first(kids, ident):
    for i, (k, _) in enumerate(kids):
        if k == ident:
            return i
    return None


def masks(log, kids, states):
    """each child's +0x9c recorder masked by the look it has after the state states[i] (when given) is set"""
    for i, (_, w) in enumerate(kids):
        w2 = bytearray(w)
        if i in states:
            w2[0x138] = states[i]
        blob_bytes(log, 22 + i, 1, NOTIFY_MASK, None if look(w2) == 1 else INNER)


def select_states(kids, selected, ident):
    """the states Select(ident) sets: the selected child 0, the new one 1"""
    states = {}
    if ident == selected:
        return states
    old = first(kids, selected)
    if ident == 0xffffffff:
        if old is not None:
            states[old] = 0
        return states
    new = first(kids, ident)
    if new is None:
        return states
    if old is not None:
        states[old] = 0
    states[new] = 1
    return states


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    g, kids = group_env(r, regions, log)
    selected = int.from_bytes(g[0x138:0x13c], 'little')
    if op == 'SetSelectedState':
        state = r.choice([0, 1, 1, r.getrandbits(8)])
        i = first(kids, selected)
        masks(log, kids, {} if i is None else {i: state})
        return Call(CALLS[op], [GROUP, state | r.choice([0, r.getrandbits(24)]) << 8], regions, conv='t', ret='v')
    ident = r.choice([k[0] for k in kids] * 2 + [selected, 0xffffffff, 0xffffffff, 7])
    masks(log, kids, select_states(kids, selected, ident))
    return Call(CALLS[op], [GROUP, ident], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

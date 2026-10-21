# The GUI library's 0x705ac0 window, a group whose selected child's state it sets (src/game/ui/radiogroup: 0x66a1b0, 0x66a295), against the
# originals. A group with 0-5 children (0x705a50 windows as in window705a50.py, ids from a few, repeats too), the selected id one of
# theirs, -1 or another. SetSelectedState: a state 0 / 1 / any (the pushed register's upper bytes random). Select: an id of the children's,
# the selected one, -1 or another. The children's +0x98 objects one recorder (k 3), their +0x9c objects each its own (k 22 + i), so that
# each logs its event masked by the look that child will have (window705a50.py says what is masked and why).
# Proc (0x66a31c): the group's flags +0x48 bit 4 set or not, its parent +0x60 a window (byte +4 clear or not) or none, the queues; the
# children's vtables mostly 0x705a50 (slot 3, the kind, 2) or of other kinds; the message 0x15, 0x100 or a few others (the top bit too),
# a a child's id, the selected one, -1 or another, b 1, 0 or any. The children's events masked by the states the message sets.
# usage: radiogroup.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetSelectedState,Select,Proc)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, blob_bytes, log_region, put, recorder, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from window import RANGES as WINDOW_RANGES, queues  # noqa: E402
from window705a50 import EVENT_MASK, INNER, NOTIFY_MASK, RANGES as BUTTON_RANGES, look, window  # noqa: E402

CALLS = {'SetSelectedState': 0x66a1b0, 'Select': 0x66a295, 'Proc': 0x66a31c}
RANGES = sorted(set([(0x66a1b0, 0x66a1e8), (0x66a295, 0x66a31c), (0x65d7bc, 0x65d832), (0x66a31c, 0x66a489), (0x65f6d1, 0x65fba8), (0x680050, 0x68005b),
                     (0x66a281, 0x66a295), (0x65d547, 0x65d56b), (0x668aad, 0x668abd), (0x65ad33, 0x65ad43), (0x66a271, 0x66a281),
                     (0x65c2c8, 0x65c2d8)] + BUTTON_RANGES + WINDOW_RANGES))
B = Bench(RANGES)
GROUP, KIDS, OBJ98, VT98, OBJS, PARENT = RAM, RAM + 0x1000, RAM + 0x200, RAM + 0x280, RAM + 0x400, RAM + 0x2000
KINDS = [0x705a50] * 4 + [0x705950, 0x705ac0, 0x705980]   # (slot 3: 2, 4, 8, 1)
KID = 0x180
IDS = [1, 2, 3, 0x19, 0x1f]


def group_env(r, regions, log, group=None, ids=IDS):
    """a group at GROUP (its bytes random, or the given ones) with 0-5 children of `ids`; returns the group's bytes and the children's (id,
    bytes) in order"""
    g = group if group is not None else bytearray(r.bytes(0x160))
    n = r.choice([0, 1, 2, 3, 4, 5])
    kids = []
    for i in range(n):
        w = window(r)
        ident = r.choice(ids)
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
    regions[GROUP] = g
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
    if op == 'Proc':
        return proc_case(r, regions, log, g, kids, selected, ident)
    masks(log, kids, select_states(kids, selected, ident))
    return Call(CALLS[op], [GROUP, ident], regions, conv='t', ret='w')


def proc_case(r, regions, log, g, kids, selected, ident):
    g[0x48] = (g[0x48] & ~0x10) | r.choice([0, 0x10])
    put(g, 0x60, 'I', r.choice([PARENT, PARENT, 0]))
    regions[PARENT] = put(bytearray(r.bytes(0x70)), 4, 'B', r.choice([0, 0, 1]))
    kinds = []
    for _, w in kids:
        kinds.append(r.choice(KINDS))
        put(w, 0, 'I', kinds[-1])
    message = r.choice([0x15] * 4 + [0x100] * 6 + [0x16, 0x101, 0x80000015, 0x80000100])
    b = r.choice([1, 1, 0, r.getrandbits(32)])
    states = {}
    if message == 0x15 and g[0x48] & 0x10 and selected == 0xffffffff and kids and kinds[0] == 0x705a50:
        states = select_states(kids, selected, kids[0][0])
    elif message == 0x100 and first(kids, ident) is not None:
        if b == 1:
            states = select_states(kids, selected, ident)
        elif not g[0x48] & 0x10:
            states = select_states(kids, selected, 0xffffffff)
        else:
            states = select_states(kids, selected, ident)
            states[first(kids, ident)] = 1
    masks(log, kids, states)
    return Call(CALLS['Proc'], [GROUP, message, ident, b], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

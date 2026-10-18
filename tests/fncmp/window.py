# The GUI library's window child search (src/game/ui/window, 0x65d7d0) and the 0x7ca1c8 dialog's check buttons (src/game/ui/dialog7ca1c8,
# 0x498090 / 0x498a50) against the originals. FindChild: a window with 0-6 children (ids from a few, repeats and misses) searched for one id.
# SetChecks / SetChecksIfUp: the dialog's child 0x18 holds the nine buttons 0x18-0x21 shuffled among a few others (now and then one missing:
# a fault in both), each with a recorder vtable whose slot 5 logs its bool (the low byte: the original pushes 0x20's with ECX's upper half);
# [0x7ca1d0] +0x50 and [0x7ca1c8] / [0x7ca1cc] (the IfUp guard) set or 0.
# usage: window.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FindChild,SetChecks,SetChecksIfUp)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, Bench, Call, RAM, log_region, put, vtable  # noqa: E402

CALLS = {'FindChild': 0x65d7d0, 'SetChecks': 0x498090, 'SetChecksIfUp': 0x498a50}
B = Bench([(0x498090, 0x4981e0), (0x498a50, 0x498a80), (0x65d7bc, 0x65d832)])
WINDOW, NODES, VTABLE, MISC = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x1100
NODE = 0x90
BUTTONS = list(range(0x18, 0x22))


def node(r, ident, nxt, child=0):
    buf = bytearray(r.bytes(NODE))
    put(buf, 0, 'I', VTABLE)
    put(buf, 0x54, 'I', nxt)
    put(buf, 0x5c, 'I', child)
    put(buf, 0x8c, 'I', ident)
    return buf


def chain(r, regions, ids, first, child_of=None):
    """nodes for ids from `first` on (each NODE bytes); the index in child_of gets that child pointer. Returns the head (0 for none)."""
    for i, ident in enumerate(ids):
        nxt = first + NODE * (i + 1) if i + 1 < len(ids) else 0
        regions[first + NODE * i] = node(r, ident, nxt, (child_of or {}).get(i, r.choice([0, r.getrandbits(32) | 0x80000000])))
    return first if ids else 0


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
    if op == 'FindChild':
        ids = [r.choice([0, 1, 5, 0x18, 0x1f, 0x100, r.getrandbits(32)]) for _ in range(r.choice([0, 1, 2, 3, 6]))]
        want = r.choice(ids + [r.choice([0, 5, 0x18, r.getrandbits(32)])]) if ids else r.choice([0, 5])
        win = bytearray(r.bytes(0x90))
        put(win, 0x5c, 'I', chain(r, regions, ids, NODES))
        regions[WINDOW] = win
        return Call(CALLS[op], [WINDOW, want], regions, conv='t', ret='a')
    struct.pack_into('<I', log, LOG_BYTES + 4 * 5, 1)
    buttons = list(BUTTONS)
    if r.random() < 0.05:
        buttons.remove(r.choice(BUTTONS[1:]))
    buttons += [r.choice([0, 7, 0x22, 0x1000]) for _ in range(r.choice([0, 1, 2]))]
    r.shuffle(buttons)
    panel_at = NODES + NODE * 12
    head = chain(r, regions, buttons, panel_at)
    outer = [r.choice([1, 0x17, 0x22]) for _ in range(r.choice([0, 1, 2]))]
    pos = r.randrange(len(outer) + 1)
    outer.insert(pos, 0x18)
    first = chain(r, regions, outer, NODES, {pos: head})
    win = bytearray(r.bytes(0x90))
    put(win, 0x5c, 'I', first)
    regions[WINDOW] = win
    misc = bytearray(r.bytes(0x60))
    misc[0x50] = r.choice([0, 0, 1, 2, 0xff])
    regions[MISC] = misc
    regions[0x7ca1d0] = struct.pack('<I', MISC)
    regions[0x7ca1c8] = struct.pack('<II', WINDOW if op == 'SetChecks' or r.random() < 0.8 else 0, r.choice([0, 1, 1, 1, 0x100]))
    flags = r.choice([0, 1, 2, 3, 0x100, 0x1ff, 0xffff, r.getrandbits(9), r.getrandbits(32)])
    return Call(CALLS[op], [flags], regions)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

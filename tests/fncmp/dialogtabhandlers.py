# The tabbed dialogs' handlers (src/game/ui/dialogtabhandlers: 0x498c60, 0x4b24b0, 0x4b7a70) against the originals: their calls recorders as DialogTabHandlersEnv's from 10, each message and each button; Window_FindChild real over windows by hand (the figures 0x2de missing now and then), the two figures' slot 7 recorded, the budget a region.
# usage: dialogtabhandlers.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, vtable  # noqa: E402

CDECL = [(0x658d9f, 4), (0x488ba0, 4), (0x486b40, 1)]
HOOKS = {a: _recorder(10 + k, n, cdecl=True) for k, (a, n) in enumerate(CDECL)}
HOOKS.update({0x5195d0: _recorder(13, 0), 0x4d05d0: _recorder(14, 1), 0x519ef0: _recorder(15, 2), 0x519450: _recorder(21, 0)})
HOOKS.update({a: _recorder(16 + k, 0, cdecl=True) for k, a in enumerate((0x49ac60, 0x49fb30, 0x4b2750, 0x498d80, 0x4a52a0))})
B = Bench([(0x498c60, 0x498d75), (0x4b24b0, 0x4b2725), (0x4b7a70, 0x4b7afe), (0x65d7bc, 0x65d840)], hooks=HOOKS)
WINDOW, DIALOG, FIGURES, KIDS, BUDGET, VT = (RAM + 0x1000 * k for k in range(6))
A = {0x498c60: [0x4f3ab, 0x4f3ac, 0x4f3ad, 0x4f3b1, 0x4f3b1, 0x4f3b0, 0x4f3aa, 0x4f3ae, 0xffffffff, 0xfffffffe, 0xfffffffd, 0],
     0x4b24b0: [0x2db, 0x2dc, 0x2dd, 0x2da, 0x2de, 0xffffffff, 0xfffffffe, 0x2e6, 0x2e7, 0x2e8, 0x2e9, 0x2ea, 0x2eb, 0x2ec, 0x2ed, 0x2ee],
     0x4b7a70: [0xadb3, 0xadb4, 0xadb5, 0xadb6, 0xfffffffe, 0xffffffff]}
M = [4, 5, 6, 0x13, 0x14, 0x15, 0xff, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x101, 0x7ff, 0x800, 0x800, 0x800, 0x800, 0x800, 0x800, 0x801]


def windows(r):
    """The dialog with the figures (absent now and then) over the two figure windows, decoys among them."""
    dialog = put(bytearray(r.bytes(0x100)), 0x5c, 'I', FIGURES if r.random() < 0.95 else 0)
    figures = bytearray(r.bytes(0x100))
    put(figures, 0x54, 'I', 0)
    put(figures, 0x5c, 'I', KIDS)
    put(figures, 0x8c, 'I', 0x2de)
    ids = [0x2df, 0x2e4, r.getrandbits(32) | 0x1000]
    r.shuffle(ids)
    kids = bytearray(r.bytes(0x300))
    for k, ident in enumerate(ids):
        put(kids, 0x100 * k, 'I', VT)
        put(kids, 0x100 * k + 0x54, 'I', KIDS + 0x100 * (k + 1) if k + 1 < len(ids) else 0)
        put(kids, 0x100 * k + 0x8c, 'I', ident)
    return dialog, figures, kids


def case(r):
    op = r.deal(list(A))
    log = log_region()
    for k in (10, 11, 13):
        set_return(log, k, r.getrandbits(32))
    set_return(log, 21, BUDGET)
    dialog, figures, kids = windows(r)
    budget = bytearray(r.bytes(0x40))
    regions = {LOG: log, DIALOG: dialog, FIGURES: figures, KIDS: kids, VT: vtable({7: 1}), BUDGET: budget, WINDOW: r.bytes(16), 0x7cc2ec: struct.pack('<I', DIALOG), 0x7ca210: r.bytes(4), 0x7cc444: r.bytes(4), 0x7ca1e8: r.bytes(4), 0x7cf83c: r.bytes(4),
               0x7cc2d4: r.bytes(4), 0x7cc2e4: r.bytes(4)}
    return Call(op, [WINDOW, r.deal(M), r.deal(A[op]), r.choice([1, 1, 0, 0xffffffff, 5, r.getrandbits(32), 0x80000000])], regions, ret='a')


if __name__ == '__main__':
    B.main(case)

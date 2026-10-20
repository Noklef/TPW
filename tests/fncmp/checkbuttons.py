# The 0x7ca1c8 dialog's check buttons (src/game/ui/dialog7ca1c8: 0x498090 / 0x498a50 / 0x481a00) against the originals. SetChecks /
# SetChecksIfUp / SetChecksIfUpThis: the dialog's child 0x18 holds the nine buttons 0x18-0x21 shuffled among a few others (now and then one
# missing: a fault in both), each with a recorder vtable whose slot 5 logs its bool (the low byte: the original pushes 0x20's with ECX's upper
# half); [0x7ca1d0] +0x50 and [0x7ca1c8] / [0x7ca1cc] (the IfUp guard) set or 0.
# usage: checkbuttons.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetChecks,SetChecksIfUp,SetChecksIfUpThis)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, LOG_BYTES, Bench, Call, log_region, put, vtable  # noqa: E402
from window import MISC, NODE, NODES, RANGES, VTABLE, WINDOW, chain  # noqa: E402

CALLS = {'SetChecks': 0x498090, 'SetChecksIfUp': 0x498a50, 'SetChecksIfUpThis': 0x481a00}
BUTTONS = list(range(0x18, 0x22))
B = Bench(RANGES)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VTABLE: vtable({5: 1})}
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
    if op == 'SetChecksIfUpThis':
        return Call(CALLS[op], [r.getrandbits(32), flags], regions, conv='t')
    return Call(CALLS[op], [flags], regions)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

# The dialog window at 0x7cb2fc (src/game/ui/dialog7cb2fc: 0x4a3960) against the original. SetToggle: the dialog [0x7cb2fc] with its child
# `row` (one of a few ids, among dialog7cb398.py's decoys; now and then missing: a fault in both) whose child 0x1d4c9 has slot 9 a recorder,
# and the 0x705a50 button `button` (as in window705a50.py, its +0x98 / +0x9c objects recorders k 23 / 24 masked as that bench says by the look
# it gets, [0xfaa5d4] set or not; now and then missing: a fault in both once selected); the label one of 0x145-0x148 or another, on / other
# / select any bytes (the pushed registers' upper bytes random). The texts as panel7cc298.py's (language.py's records of up to 12 bytes, so
# the stack copy of 31 units always ends in a terminator), the wide buffer 0x7ba058 a short wide string first (what the converter off
# leaves there).
# usage: dialog7cb2fc.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetToggle)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, Bench, Call, blob_bytes, log_region, put, recorder, vtable  # noqa: E402
from dialog7cb398 import KVTABLE, children, window  # noqa: E402
from language import text  # noqa: E402
from panel7cc298 import TEXTS, texts  # noqa: E402
from window import queues  # noqa: E402
from window705a50 import EVENT_MASK, INNER, NOTIFY_MASK, RANGES as BUTTON_RANGES, look, window as button  # noqa: E402

CALLS = {'SetToggle': 0x4a3960}
B = Bench([(0x4a3960, 0x4a3a2b), (0x485b00, 0x485b40), (0x6adc80, 0x6add25), (0x6ace60, 0x6acea0), (0x65d7bc, 0x65d832),
           (0x67b0d0, 0x67b140), (0x67bed0, 0x67bf0f)] + BUTTON_RANGES)
DIALOG, ROW, ITEM, BUTTON, DECOYS, OBJS = 0x33200000, 0x33210000, 0x33220000, 0x33230000, 0x33240000, 0x33250000
ROWS = [0x1d4cb, 0x1d4cc, 0x1d4cd]
BUTTONS = [0x1d4d1, 0x1d4d2]


def case(r):
    r.deal(OPS)
    log = log_region()
    regions = {LOG: log}
    row, btn_id = r.choice(ROWS), r.choice(BUTTONS)
    window(r, regions, ITEM, ROW, 0)
    window(r, regions, ROW, DIALOG, 0)
    regions[KVTABLE] = vtable({9: 1})
    put(regions[ROW], 0x5c, 'I', children(r, regions, [(ITEM, 0x1d4c9)], ROW, DECOYS + 0x1000, [0, 0x1d4c8, 0x1d4ca]))
    btn = button(r)
    put(btn, 0x98, 'I', OBJS if r.random() < 0.8 else 0)
    put(btn, 0x9c, 'I', OBJS + 0x40 if r.random() < 0.8 else 0)
    put(btn, 0x5c, 'I', 0)
    regions[BUTTON] = btn
    for i, k in enumerate((23, 24)):
        vt = vtable({}, size=8)
        put(vt, 0xc, 'I', recorder(k, 2))
        regions[OBJS + 0x40 * i] = put(bytearray(r.bytes(0x20)), 0, 'I', OBJS + 0x40 * i + 0x20) + vt
    kids = ([(ROW, row)] if r.random() < 0.97 else []) + ([(BUTTON, btn_id)] if r.random() < 0.95 else [])
    window(r, regions, DIALOG, 0, 0)
    put(regions[DIALOG], 0x5c, 'I', children(r, regions, kids, DIALOG, DECOYS, [0, 0x1d4c9, 0x1d4ca, 0x1d4ce]))
    regions[0x7cb2fc] = struct.pack('<I', DIALOG)
    first = b''.join(r.randrange(0x20, 0x7f).to_bytes(2, 'little') for _ in range(r.choice([0, 1, 4, r.randrange(12)])))
    texts(r, regions, 0x145, 11, first + b'\0\0' + r.bytes(0x400 - len(first)))
    if r.random() < 0.9:   # (mostly all eleven there and the on / off texts 0x14c-0x14f text records: none faults in both)
        put(regions[TEXTS + 0xa00], 8, 'I', 0x150)
        pool = bytearray(regions[TEXTS + 0xc00])
        for i in range(7, 11):
            body = text(r) + bytes([0]) * r.choice([1, 1, 2])
            pool[0x40 * i:0x40 * (i + 1)] = struct.pack('<I', 1 | len(body) << 8) + body + bytes(0x3c - len(body))
        regions[TEXTS + 0xc00] = bytes(pool)
    queues(r, regions)
    regions[0xfaa5d4] = put(bytearray(4), 0, 'I', r.choice([0, 1, 0x80000000]))
    on, other, select = (r.choice([0, 1, 1, r.getrandbits(8)]) for _ in range(3))
    after = bytearray(btn)   # (the look the notification sees: after the state is set)
    after[0x138] = 0 if on else 1
    blob_bytes(log, 23, 1, EVENT_MASK)
    blob_bytes(log, 24, 1, NOTIFY_MASK, None if look(after) == 1 else INNER)
    hi = lambda: r.choice([0, r.getrandbits(24)]) << 8
    label = r.choice([0x145, 0x146, 0x147, 0x148, 0x14c, 0x150, r.getrandbits(32)])
    args = [row, label, on | hi(), other | hi(), select | hi(), r.choice([btn_id] * 15 + [0x1d4ce])]
    return Call(CALLS['SetToggle'], args, regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)

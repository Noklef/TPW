# Scene objects leaving and stepping through their items (src/game/frontend/sceneleave) against the originals: Sound_Stop (k 20), Sound_PlayAt (k 21), Sound_ApplySettings (k 22), Obj_DrainList0c (k 23), Obj_DrainList04 (k 24),
# HolderUser_Release (k 25) and the four dialogs' Message4 (k 26-29) recorders; the object's vtable recorders (slot 9 answering 0 or 1 in turn, slot 19 changing the current item by a fill), its lists of 0 to 4 nodes, the items of a
# leaving object now and then 0, a current item of 0, in the list or not in it (a fault in both).
# usage: sceneleave.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=View,View2,Leave,Scene1,Scene2,First,Next,Previous)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, fill, heap, log_region, put, vtable  # noqa: E402

CALLS = {'View': 0x5dd220, 'View2': 0x5dd690, 'Leave': 0x5e1450, 'Scene1': 0x5e1bd0, 'Scene2': 0x5e2550, 'First': 0x5e14a0, 'Next': 0x5e1640, 'Previous': 0x5e1730}
B = Bench([(0x5dd220, 0x5dd30e), (0x5dd690, 0x5dd795), (0x5e1450, 0x5e1539), (0x5e1640, 0x5e1725), (0x5e1730, 0x5e1821), (0x5e1bd0, 0x5e1c93), (0x5e2550, 0x5e2613), (0x67acb0, 0x67acb1)],
          hooks={0x51c300: _recorder(20, 1, cdecl=True), 0x51bfc0: _recorder(21, 6, cdecl=True), 0x51bd70: _recorder(22, 0, cdecl=True), 0x5d8a10: _recorder(23, 0), 0x5da3e0: _recorder(24, 0), 0x5d8820: _recorder(25, 0),
                 0x4c5780: _recorder(26, 0, cdecl=True), 0x4b9c00: _recorder(27, 0, cdecl=True), 0x4b8e80: _recorder(28, 0, cdecl=True), 0x4b87f0: _recorder(29, 0, cdecl=True)})
ITEMS = RAM
THIS, VT, TABLE, ACTIVE = ITEMS + 0xc0, RAM + 0x200, RAM + 0x300, RAM + 0x400
LIST, NODES, LIST2, NODES2, HELD = RAM + 0x600, RAM + 0x640, RAM + 0x700, RAM + 0x740, RAM + 0x800
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def item(r):
    return put(put(bytearray(r.bytes(0x30)), 0, 'I', VT), 0x18, 'I', r.randrange(8))


def add_list(r, regions, at, nodes_at, items):
    n = len(items)
    nodes = bytearray(r.bytes(0x60))
    for k, it in enumerate(items):
        put(nodes, 0x10 * k, 'III', it, nodes_at + 0x10 * (k + 1) if k + 1 < n else 0, nodes_at + 0x10 * (k - 1) if k else 0)
    lst = bytearray(r.bytes(0x10))
    put(lst, 4, 'III', nodes_at if n else 0, nodes_at + 0x10 * (n - 1) if n else 0, n)
    regions[at], regions[nodes_at] = lst, nodes


def case(r):
    name = r.choice(OPS)
    log = log_region()
    answers(log, 9, ACTIVE)
    regions = {LOG: log, HEAP: heap(), VT: vtable({0: 1, 19: 1}), TABLE: r.bytes(0x40), 0x803a4c: struct.pack('<I', TABLE), 0xf83774: r.bytes(4), 0xf83784: r.bytes(4), 0x8bcbc8: r.bytes(4),
               ACTIVE: b''.join(struct.pack('<I', r.choice([0, 1, 1, 1, r.getrandbits(32) | 1])) for _ in range(48))}
    this = put(bytearray(r.bytes(0x80)), 0, 'I', VT)
    regions[THIS] = this
    for k in range(5):
        regions[ITEMS + 0x30 * k if k < 4 else ITEMS + 0x90] = item(r)
    items = [ITEMS + 0x20 * 0 + 0x30 * k for k in range(r.randrange(0, 4))]
    stray = ITEMS + 0x90
    current = r.choice(items + items + [0, 0] + ([stray] if r.random() < 0.3 else []))
    put(this, 0xc, 'I', current)
    if name in ('View', 'View2', 'Leave', 'Scene1', 'Scene2'):
        held = [r.choice([0, HELD + 0x40 * k, HELD + 0x40 * k, HELD + 0x40 * k]) for k in range(r.randrange(0, 4))]
        for k in range(4):
            regions[HELD + 0x40 * k] = put(bytearray(r.bytes(0x10)), 0, 'I', VT)
        add_list(r, regions, LIST, NODES, held)
        add_list(r, regions, LIST2, NODES2, held[::-1])
        put(this, 8, 'I', LIST)
        put(this, 0x6c, 'I', LIST2)
    else:
        if name == 'First':
            items = [r.choice([0, x, x, x]) if not k else x for k, x in enumerate(items)]
        add_list(r, regions, LIST, NODES, items)
        put(this, 8, 'I', LIST if name != 'First' or r.random() < 0.9 else 0)
        put(this, 0x14, 'I', r.choice([0, 0, 0, 0, 0, r.getrandbits(32) | 1]))
        if name in ('Next', 'Previous') and items:
            at = items.index(current) if current in items else None
            step = 1 if name == 'Next' else -1
            chosen = items[0 if step == 1 else -1] if not current else None if at is None else items[(at + step) % len(items)]
            if chosen:
                fill(log, 19, 0, struct.pack('<I', r.choice([chosen, chosen, items[0], 0, current])), offset=THIS + 0xc - chosen)
    return Call(CALLS[name], [THIS], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

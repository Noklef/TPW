# The world's start (World_Start 0x5156a0, src/game/world/worldstart) against the original. The setting read once or not, setting 0, 1, 2 or any; the balance loads answering 0 now and then, their paths
# logged as strings; the ten constructors recorders answering things whose handles are random; the object definitions (gObjectDefs at 0x7890a0, as arrivalvehicle.py) naming the gates and the lights or not; the object's
# cell and id logged as words (stack bytes and a register's in their high halves); the heap failing at any allocation (a null thing's handle read faults, in both); the message centre set or not.
# usage: worldstart.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, blob, heap, log_region, put, set_return, strings  # noqa: E402

BALANCE, OBJECT, POST, CTORS = 4, 5, 6, 10
THINGS = [0x5089c0, 0x4da920, 0x599e70, 0x509b40, 0x4c58c0, 0x502d20, 0x511f70, 0x4cf7c0, 0x50c9b0, 0x4d1bd0]
hooks = {0x4d3de0: _recorder(0, 0), 0x403b00: _recorder(1, 0), 0x550ca0: _recorder(2, 0), 0x67a7d0: _recorder(3, 1, cdecl=True), 0x401650: _recorder(BALANCE, 2), 0x4db090: _recorder(OBJECT, 4), 0x40fb10: _recorder(POST, 1),
         0x67d3f0: _recorder(7, 1, cdecl=True), 0x4d7bb0: _recorder(8, 0), 0x4d7af0: _recorder(9, 0), 0x502d60: _recorder(20, 0), 0x544360: _recorder(21, 0, cdecl=True), 0x50fdf0: _recorder(22, 0, cdecl=True),
         0x67acf0: _recorder(23, 5, com=True)}
hooks.update({at: _recorder(CTORS + i, 0) for i, at in enumerate(THINGS)})
B = Bench([(0x5156a0, 0x515dce), (0x412e60, 0x412f00), (0x50b350, 0x50b35d), (0x40f630, 0x40f649), (0x67a400, 0x690000), (0x5da3c0, 0x5da3c1), (0x5f5fc0, 0x5f7f00)], hooks=hooks)
DEFS, ITEMS, ITEM, TEXTS, LEVEL, THING0 = 0x7890a0, RAM + 0x1000, 0x400, RAM + 0x4000, RAM + 0x4800, RAM + 0x6000
WORLD = RAM + 0x10000 - 0x1da700
NAMES = {0x8021b0: 'Gates', 0x7cf938: 'Lights'}
ROWS = 16


def wide(s):
    return s.encode('utf-16-le') + b'\0\0'


def case(r):
    log = log_region()
    regions = {HEAP: heap(0x30000, fail=r.choice([0] * 100 + list(range(1, 14)) + [12, 13] * 8)), LOG: log, LEVEL: r.choice([b'data\\levels\\a\0', b'b\0', b'\0'])}
    regions[RAM + 0x10000] = bytearray(r.bytes(0x40))
    regions[0x7cf83c] = struct.pack('<I', r.getrandbits(32))
    regions[0x802080] = r.bytes(0x24)
    regions[0xfb34a8] = bytes([r.getrandbits(8), r.getrandbits(8) if r.random() < 0.3 else r.getrandbits(8) & 0xfe, r.getrandbits(8), r.getrandbits(8)])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([0, 1, 2, 2, r.getrandbits(32)]))
    regions[0x7cf4a0] = struct.pack('<I', TEXTS + 0x400)
    regions[TEXTS + 0x400] = r.choice([b'Easy\\\0', b'\0'])
    regions[0x788d3c] = struct.pack('<I', r.choice([0, r.getrandbits(32)]))
    # The things: their handles (the table's rows for ResearchLab_Init's).
    rows = bytearray(r.bytes(20 * ROWS))
    things = bytearray(r.bytes(0x200))
    for i in range(10):
        set_return(log, CTORS + i, THING0 + 0x20 * i)
        put(things, 0x20 * i, 'H', r.randrange(ROWS))
    regions[THING0] = things
    regions[0x7cfb90] = rows
    strings(log, BALANCE, 1)
    regions[TEXTS + 0x600] = struct.pack('<40I', *[r.choice([1, 1, 1, 1, 0, 2, r.getrandbits(32)]) for _ in range(40)])
    answers(log, BALANCE, TEXTS + 0x600)   # (each load its own answer: one failing alone decides)
    set_return(log, 7, r.getrandbits(32))
    set_return(log, OBJECT, THING0 + 0x180)
    blob(log, POST, 0, 7)
    struct.pack_into('<I', log, LOG_BYTES + 4 * OBJECT, 3 << 16)   # (the cell and the id: words)
    regions[0x7cf930] = struct.pack('<I', r.getrandbits(32)) * 2 + struct.pack('<I', TEXTS + 0x100) + r.bytes(0x10)
    regions[0x8021a8] = r.bytes(8) + struct.pack('<I', TEXTS)
    regions[TEXTS] = wide('Gates')
    regions[TEXTS + 0x100] = wide('Lights')
    n = r.randrange(1, 7)
    dtab = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(ITEM * n))
    for i in range(n):
        put(dtab, 4 * i, 'I', ITEMS + ITEM * i)
        s = r.choice(list(NAMES.values()) + ['gates', 'LIGHTS', 'Other', ''])
        items[ITEM * i + 0x390:ITEM * i + 0x390 + 2 * len(s) + 2] = wide(s)
    put(dtab, 0x528, 'i', r.choice([n, n, n, n, n - 1, 0]))
    put(dtab, 0x72c, 'I', r.getrandbits(32))
    regions.update({DEFS: dtab, ITEMS: items})
    return Call(0x5156a0, [WORLD, LEVEL], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)

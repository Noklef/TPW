# The recorded actions (src/game/core/actionrec: 0x447e10, 0x52fa90, 0x52fad0, 0x52f1b0, 0x52f880) against the originals: the
# global recorder 0x785a08 recording or not, playing back or not, its first stream a record stream (the driver's write stub,
# at most `left` bytes to the cursor); values about the word's range, the 0x52f1b0 value equal to 0x81b134 or not with force
# 0, 1 or a dword with a clear low byte; the 0x52f880 id a definition the object database finds (as World_MarkRideBuilt's
# bench sets it up), its flag 0, 1 or random.
# usage: recorded_actions.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Coaster45,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, STUBS, Bench, Call, put  # noqa: E402

B = Bench([(0x403be0, 0x403f00), (0x447e10, 0x447e40), (0x52f1b0, 0x52f200), (0x52f880, 0x52f8e0), (0x52fa90, 0x52fac0),
           (0x52fad0, 0x52fb00), (0x51ab20, 0x51ab70), (0x412e60, 0x412e6d), (0x412e70, 0x412e8f), (0x412e90, 0x412f00),
           (0x5da3c0, 0x5da3c1)])
RECORDER, VT, BUF, COASTER, ID, ITEMS = 0x785a08, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x700, RAM + 0x1000
DEFS, BUILT, NITEMS = 0x7890a0, 0x802080, 24
CALLS = {'Coaster45': 0x447e10, 'Set81b0cc': 0x52fa90, 'Set81ecac': 0x52fad0, 'Set81d7a4': 0x52f1b0, 'Set8186e0': 0x52f880}


def recorder(r):
    """The global recorder: its flags, its first stream a record stream (vtable VT, cursor BUF, bytes left)."""
    o = bytearray(r.bytes(0x30))
    struct.pack_into('<2I', o, 0, r.choice([0, 1, 1, 1, r.getrandbits(32)]), r.choice([0, 0, 1, 2, r.getrandbits(32)]))
    struct.pack_into('<3I', o, 0xc, VT, BUF, r.choice([0, 1, 2, 3, 4, 0x20, 0x20, 0x20, 0x20]))
    struct.pack_into('<I', o, 0x24, r.choice([0, 0, 0, 0, 1, r.getrandbits(32)]))
    vt = bytearray(r.bytes(0x10))
    struct.pack_into('<I', vt, 0xc, STUBS + 0x10)
    return {RECORDER: o, VT: vt, BUF: r.bytes(0x20)}


def value(r):
    return r.choice([0, 1, 0xffff, 0x10000, 0x12345, r.randrange(0x100), r.getrandbits(32)])


def case(r):
    op = r.choice(OPS)
    regions = recorder(r)
    globals_ = {0x81b0c8: 0xc, 0x81eca8: 0xc, 0x81b130: 0xc, 0x81d7a0: 0xc, 0x8186d8: 0x20, 0x8187f0: 0x10}
    for address, size in globals_.items():
        regions[address] = r.bytes(size)
    if op == 'Coaster45':
        regions[COASTER] = r.bytes(0x20)
        return Call(0x447e10, [COASTER, value(r)], regions)
    if op in ('Set81b0cc', 'Set81ecac'):
        return Call(CALLS[op], [value(r)], regions)
    if op == 'Set81d7a4':
        v = value(r)
        if r.random() < 0.5:
            regions[0x81b130] = put(bytearray(regions[0x81b130]), 4, 'I', v)
        return Call(0x52f1b0, [v, r.choice([0, 0, 1, 0x100, 0xff, r.getrandbits(32)])], regions)
    defs = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(NITEMS * 0xa0))
    count = r.choice([1, r.randrange(1, NITEMS + 1), NITEMS])
    for i in range(NITEMS):
        put(defs, i * 4, 'I', ITEMS + 0xa0 * i)
        put(items, i * 0xa0, 'H', i + 1)
        put(items, i * 0xa0 + 0x94, 'I', r.choice([0, 1, 8, 9, 256, r.randrange(1, 257), r.randrange(1, 17)]))
    put(defs, 0x528, 'i', count)
    put(defs, 0x730, 'HxxI', 0, 0)  # no cache hit
    id_ = bytearray(r.bytes(4))
    put(id_, 0, 'H', r.randrange(1, count + 1))  # (found, as the original needs)
    regions.update({DEFS: defs, ITEMS: items, BUILT: r.bytes(0x20), ID: id_, 0x7cf83c: r.bytes(4)})
    return Call(0x52f880, [ID, r.choice([0, 1, 0x100, r.getrandbits(32)])], regions)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

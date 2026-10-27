# The slot table functions and the counted list erase of src/game/core/slots_lists against the originals: slot selections around the limits,
# matching and mismatching ids, entries naming object definitions or control records, nodes with counted buffers. Event (0x4d2860,
# src/game/world/object): Add's object (the selected slot's id 0x18, 0x19, 0x1b, 0x10 or any, its +0x10 0-4 or not), an event whose slot 0
# (a recorder) answers 0xb mostly, 0x11, 0x1a or another, its id / amount / kind word the slot's or not; thingenv's park of objects, people
# and staff, their floats +0x44 / +0x1f8 0..100 mostly; the game [[0x7cf6ec]] whose word +0x1da720 names one of the park's slots.
# usage: slots_lists.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Add,Get,Erase,Event)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, HEAP, RAM, heap, log_region, put, set_return, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from thingenv import POOL, THING, park  # noqa: E402

CALLS = {'Add': (0x4d27a0, 0xa0), 'Get': (0x503a20, 0xa0), 'Erase': (0x4f3400, 0xa0), 'Event': (0x4d2860, 0x200)}
B = Bench([(a, a + n) for (a, n) in CALLS.values()] + [(0x5da3c0, 0x5da3c1), (0x412e90, 0x412eea), (0x4d3d80, 0x4d3df0),
                                                       (0x4d2a60, 0x4d2a6e), (0x59b540, 0x59b544), (0x51c050, 0x51c054), (0x409cd0, 0x409cdd),
                                                       (0x519510, 0x51954d), (0x4c9cc0, 0x4c9d73), (0x4c9d80, 0x4c9e38), (0x4ca160, 0x4ca214),
                                                       (0x516130, 0x516160), (0x516160, 0x5161b0), (0x5161b0, 0x516200), (0x50b350, 0x50b360),
                                                       (0x41a960, 0x41a970), (0x4e0440, 0x4e0450), (0x4e2710, 0x4e2740), (0x4dd4e0, 0x4dd500)])
LIST, BUFS, TABLE = RAM + 0x1000, RAM + 0x2000, RAM + 0x20000
EVENT, EVTVT, PG, GAME = RAM + 0x1600, RAM + 0x1700, RAM + 0x1800, RAM + 0x40000


def event_case(r):
    """Event (above)"""
    obj = bytearray(r.bytes(0x1500))
    regions, _h, _a = park(r, kinds=(3, 3, 1, 4, 5, 6, 7, 8, 0), defs=False)
    things = bytearray(regions[POOL])
    for i in range(len(things) // THING):
        for off in (0x44, 0x1f8):
            f = r.choice([0.0, 1.0, 50.0, 99.9, 100.0, r.uniform(0, 100)]) if r.random() < 0.9 else r.choice([-1.0, 255.9, 300.0, float('nan')])
            put(things, THING * i + off, 'f', f)
    regions[POOL] = things
    log = log_region()
    set_return(log, 0, r.choice([0xb] * 5 + [0x11, 0x11, 0x1a, 0x1a, 0, 0x12, r.getrandbits(32)]))
    index = r.choice([0, 1, 5, 19, r.randrange(20), r.randrange(20), -1, 20])
    put(obj, 0x3cc, 'i', index)
    obj[0x3d9] = r.choice([0, 1, 1, 1, 7])
    put(obj, 0x3e0, 'H', r.choice([0, 0, 0, 5]))
    ident = r.choice([0x18, 0x19, 0x1b, 0x1b, 0x10, 0x1a, r.getrandbits(32)])
    kind = r.choice([0, 0, 0, 5, 0x1234])
    if 0 <= index < 20:
        put(obj, 0xc + 48 * index, 'I', ident)
        put(obj, 0xc + 48 * index + 0xc, 'H', kind)
        put(obj, 0xc + 48 * index + 0x10, 'I', r.choice([0, 1, 2, 3, 4, 4, 5, r.getrandbits(32)]))
    ev = bytearray(r.bytes(0x20))
    put(ev, 0, 'I', EVTVT)
    put(ev, 8, 'I', r.choice([ident, ident, r.getrandbits(32)]))
    put(ev, 0xc, 'I', r.choice([0, 0, 1, 100, r.getrandbits(32)]))
    put(ev, 0x10, 'H', r.choice([kind, kind, kind ^ 1]))
    game = put(bytearray(r.bytes(0x40)), 0x20, 'H', r.randrange(12))
    regions.update({RAM: obj, LOG: log, EVENT: ev, EVTVT: vtable({0: 0}), PG: struct.pack('<I', GAME), GAME + 0x1da700: game,
                    0x7cf6ec: struct.pack('<I', PG)})
    return Call(0x4d2860, [RAM, EVENT], regions, conv='t', ret='v')


def case(r):
    name = r.choice(OPS)
    if name == 'Event':
        return event_case(r)
    address, _n = CALLS[name]
    obj = bytearray(r.bytes(0x1500))
    regions = {RAM: obj, HEAP: heap()}
    if name == 'Add':
        put(obj, 0x3cc, 'i', r.choice([-1, 0, 1, 5, 19, 20, 21, 0x7fffffff]))
        obj[0x3d9] = r.choice([0, 1, 1, 7])
        put(obj, 0x3e0, 'H', r.choice([0, 0, 5, 0x1234]))
        put(obj, 0x3dc, 'I', r.getrandbits(32))
        ident, kind, extra = r.choice([1, 5, 0x1234, 0xffffffff]), r.choice([0, 5, 0x1234, 0x11234]), r.choice([0, 5, 0x1234, 0x21234])
        slot = r.randrange(20)
        for n in range(20):
            put(obj, 0xc + 48 * n, 'I', ident if r.random() < 0.7 else r.getrandbits(32))
            put(obj, 0xc + 48 * n + 0xc, 'H', kind & 0xffff if r.random() < 0.7 else r.getrandbits(16))
        return Call(address, [RAM, ident, r.choice([0, 1, 100, 0xffffffff]), kind, extra], regions, conv='t', ret='v')
    if name == 'Get':
        for k in range(5):
            put(obj, 0x1414 + 16 * k, 'i', r.choice([-1, 0, 1, 2, 3]))
        dtab, _h, _a = None, None, None
        defs, _h, _a = park(r, count=0, defs=True)
        regions.update({k: v for k, v in defs.items() if k in (0x7890a0, 0x30008000)})
        items = defs[0x30008000]
        for n in range(4):  # the entries: word +0xc names a definition (slot 4) or a control record, byte +0x15 a part
            o = 0xc + 20 * n
            put(obj, o, 'H', struct.unpack_from('<H', items, 0x500 * r.randrange(16))[0] if r.random() < 0.7 else r.choice([1, 2, 3, 9]))
            obj[20 * n + 0x15] = r.choice([0, 1, 2, 5, 7])
        table = bytearray(r.bytes(0x1400))
        have = r.choice([0, 1, 3, 6])
        put(table, 0x12c0, 'i', have)
        for k in range(have):
            put(table, 0x20 * k, 'H', r.choice([1, 2, 3, 9, struct.unpack_from('<H', obj, 0xc)[0]]))
        put(table, 0x12c4, 'H', r.choice([0, 1, 2, 3, 9]))
        put(table, 0x12c8, 'I', r.choice([0, TABLE + 0x20 * r.randrange(6)]))
        regions[TABLE] = table
        regions[0x80239c] = struct.pack('<I', TABLE - 0x1d943c)
        return Call(address, [RAM, r.choice([0, 1, 2, 3, 4, 4, 4])], regions, conv='t', ret='a')
    n = r.choice([1, 2, 3, 4])
    nodes = [LIST + 0x20 * (i + 1) for i in range(n)]
    order = [LIST] + nodes
    data = bytearray(r.bytes(0x20 * (n + 1)))
    bufs = bytearray(r.bytes(0x100))
    for i, a in enumerate(order):
        put(data, a - LIST, 'II', order[(i + 1) % len(order)], order[i - 1])
        use = r.choice([0, 1, 2, 5, 0xfe, 0xff])
        bufs[0x20 * i + 2] = use
        put(data, a - LIST + 0xc, 'I', r.choice([0, BUFS + 0x20 * i + 3, BUFS + 0x20 * i + 3]))
    put(obj, 8, 'I', r.choice([n, n + 1, 0, 100]))
    regions.update({LIST: data, BUFS: bufs})
    return Call(address, [RAM, RAM + 0x300, r.choice(order), r.choice(order)], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

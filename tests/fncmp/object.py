# The objects' definition lookups, counters and setters (src/game/world/object: 0x4dd4e0, 0x4e1670,
# 0x4e1690, 0x4e1e00, 0x4e2190, 0x4e21c0, 0x4e2710, 0x4e2b50) against the originals: ids found in
# gObjectDefs or its cache (the pushed high word anything), counts and averages about the wraps. The scrap value by age (0x4e2290): the
# world's calendar clock (funnytime.py's recorders) and the object's date set so its age is about 0, 30 days and 1 to 3 years (365 days)
# or anything; an upgrade level of 0 to 7. Its share of the first level's cost (0x4e2400): costs about the product's wrap. The
# mechanic's stamp (0x4e01f0) and +0x180 - +0x184 (0x4e1c60): any object bytes, handle word and clock. ShowRefund (0x4b47a0, the panel
# 0x7cc328's slot 5, src/game/ui/panel7cc328): Refund's surroundings with the object the shown thing [0x7c2658], the panel's window +0x14
# with the child 0x1e8a among 0-3 decoys, its slot 7 a recorder of one.
# usage: object.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=GetDef,AddCustomer,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE as IAT, LOG, Bench, Call, RAM, fill, import_table, log_region, put, set_return, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from funnytime import IMPORTS, K, RANGES as CLOCK_RANGES  # noqa: E402

B = Bench([(0x4dd4e0, 0x4dd4fb), (0x4e1670, 0x4e168b), (0x4e1690, 0x4e16ab), (0x4e1e00, 0x4e1e2b),
           (0x4e2190, 0x4e21a2), (0x4e21c0, 0x4e21d2), (0x4e2710, 0x4e273b), (0x4e2b50, 0x4e2b7b),
           (0x412e90, 0x412eea), (0x4e2290, 0x4e23f4), (0x4e2400, 0x4e243f), (0x4e01f0, 0x4e0209), (0x4e1c60, 0x4e1c70),
           (0x4b47a0, 0x4b47cb), (0x65d7bc, 0x65d822)] + CLOCK_RANGES)
DEFS, OBJ, ITEMS, NITEMS = 0x7890a0, RAM, RAM + 0x400, 16
WORLD = 0x31000000   # (its calendar clock at +0x2a0 and its ticks at +0x1da70c)
DAY, YEAR = 864000000000, 365 * 864000000000   # FILETIME ticks
CALLS = {'GetDef': (0x4dd4e0, 'f', 'a'), 'AddWalkAway': (0x4e1670, 'f', 'v'), 'AddCustomer': (0x4e1690, 'f', 'v'),
         'Happiness': (0x4e1e00, 't', 'v'), 'Quality': (0x4e2190, 't', 'v'), 'Chance': (0x4e21c0, 't', 'v'),
         'Word284': (0x4e2710, 't', 'a'), 'IsKind3': (0x4e2b50, 'f', 'a'), 'Scrap': (0x4e2290, 'f', 'a'), 'Refund': (0x4e2400, 'f', 'a'),
         'Stamp5e': (0x4e01f0, 't', 'a'), 'Diff': (0x4e1c60, 't', 'a'), 'ShowRefund': (0x4b47a0, 't', 'v')}
PANEL, WIN, KIDS, KVT = RAM + 0x8000, RAM + 0x8100, RAM + 0x8200, RAM + 0x8800


def clock(r, regions, obj):
    """The world's clock, its ticks and the imports; the object's date (+0x18) an age chosen about the steps before now."""
    log = log_region()
    regions[LOG] = log
    regions[IAT] = import_table(log, IMPORTS)
    fill(log, K['FileTimeToSystemTime'], 1, r.bytes(16), 0)
    set_return(log, K['FileTimeToSystemTime'], r.choice([0, 1]))
    regions[0x80239c] = struct.pack('<I', WORLD)
    rate, ticks = r.choice([0, 1, 15000, r.randrange(1 << 16), r.getrandbits(32)]), r.choice([0, 4, r.randrange(1 << 20), r.getrandbits(32)])
    start = r.choice([0, r.getrandbits(56), r.getrandbits(64)])
    c = bytearray(r.bytes(0x20))
    put(c, 0, 'Q', start)
    put(c, 0x1c, 'I', rate)
    regions[WORLD + 0x2a0] = c
    regions[WORLD + 0x1da70c] = struct.pack('<I', ticks)
    product = rate * ticks
    seconds = product // 4 if product < 1 << 63 else -((2 ** 64 - product) // 4)
    now = (start + seconds * 10000000) % 2 ** 64
    age = r.choice([0, 1, 30 * DAY - 1, 30 * DAY, 30 * DAY + 1, YEAR - 1, YEAR, 2 * YEAR - 1, 2 * YEAR, 3 * YEAR - 1, 3 * YEAR, 3 * YEAR + 1,
                    r.randrange(30 * DAY), r.randrange(4 * YEAR), r.randrange(1 << 63), -r.randrange(1, 4 * YEAR), r.getrandbits(64),
                    YEAR << 32, (YEAR << 32) + YEAR, (30 * DAY) << 32])
    put(obj, 0x18, 'Q', (now - age) % 2 ** 64)
    obj[0x50] = r.choice([0, 0, 1, r.randrange(8)])


def word(r):
    return r.choice([0, 1, 0x7fffffff, 0x80000000, 0xffffffff, r.randrange(256), r.getrandbits(32)])


def case(r):
    op = r.deal(OPS)
    addr, conv, ret = CALLS[op]
    if op == 'Stamp5e':  # any object bytes and handle word; the world's clock
        regions = {OBJ: r.bytes(0x80), 0x802390: put(bytearray(r.bytes(0x20)), 0xc, 'I', RAM + 0x1000 - 0x1da70c), RAM + 0x1000: r.bytes(0x20)}
        return Call(addr, [OBJ, r.getrandbits(32)], regions, conv=conv, ret=ret)
    if op == 'Diff':  # any object bytes
        return Call(addr, [OBJ], {OBJ: r.bytes(0x200)}, conv=conv, ret=ret)
    obj = bytearray(r.bytes(0x400))
    for off in (0x1a0, 0x1a4, 0x1a8, 0x230, 0x340):
        put(obj, off, 'I', word(r))
    defs = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(NITEMS * 0x300 + 0x400))   # (the last's upgrade levels run past its 0x300)
    n = r.randrange(1, NITEMS + 1)
    ids = [r.choice([i, r.getrandbits(16)]) for i in range(NITEMS)]
    for i in range(NITEMS):
        put(defs, i * 4, 'I', ITEMS + 0x300 * i)
        put(items, i * 0x300, 'H', ids[i])
        put(items, i * 0x300 + 0x9c, 'I', r.choice([3, 3, 0, 2, 0x103, r.getrandbits(32)]))
    put(defs, 0x528, 'i', n)
    k = r.randrange(n)
    id_ = ids[k]
    hit = r.random() < 0.3
    put(defs, 0x730, 'HxxI', id_ if hit else id_ ^ r.randrange(1, 0x10000),
        ITEMS + 0x300 * r.randrange(NITEMS))
    put(obj, 0xe, 'H', id_)
    if op in ('Scrap', 'Refund', 'ShowRefund'):
        regions = {DEFS: defs, OBJ: obj, ITEMS: items}
        clock(r, regions, obj)
        for i in range(NITEMS):
            put(items, i * 0x300 + 0x188, 'i', r.choice([0, 1, -1, 100, r.getrandbits(32) - (1 << 31)]))
            put(items, i * 0x300 + 0x1b8, 'I', r.choice([0, 1, 100, 5000, 0x28f5c29, r.getrandbits(32)]))
        if r.random() < 0.5:
            put(obj, 0x1a0, 'I', 0)
        if op == 'ShowRefund':   # (the child 0x1e8a last of the window's children, 0-3 decoys ahead)
            n = r.choice([0, 1, 2, 3])
            kids = bytearray()
            for k in range(n + 1):
                kid = bytearray(r.bytes(0x100))
                put(kid, 0, 'I', KVT)
                put(kid, 0x8c, 'I', 0x1e8a if k == n else r.choice([0, 0x1e89, 0x1e8b, 0x11e8a, r.getrandbits(32)]))
                put(kid, 0x54, 'I', KIDS + 0x100 * (k + 1) if k < n else r.choice([0, 0, KIDS]))
                kids += kid
            regions.update({KIDS: kids, WIN: put(bytearray(r.bytes(0x100)), 0x5c, 'I', KIDS), KVT: vtable({7: 1}),
                            PANEL: put(bytearray(r.bytes(0x40)), 0x14, 'I', WIN), 0x7c2658: struct.pack('<I', OBJ)})
            return Call(addr, [PANEL], regions, conv=conv, ret=ret)
        return Call(addr, [OBJ], regions, conv=conv, ret=ret)
    args = [OBJ]
    if op in ('Happiness', 'Quality', 'Chance'):
        args.append(word(r))
    elif op == 'Word284':
        args.append(OBJ + 0x3f0 + r.randrange(4))
    return Call(addr, args, {DEFS: defs, OBJ: obj, ITEMS: items}, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)

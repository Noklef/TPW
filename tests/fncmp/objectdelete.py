# An object deleted (src/game/world/object: Object_Delete 0x4dd0a0, Thing_Delete's kind 3) against the original, in thingdetach.py's
# surroundings of Thing_Detach (its park, the thing's map cell chain, the message centre; agent B's unported Ride_Destroy 0x463c80 its
# recorder), the thing the object (kind 3) at column 0-127, row 0-119. The world [0x80239c] (and [0x7cf83c]) object.py's clock world with
# the object's date about the scrap value's steps: its controls table (+0x1d943c: 0-5 records of a few ids, the object's mostly, cached or
# not), its list of objects (head +0x1da746, the things' +0xc words) with the object in it, the money record (+0x1da726's slot,
# its +0xc and +0x124 any) and the record +0x1da720's slot (+0x1fc90 any), the mode +0x1da738 2 or not. The definitions park()'s, their
# +0x188 values and +0x1b8 costs about the product's wrap. The flags +0x32 bits 0, 4 and 7 any; the ride list (0x8791b0, ready or not)
# with the object's ride +0x24 mostly, its element 0 about 2 (or no elements); the float +0x44 about 25 (fractions, past 255, negative,
# huge or NaN); the map's spread records 1, 4, 6 and 7 of [0x8023a0] (radius 0-3) and the cells around the object's; the sprite script
# +0x54 0 or a slot of the script VM's table (thingdetach.py's); 0x785a2c and the global 0x7c2654 (the object or not). The post (agent
# B's 0x40fb10 through the objects' env) recorder 21 logging the message's bytes 0-9, agent B's unported RSSE_KillScript (0x559060) cdecl
# recorder 20 (driver.cpp's kObjectDeleteEnv).
# usage: objectdelete.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import object as objectbench  # noqa: E402
import thingdetach as td  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob_bytes, global_heap, heap, put  # noqa: E402
from funnytime import RANGES as CLOCK_RANGES  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, SLOTS, THING  # noqa: E402
from wctimage import merged  # noqa: E402

B = Bench(merged(td.RANGES + CLOCK_RANGES + [
    (0x4dd0a0, 0x4dd303), (0x4d3d10, 0x4d3d80), (0x519dc0, 0x519f00), (0x40f470, 0x40f4a0), (0x412e90, 0x412eea), (0x4e2290, 0x4e23f4),
    (0x4d0190, 0x4d01f0), (0x5195d0, 0x51960d), (0x519510, 0x51954d), (0x4815a0, 0x4815c0), (0x48c930, 0x48c94a), (0x4d8460, 0x4d8620),
    (0x55a070, 0x55a0c0), (0x55a390, 0x55a3f0)]),
          hooks={0x463c80: _recorder(31, 1, cdecl=True), 0x40fb10: _recorder(21, 1), 0x559060: _recorder(20, 2, cdecl=True),
                 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
WORLD = objectbench.WORLD
MONEY, PAID, RIDES = RAM + 0x63000, RAM + 0x64000, RAM + 0x65000
FLOATS = [0.0, 10.0, 24.0, 24.9, 25.0, 25.5, 26.0, 100.0, 255.0, 256.0 + 24, 300.0, -1.0, -300.0, 1e10, float('inf'), float('nan')]


def spread(r, regions, base, cell):
    """Spread records 1, 4, 6 and 7 of the map at base and the cells around the map id cell."""
    records = bytearray(r.bytes(12 * 8))
    radius = 0
    for k in (1, 4, 6, 7):
        records[12 * k + 0xa] = r.choice([0, 1, 2, 3, 3])
        radius = max(radius, records[12 * k + 0xa])
        struct.pack_into('<5h', records, 12 * k, *[r.choice([0, 1, -1, 100, -100, 0x7fff, -0x8000, r.randrange(-300, 300)]) for _ in range(5)])
    regions[base + 0x1d9104] = records
    at = (cell - 1) & 0xffff
    cx, cy = at & 0x7f, at >> 7
    xmin, xmax = max(cx - radius, 0), min(cx + radius, 0x7f)
    for y in range(max(cy - radius, 0), min(cy + radius, 0x7f) + 1):
        regions[base + 0x1b1104 + ((y << 7) + xmin) * 10] = r.bytes((xmax - xmin + 1) * 10)


def case(r):
    r.deal(['Delete'])
    td.OPS = ['Thing']
    c = td.case(r)
    thing, regions = c.args[0], c.regions
    pool, slots = bytearray(regions[td.POOL]), bytearray(regions[SLOTS])
    t = thing - td.POOL
    things = {}
    for h in range(1, 12):
        ptr = struct.unpack_from('<I', slots, 20 * h)[0]
        if td.POOL <= ptr < td.POOL + 6 * THING:
            things[h] = ptr - td.POOL
    handles = sorted(things)
    free = [h for h in range(1, 12) if h not in things]
    mine = struct.unpack_from('<H', pool, t)[0]
    pool[t + 2] = 3
    col, row = r.randrange(128), r.randrange(120)
    pool[t + 5], pool[t + 7] = col, row
    ident = (row << 7) + col + 1
    base = (td.CELL - ident * 0x44 + 0x44) & 0xffffffff
    regions[0x8023a0] = struct.pack('<I', base)
    spread(r, regions, base, ident)
    obj = bytearray(pool[t:t + THING])
    objectbench.clock(r, regions, obj)   # (the clock world, [0x80239c], the object's date and upgrade level +0x50)
    regions[0x7cf83c] = regions[0x80239c]
    blob_bytes(regions[LOG], 21, 0, 0x3ff)
    # the definitions (park()'s): scrap values and costs
    items = bytearray(regions[ITEMS])
    for i in range(NITEMS):
        for year in range(4):   # (the upgrade level +0x50 is 0-7: within the item)
            put(items, ITEM * i + 0x188 + 0x40 * obj[0x50] + 4 * year, 'i', r.choice([0, 1, -1, 50, 100, r.getrandbits(32) - (1 << 31)]))
        put(items, ITEM * i + 0x1b8, 'I', r.choice([0, 1, 100, 5000, 0x28f5c29, r.getrandbits(32)]))
    regions[ITEMS] = items
    if r.random() < 0.5:
        put(obj, 0x1a0, 'I', 0)
    # the controls table
    oid = struct.unpack_from('<H', obj, 0xe)[0]
    table = bytearray(r.bytes(0x12cc))
    have = r.choice([1, 2, 3, 5])
    put(table, 0x12c0, 'i', have)
    ids = [r.choice([oid ^ 1, oid ^ 0x100, r.getrandbits(16)]) for _ in range(have)]
    if r.random() < 0.95:
        ids[r.randrange(have)] = oid
    for k, i in enumerate(ids):
        put(table, 0x20 * k, 'H', i)
        put(table, 0x20 * k + 0x18, 'i', r.choice([0, 1, 5, -1, r.getrandbits(32) - (1 << 31)]))
    cached = r.random() < 0.3 and oid in ids
    put(table, 0x12c4, 'HxxI', oid if cached else oid ^ r.randrange(1, 0x10000), WORLD + 0x1d943c + 0x20 * ids.index(oid) if cached else 0)
    regions[WORLD + 0x1d943c] = table
    # the game object's handles and its list of objects (links +0xc)
    game = bytearray(r.bytes(0x30))   # (+0x1da720-0x1da74f)
    h_money, h_paid = r.sample(free, 2)
    put(slots, 20 * h_money, 'I', MONEY)
    put(slots, 20 * h_paid, 'I', PAID)
    regions[SLOTS] = slots
    regions[MONEY] = r.bytes(0x130)
    regions[PAID + 0x1fc90] = r.bytes(4)
    put(game, 0, 'H', h_paid)
    put(game, 6, 'H', h_money)
    put(game, 0x18, 'I', r.choice([2, 2, 0, 1, 3, r.getrandbits(32)]))
    order = r.sample(handles, r.randrange(0, len(handles) + 1))
    if mine not in order:   # (always in it: the walk runs off the end otherwise)
        order.insert(r.randrange(len(order) + 1), mine)
    for k, h in enumerate(order):
        put(pool if h != mine else obj, (things[h] if h != mine else 0) + 0xc, 'H', order[k + 1] if k + 1 < len(order) else 0)
    put(game, 0x26, 'H', order[0] if order else 0)
    regions[WORLD + 0x1da720] = game
    # the flags, the ride, the float, the script
    obj[0x32] = r.getrandbits(8)
    rid = r.choice([1, 2, 7])
    put(obj, 0x24, 'i', r.choice([rid, rid, rid, 3]))
    node = bytearray(r.bytes(0x100))
    put(node, 0, 'I', 0)
    put(node, 8, 'i', rid)
    put(node, 0x1c, 'I', RIDES + 0x100)
    put(node, 0x8c, 'i', r.choice([1, 1, 2, 0]))
    regions[RIDES] = node
    regions[RIDES + 0x100] = put(bytearray(r.bytes(0x10)), 0, 'i', r.choice([0, 1, 2, 3, -1, r.getrandbits(32) - (1 << 31)]))
    regions[0x8791a0] = struct.pack('<I', r.choice([0, 1, 1, 1]))
    regions[0x8791b0] = struct.pack('<I', RIDES)
    put(obj, 0x44, 'f', r.choice(FLOATS))
    td.scripts(r, regions, bytearray(THING), handles)   # (the script VM and its table)
    put(obj, 0x54, 'i', r.choice([0, 0, 0, 1, 2, 5, 7, 8, -1]))
    regions[HEAP] = heap()
    iat = bytearray(regions.get(IMPORT_TABLE, bytes(0x400)))   # (the clock's import table: SpriteScript_Kill's GlobalFree too)
    for slot, stub in global_heap().items():
        iat[slot - IMPORT_TABLE:slot - IMPORT_TABLE + 4] = stub
    regions[IMPORT_TABLE] = iat
    regions[0x785a2c] = struct.pack('<I', r.choice([0, 1]))
    regions[0x7c2654] = struct.pack('<I', thing if r.random() < 0.4 else r.choice([0, td.POOL]))
    pool[t:t + THING] = obj
    regions[td.POOL] = pool
    return Call(0x4dd0a0, [thing], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)

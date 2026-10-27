# A thing taken out of the world (src/game/world/things: Thing_Detach 0x50b980; person: Person_Detach 0x4f8ae0, Visitor_Detach 0x4fb330;
# weather: Weather_Detach 0x512000) against the originals: thingenv's park (1-6 things, their slots; unused slots without a thing), the thing one
# of them, linked into its map cell's chain (the cell at its column +5 / row +7 alone in the case, [0x8023a0] set so that the id lands on it;
# the chain's order random, the thing in it mostly), its kind (+2) any of the ones the code tests and others; agent B's unported Ride_Destroy
# (0x463c80) a cdecl recorder (k 31, driver.cpp's kThingDetachEnv); 0x877350 the thing or not; the message centre made ([0x788d3c]) with 29
# slots of maps (stdtree.py's trees of 0-4 handles among the park's, so the thing's handle is erased from some). Person / Visitor: the script
# slots +0xc, +0xb4 (the history's +0x84) and +0x210 set or not, into the script VM's slot table (0x7b49f0: table, count; ready at 0x7b4a14)
# or past it; the history at +0x30 as actiondump.py's. Weather: the renderer at [0x8bcbac] as weatherfx.py's FreeTextures.
# usage: thingdetach.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Thing,Person,Visitor,Weather)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import stdtree  # noqa: E402
import weatherfx  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, global_heap, heap, log_region, put  # noqa: E402
from thingenv import SLOTS, THING, park  # noqa: E402

B = Bench([(0x50b980, 0x50ba10), (0x4d9280, 0x4d93b0), (0x4f8ae0, 0x4f8b08), (0x4fb330, 0x4fb359), (0x512000, 0x512010),
           (0x4d4170, 0x4d4186), (0x475550, 0x47558b), (0x5f5660, 0x5f56a0), (0x50bab0, 0x50bac4), (0x4faa60, 0x4fada1), (0x50c190, 0x50c1d0),
           # (messagecentre.py's: Instance, RemoveThing and the tree code under them)
           (0x409250, 0x409260), (0x40f970, 0x40f9e0), (0x40fa40, 0x40fb03), (0x40f950, 0x40f961), (0x410ac0, 0x410b11), (0x50b350, 0x50b35d),
           (0x5f5e50, 0x5f5e80), (0x5da3c0, 0x5da3c1), (0x40ff80, 0x4104e0), (0x4104b0, 0x410610), (0x410670, 0x410720), (0x410720, 0x4107a0),
           (0x67acb0, 0x67acb1), (0x67afa0, 0x67b0d0), (0x40f9e0, 0x40fa40), (0x40fe60, 0x40ff80), (0x4107b0, 0x410ac0), (0x410b20, 0x410bf0),
           (0x5e7720, 0x5e7750), (0x5f5e20, 0x5f5e50), (0x405460, 0x405530), (0x415e00, 0x415e80), (0x67acf0, 0x67ada0), (0x4a8bd0, 0x4a8beb),
           # (weatherfx.py's, for FreeTextures)
           (0x57d720, 0x57d7ed), (0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e)],
          hooks={0x463c80: _recorder(31, 1, cdecl=True), 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
CALLS = {'Thing': 0x50b980, 'Person': 0x4f8ae0, 'Visitor': 0x4fb330, 'Weather': 0x512000}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
POOL, CELL, TABLE = RAM + 0x40000, RAM + 0x48000, RAM + 0x58000
CENTRE, HEADS, NIL, NODES = RAM + 0x50000, RAM + 0x50400, RAM + 0x50800, RAM + 0x51000
COUNTER, SLOT_COUNT, MAX_NODES = 0x785de0, 0x1d, 4
KINDS = [1, 3, 3, 4, 5, 6, 7, 8, 0x12, 0, 2, 9, 0x11, 0x13, 0xff]


def centre(r, regions):
    """The message centre (29 slots: a dword, then a map of tree B) whose maps hold 0-4 of the handles 1-11."""
    slots, heads, nodes_blob = bytearray(0x14 * SLOT_COUNT), bytearray(0x20 * SLOT_COUNT), bytearray(r.bytes(0x20 * MAX_NODES * SLOT_COUNT))
    for k in range(SLOT_COUNT):
        n = r.choice([0, 0, 1, 2, 3, 4])
        nodes, root = stdtree.build(r, n, range(1, 12))
        keys = sorted(nodes)
        head = HEADS + 0x20 * k
        addr = {key: NODES + 0x20 * (MAX_NODES * k + i) for i, key in enumerate(keys)}

        def ptr(key):
            return NIL if key is None else addr[key]

        for key in keys:
            base = addr[key] - NODES
            struct.pack_into('<4I', nodes_blob, base, ptr(nodes[key][0]), head if nodes[key][1] is None else addr[nodes[key][1]], ptr(nodes[key][2]), key)
            struct.pack_into('<I', nodes_blob, base + 0x10, nodes[key][3])
        if n:
            struct.pack_into('<3I', heads, 0x20 * k, addr[keys[0]], addr[root], addr[keys[-1]])
        else:
            struct.pack_into('<3I', heads, 0x20 * k, head, NIL, head)
        struct.pack_into('<II', heads, 0x20 * k + 0xc, r.getrandbits(32), stdtree.RED)
        struct.pack_into('<5I', slots, 0x14 * k, r.getrandbits(32), r.getrandbits(32), head, r.getrandbits(32) & 0xff, n)
    nil = bytearray(0x20)
    struct.pack_into('<I', nil, 0x10, stdtree.BLACK)
    regions.update({CENTRE: slots, HEADS: heads, NIL: nil, NODES: nodes_blob, COUNTER: struct.pack('<II', r.choice([1, 2, 7]), NIL),
                    0x788d3c: struct.pack('<I', CENTRE)})


def scripts(r, regions, person, handles):
    """The person's script slots and history, and the script VM's slot table."""
    count = r.choice([0, 4, 8])
    table = bytearray(4 * 8)
    for i in range(8):
        put(table, 4 * i, 'I', r.choice([0, TABLE + 0x100 + 0x10 * i, TABLE + 0x100 + 0x10 * i]))
    regions[TABLE] = table
    vm = bytearray(r.bytes(0x28))
    put(vm, 0, 'Ii', TABLE, count)
    put(vm, 0x24, 'I', r.choice([0, 1, 1, 1, 0x100]))
    regions[0x7b49f0] = vm
    for off in (0xc, 0xb4, 0x210):
        put(person, off, 'i', r.choice([0, 0, 1, 2, 5, 7, 8, -1]))
    for i in range(32):
        handle = r.choice(handles * 3 + [0, 0]) if handles else 0
        struct.pack_into('<HH', person, 0x34 + 4 * i, r.choice(list(range(0x1a)) + [0x1a, 0xffff, r.getrandbits(16)]), handle)
    struct.pack_into('<I', person, 0xb8, r.choice([r.randrange(32), r.randrange(32), 0, 31, 32, 33]))


def case(r):
    op = r.deal(OPS)
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    if op == 'Weather':
        c = weatherfx.free_textures(r, 'FreeTextures')
        regions.update(c.regions)
        regions[0x8bcbac] = struct.pack('<I', weatherfx.FX)
    count = r.choice([1, 2, 3, 4, 6])
    world, handles, things = park(r, count=count, kinds=(1,), defs=op != 'Weather', pool=POOL)
    regions.update(world)
    slots = bytearray(regions[SLOTS])
    for h in range(12):
        if h not in handles:
            struct.pack_into('<I', slots, 20 * h, 0)
    regions[SLOTS] = slots
    pool = bytearray(regions[POOL])
    i = r.randrange(count)
    thing = POOL + THING * i
    t = THING * i
    pool[t + 2] = r.choice(KINDS)
    col, row = r.randrange(256), r.randrange(256)
    pool[t + 5], pool[t + 7] = col, row
    ident = ((row << 7) + col + 1) & 0xffff
    regions[0x8023a0] = struct.pack('<I', (CELL - ident * 0x44 + 0x44) & 0xffffffff)
    order = r.sample(range(count), r.randrange(1, count + 1))
    if i not in order and r.random() < 0.85:
        order[r.randrange(len(order))] = i
    for k, j in enumerate(order):
        put(pool, THING * j + 8, 'HH', handles[order[k - 1]] if k else 0, handles[order[k + 1]] if k + 1 < len(order) else 0)
    for j in range(count):
        if j not in order:
            put(pool, THING * j + 8, 'HH', r.choice([0, 0, handles[j]]), r.choice([0, 0, handles[j]]))
    cell = bytearray(r.bytes(0x44))
    put(cell, 0x24, 'H', handles[order[0]] if r.random() < 0.9 else 0)
    regions[CELL] = cell
    if op in ('Person', 'Visitor'):
        person = bytearray(pool[t:t + THING])
        scripts(r, regions, person, handles)
        pool[t:t + THING] = person
    regions[POOL] = pool
    regions[0x877350] = struct.pack('<I', thing if r.random() < 0.5 else r.choice([0, POOL + THING * r.randrange(count)]))
    centre(r, regions)
    return Call(CALLS[op], [thing], regions, conv='t', ret='a' if op in ('Thing', 'Weather') else 'v')


if __name__ == '__main__':
    B.main(case)

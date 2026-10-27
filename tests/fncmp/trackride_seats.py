# A ride's seats emptied and filled (src/game/world/trackrides: 0x54ac70 TrakRide_Unload, 0x549c60 TrakRide_Load) against the originals, with
# their callees run. Unload: 0-4 nodes on the
# ride's list (parts -1 or one of partstate.py's fixture's four, which PartStates_Release releases; moved clear of particles.py's state), the
# record at +0x9c with a list and a count, the instance at [0x7a4610 + 4 * index] holding the fixture's holder (its model's root position
# floats, now and then a NaN or out of the integers' range), effect 0, 1 or any dword (Particle_Spawn(0x13) on particles.py's Spawn state);
# the module loaded or not (the Debug_Printf's reads). Load: the same set-up, the nodes' handles (+0) 0-3 (the fixture's things) or others,
# the holder's model with 0-4 parts (flags with 0x80 or not, seat keys 0-5) for Model_FindPartByKey4, activated on the fixture.
# usage: trackride_seats.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Unload,Load)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import particles  # noqa: E402
import partstate  # noqa: E402
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

B = Bench(particles.RANGES + partstate.RANGES + [(0x44b4c0, 0x44b505), (0x54ac70, 0x54ad8e), (0x67a830, 0x67a860), (0x549c60, 0x549d29),
                                                  (0x44b220, 0x44b267), (0x44b410, 0x44b4c0), (0x475550, 0x47558b)])
RIDE, RECORD, NODES, INSTANCE, MODEL, ROOT, PARTS = (RAM + o for o in (0x30000, 0x30100, 0x30200, 0x30300, 0x30400, 0x30500, 0x30600))
# partstate.py's activation, moved clear of particles.py's state
partstate.INSTANCE, partstate.PS, partstate.ENTRIES, partstate.THINGS, partstate.TABLE, partstate.SPRITES, partstate.WORLD = (
    RAM + o for o in (0x21000, 0x21100, 0x21200, 0x21400, 0x21800, 0x22000, 0x23000))


def coord(r):
    return r.choice([r.uniform(-300, 300), r.uniform(-300, 300), r.uniform(-3e6, 3e6), float('nan'), 0.0])


def case(r):
    load = r.deal(OPS) == 'Load'
    regions = dict(particles.spawn(r).regions)
    defs = regions[0x804370]
    for k in range(0x69):   # no voice marks (as particles.py's update()): a case spawns up to four times, and a later voice start's mark
        if defs[0x140 * k + 0xba]:   # indexes by its counter (2 and up) past the game's image (0x816506 + 0x68 * 0x2xxxx), where the
            put(defs, 0x140 * k + 0xb8, 'H', 0)   # driver may or may not have memory (main's gate: 3 cases crashed in some runs only)
    regions[HEAP] = heap()
    partstate.activation(r, regions)
    holder = bytearray(regions[partstate.INSTANCE])
    put(holder, 4, 'I', MODEL)
    regions[partstate.INSTANCE] = bytes(holder)
    model = bytearray(r.bytes(0x80))
    put(model, 0x78, 'I', ROOT)
    nparts = r.choice([0, 1, 2, 3, 4, 4])
    put(model, 0x48, 'H', nparts)
    put(model, 0x7c, 'I', PARTS)
    parts = bytearray(r.bytes(0x14 * 4))
    for i in range(nparts):
        put(parts, 0x14 * i, 'I', r.getrandbits(32) & ~0x80 | r.choice([0x80, 0x80, 0]))
        put(parts, 0x14 * i + 4, 'I', r.choice([1, 2, 3, 4, 0, 5]))
    root = bytearray(r.bytes(0x50))
    put(root, 0x40, '3f', coord(r), coord(r), coord(r))
    inst = bytearray(r.bytes(0x10))
    put(inst, 8, 'I', partstate.INSTANCE)
    index = r.randrange(4)
    regions[0x7a4610] = b''.join(struct.pack('<I', INSTANCE if k == index else r.getrandbits(32)) for k in range(4))
    ride = bytearray(r.bytes(0xa0))
    n = r.choice([0, 1, 2, 3, 4, 4])
    nodes = bytearray(r.bytes(0x20 * 4))
    for k in range(n):
        put(nodes, 0x20 * k + 8, 'i', r.choice([-1, r.randrange(4), r.randrange(4)]))
        put(nodes, 0x20 * k, 'I', r.choice([0, 1, 2, 3, 3, 0x10002, r.getrandbits(32)]))
        put(nodes, 0x20 * k + 0x10, 'I', NODES + 0x20 * (k + 1) if k + 1 < n else 0)
    put(ride, 8, 'I', index)
    put(ride, 0x30, 'I', NODES if n else 0)
    put(ride, 0x9c, 'I', RECORD)
    record = bytearray(r.bytes(0xd0))
    put(record, 0x60, 'i', r.choice([0, n, r.randrange(-3, 20)]))
    module = bytearray(r.bytes(0x10))
    if r.random() < 0.5:
        put(module, 0, 'I', 0x4a454647)
    put(module, 8, 'I', r.choice([RECORD, RECORD - 0xd0 * 3, r.getrandbits(32)]))
    regions.update({RIDE: ride, RECORD: record, NODES: nodes, INSTANCE: inst, MODEL: model, ROOT: root, PARTS: parts, 0x877b58: module})
    effect = r.choice([0, 1, 1, 0x100, r.getrandbits(32)])
    return Call(0x549c60 if load else 0x54ac70, [RIDE, effect], regions, ret='v' if load else 'a')


OPS = [k for k in os.environ.get('OPS', 'Unload,Load').split(',') if k]


if __name__ == '__main__':
    B.main(case)

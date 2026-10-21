# A ride script's particle effect toggle (src/game/coaster/ridebounds: 0x556de0) against the original: on or off (any dword now and then),
# the context's system +0xcc none, a live system's handle or random; its thing slot +0xac 0 or one of a few (gThingSlots' things with a
# type gObjectDefs finds, mostly: a definition with effect 0 or one of particles.py's); its ride index +0xc8 0 or ridebounds.py's ride (with
# or without a box); the particle state as particles.py's Spawn cases.
# usage: rideeffect.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import RAM, Bench, Call, put  # noqa: E402
import particles  # noqa: E402
import ridebounds  # noqa: E402

B = Bench(particles.RANGES + [(0x556de0, 0x556f40), (0x466b70, 0x466c7f), (0x412e90, 0x412eec), (0x67a830, 0x67a860)])
CONTEXT, DEFS, THINGS = RAM + 0x4000, RAM + 0x5000, RAM + 0x6000


def case(r):
    c = ridebounds.box(r)
    regions = dict(particles.spawn(r).regions)
    regions.update(c.regions)
    ctx = bytearray(r.bytes(0x100))
    slot = r.randrange(6)
    serial = struct.unpack_from('<H', regions[0x80cefc], 0x140 * slot + 0xa)[0]
    put(ctx, 0xcc, 'I', r.choice([0, serial << 16 | slot, serial << 16 | slot, r.getrandbits(32)]))
    put(ctx, 0xac, 'H', r.choice([0, 1, 2, 3, 4, 5, 5, 7]))
    put(ctx, 0xc8, 'I', r.choice([0, c.args[0], c.args[0], c.args[0]]))
    regions[CONTEXT] = ctx
    # gThingSlots (0x14 bytes each): slots 1-7 hold things of types 0x10..0x16 (now and then one no definition has)
    slots = bytearray(r.bytes(0x14 * 8))
    things = bytearray(r.bytes(0x20 * 8))
    for k in range(8):
        put(slots, 0x14 * k, 'I', THINGS + 0x20 * k)
        put(things, 0x20 * k + 0xe, 'H', 0x10 + k if r.random() < 0.97 else 0x99)
    regions[0x7cfb90] = slots
    regions[THINGS] = things
    # gObjectDefs: eight definitions in a random order, ids 0x10..0x17, effects 0 or one of the particles' (the cache some id or another)
    defs = bytearray(r.bytes(0x80 * 8))
    table = bytearray(r.bytes(0x738))
    for i, k in enumerate(r.sample(range(8), 8)):
        put(table, 4 * i, 'I', DEFS + 0x80 * k)
        put(defs, 0x80 * k, 'H', 0x10 + k)
        effect = r.choice([0, 0, 1, 0x68, r.randrange(0x69)])
        put(defs, 0x80 * k + 0x68, 'I', effect)
    put(table, 0x528, 'i', 8)
    k = r.randrange(8)
    put(table, 0x730, 'HHI', r.choice([0x10 + k, 0x30]), 0, DEFS + 0x80 * k)
    regions[0x7890a0] = table
    regions[DEFS] = defs
    on = r.choice([0, 0, 1, 1, 1, 0x100, r.getrandbits(32)])
    return Call(0x556de0, [CONTEXT, on], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
